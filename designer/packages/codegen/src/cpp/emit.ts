/**
 * C++ の書き出し(docs/designer/codegen-design.md §2〜§5)。
 *
 * - ヘッダ: 区間 tokotachi_header_begin(インクルードガードと <tokotachi/tokotachi.hpp>)、tokotachi_namespace_begin、
 *   生成するクラス(区間 declarations にメンバとハンドラの宣言)、tokotachi_namespace_end、tokotachi_header_end。
 * - ソース: 区間 tokotachi_source_begin(ヘッダの include)、tokotachi_namespace_begin、コンストラクタ、区間 tokotachi_setup_ui、
 *   ハンドラの雛形、tokotachi_namespace_end。
 * - 生成したコードは tokotachi:: を省かずに書き、using namespace を書かない(N2)。
 */
import {
  findClass,
  getCatalog,
  iconEnumName,
  iconOf,
  isImageType,
  namedNodes,
  normalizePath,
  themePresetReferences,
  variableClass,
  type TtuiDocument,
} from '@tokotachi-designer/core';
import { generatedComments } from '../comments.ts';
import { buildSetup, type Arg, type Op } from '../model.ts';
import { fileNameOf, type CppTarget, type CommentLocale } from '../names.ts';
import type { GeneratedCode, HandlerStub, Region } from '../region.ts';
import {
  imageDeclarations,
  imageDefinitions,
  imageExpression,
  type EmbeddedImage,
} from './images.ts';
import { cppString, cppValue, enumValue, placementStatements } from './values.ts';

const INDENT = '    ';

/** 置いた部品ファイル(docs/adr/0043)。 */
export interface EmittedComponent {
  readonly document: TtuiDocument;
  /** ヘッダから部品ファイルのヘッダを include するパス。 */
  readonly include: string;
}

export interface CppFiles {
  readonly header: GeneratedCode;
  readonly source: GeneratedCode;
  /** 埋め込む画像のソース(画像を参照するときだけ)。 */
  readonly images?: GeneratedCode;
}

/** ハンドラの型の引数の並び(`tokotachi::Object* sender, bool& can_close`)。 */
function paramList(type: string): string {
  return (getCatalog().handler_types[type] ?? [])
    .map((p) => `${/^[A-Z]/.test(p.type) ? `tokotachi::${p.type}` : p.type} ${p.name}`)
    .join(', ');
}

function argList(type: string): string {
  return (getCatalog().handler_types[type] ?? []).map((p) => p.name).join(', ');
}

/** target が undefined ならウィンドウ自身(メンバをそのまま書く)。 */
function access(target: string | undefined, member: string): string {
  return target === undefined ? member : `${target}->${member}`;
}

function argText(arg: Arg): string {
  switch (arg.kind) {
    case 'ref':
      return arg.name;
    case 'int':
      return String(arg.value);
    case 'float':
      return cppValue('float', arg.value);
    case 'string':
      return cppString(arg.value);
    case 'enum':
      return enumValue(arg.enum, arg.value);
  }
}

/** C++ のクラスの名前(ライブラリのクラスは `tokotachi::Button`、自作部品は名前空間を付けた名前。docs/adr/0044)。 */
export function cppClass(cls: string): string {
  return findClass(cls)?.custom?.qualified ?? `tokotachi::${cls}`;
}

/** 名前があればメンバへの代入、無ければ一時的な変数の宣言。 */
function assignee(variable: string, cls: string, declare: boolean): string {
  return declare ? `${cppClass(cls)}* ${variable}` : variable;
}

/**
 * 操作の並びを tokotachi_setup_ui の中身の行にする(空行は '')。
 * @param images 画像のパス(正規化したもの)→ 埋め込んだ画像を読む式
 */
function printOps(ops: readonly Op[], images: ReadonlyMap<string, string>): string[] {
  return ops.flatMap((op): string[] => {
    switch (op.op) {
      case 'new':
        return [
          op.component !== undefined
            ? `${op.variable} = new ${op.component}(${op.parent ?? ''});`
            : `${assignee(op.variable, op.class, op.declare)} = new ${cppClass(op.class)}(${op.parent ?? ''});`,
        ];
      case 'set': {
        const icon = isImageType(op.type) ? iconOf(op.value) : undefined;
        const image =
          icon !== undefined
            ? `tokotachi::Image::icon(tokotachi::Icon::${iconEnumName(icon)})`
            : isImageType(op.type) && typeof op.value === 'string'
              ? images.get(normalizePath(op.value))
              : undefined;
        const value = image ?? cppValue(op.type, op.value);
        return [`${access(op.target, op.property)} = ${value};`];
      }
      case 'variable': {
        const cls = `tokotachi::${variableClass(op.type)}`;
        const value = op.value === undefined ? '' : `, ${cppValue(op.type, op.value)}`;
        return [`${op.variable} = new ${cls}(this${value});`];
      }
      case 'bind':
        return [`tokotachi::bind(${access(op.target, op.property)}, ${op.variable}->value);`];
      case 'connect':
        return [
          `${access(op.target, op.property)} = [this](${paramList(op.handlerType)}) { ${op.handler}(${argList(op.handlerType)}); };`,
        ];
      case 'call': {
        const call = `${access(op.target, op.method)}(${op.args.map(argText).join(', ')});`;
        return [
          op.result
            ? `${assignee(op.result.variable, op.result.class, op.result.declare)} = ${call}`
            : call,
        ];
      }
      case 'place':
        return [
          '{',
          `${INDENT}tokotachi::Placement placement;`,
          ...placementStatements('placement', op.placement).map((s) => INDENT + s),
          `${INDENT}${op.layout}->add(${op.widget}, placement);`,
          '}',
        ];
      case 'blank':
        return [''];
    }
  });
}

/** 名前空間の始まり(C++11 でも通るよう、入れ子は 1 つずつ書く)。 */
export function namespaceBegin(namespace: readonly string[]): string {
  return namespace.map((n) => `namespace ${n}\n{`).join('\n');
}

export function namespaceEnd(namespace: readonly string[]): string {
  return [...namespace]
    .reverse()
    .map((n) => `}  // namespace ${n}`)
    .join('\n');
}

/** コンストラクタの引数(Dialog は持ち主のウィンドウ、部品ファイルは親の部品)。 */
function constructorParams(base: string): string {
  switch (base) {
    case 'Dialog':
      return 'tokotachi::Window* parent';
    case 'Widget':
      return 'tokotachi::Widget* parent';
    default:
      return '';
  }
}

function baseInitializer(base: string): string {
  switch (base) {
    case 'Dialog':
      return 'tokotachi::Dialog(parent, std::string())';
    case 'Widget':
      return 'tokotachi::Widget(parent)';
    default:
      return 'tokotachi::Window(std::string())';
  }
}

/**
 * @param sourceName 定義ファイルの名前(生成物の説明に使う)
 * @param images 埋め込む画像(定義ファイルが参照するもの。cpp/images.ts の embedImages)
 * @param components 置いた部品ファイル(パスは定義ファイルに書いたまま)と、そのクラス・ヘッダを include するパス
 * @param customIncludes 置いた自作部品(docs/adr/0044)のヘッダを include するパス
 */
export function emitCpp(
  doc: TtuiDocument,
  sourceName: string,
  target: CppTarget,
  locale: CommentLocale,
  images: readonly EmbeddedImage[] = [],
  components: ReadonlyMap<string, EmittedComponent> = new Map(),
  customIncludes: readonly string[] = [],
): CppFiles {
  const comments = generatedComments(locale);
  const { className, includeGuard, namespace } = target;
  const base = doc.window.class;
  const setup = buildSetup(doc, {
    resolve: (path) => {
      const c = components.get(path);
      return c ? { document: c.document } : undefined;
    },
  });
  const body = printOps(
    setup.ops,
    new Map(images.map((i) => [i.path, imageExpression(className, i.variable)])),
  );
  const members = namedNodes(doc);
  // 部品ファイルを置いた部品のメンバの型は、部品ファイルのクラス
  const componentClasses = new Map(
    setup.ops.flatMap((op) =>
      op.op === 'new' && op.component !== undefined ? [[op.variable, op.component] as const] : [],
    ),
  );
  const includes = [
    ...new Set([...customIncludes, ...[...components.values()].map((c) => c.include)]),
  ];

  const declarations: Region = {
    id: 'declarations',
    indent: 1,
    content: [
      ...members.map(
        (m) => `${INDENT}${componentClasses.get(m.name) ?? cppClass(m.class)}* ${m.name};`,
      ),
      ...(members.length > 0 && setup.handlers.length > 0 ? [''] : []),
      ...setup.handlers.map((h) => `${INDENT}void ${h.name}(${paramList(h.type)});`),
    ].join('\n'),
  };
  const headerBegin: Region = {
    id: 'tokotachi_header_begin',
    indent: 0,
    content: [
      `#ifndef ${includeGuard}`,
      `#define ${includeGuard}`,
      '',
      '#include <tokotachi/tokotachi.hpp>',
      ...includes.map((i) => `#include "${i}"`),
    ].join('\n'),
  };
  const nsBegin: Region = {
    id: 'tokotachi_namespace_begin',
    indent: 0,
    content: namespaceBegin(namespace),
  };
  const nsEnd: Region = {
    id: 'tokotachi_namespace_end',
    indent: 0,
    content: namespaceEnd(namespace),
  };
  const headerEnd: Region = {
    id: 'tokotachi_header_end',
    indent: 0,
    content: `#endif  // ${includeGuard}`,
  };
  const params = constructorParams(base);
  const constructor = params === '' ? `${className}();` : `explicit ${className}(${params});`;

  const header: GeneratedCode = {
    regions: [headerBegin, nsBegin, declarations, nsEnd, headerEnd],
    stubs: [],
    scaffold: (rendered) =>
      [
        rendered('tokotachi_header_begin'),
        '',
        rendered('tokotachi_namespace_begin'),
        '',
        `/// ${base === 'Widget' ? comments.componentDoc(sourceName) : comments.classDoc(sourceName)}`,
        `class ${className} : public tokotachi::${base}`,
        '{',
        'public:',
        `${INDENT}${constructor}`,
        '',
        rendered('declarations'),
        '',
        'private:',
        `${INDENT}void tokotachi_setup_ui();`,
        '};',
        '',
        rendered('tokotachi_namespace_end'),
        '',
        rendered('tokotachi_header_end'),
        '',
      ].join('\n'),
  };

  const setupRegion: Region = {
    id: 'tokotachi_setup_ui',
    indent: 0,
    content: [
      ...imageDeclarations(
        images,
        className,
        comments.imagesDeclaration(fileNameOf(target.images)),
      ),
      `// ${comments.setupUi}`,
      `void ${className}::tokotachi_setup_ui()`,
      '{',
      ...body.map((l) => (l === '' ? '' : INDENT + l)),
      '}',
    ].join('\n'),
  };
  const stubs: HandlerStub[] = setup.handlers.map((h) => ({
    name: h.name,
    code: [`void ${className}::${h.name}(${paramList(h.type)})`, '{', '}'].join('\n'),
  }));
  const sourceBegin: Region = {
    id: 'tokotachi_source_begin',
    indent: 0,
    content: [
      `#include "${target.headerInclude}"`,
      // プロジェクトに登録したテーマを参照していれば、その関数のヘッダ
      ...(target.themesInclude !== undefined && themePresetReferences(doc).length > 0
        ? [`#include "${target.themesInclude}"`]
        : []),
    ].join('\n'),
  };
  const init = `${INDENT}: ${baseInitializer(base)}`;
  const source: GeneratedCode = {
    regions: [sourceBegin, nsBegin, setupRegion, nsEnd],
    stubs,
    scaffold: (rendered) =>
      [
        rendered('tokotachi_source_begin'),
        '',
        rendered('tokotachi_namespace_begin'),
        '',
        `${className}::${className}(${params})`,
        init,
        '{',
        `${INDENT}tokotachi_setup_ui();`,
        '}',
        '',
        rendered('tokotachi_setup_ui'),
        '',
        '// <tokotachi-designer:handler-stubs>',
        ...stubs.flatMap((s) => [s.code, '']),
        rendered('tokotachi_namespace_end'),
        '',
      ].join('\n'),
  };
  // 画像は別のファイルに書く(全体を 1 つの区間にして、生成し直すと上書きする)
  const imagesFile: GeneratedCode | undefined =
    images.length === 0
      ? undefined
      : {
          regions: [
            {
              id: 'tokotachi_images',
              indent: 0,
              content: [
                `// ${comments.imagesFile(sourceName)}`,
                '',
                ...imageDefinitions(
                  images,
                  className,
                  namespaceBegin(namespace),
                  namespaceEnd(namespace),
                ),
              ].join('\n'),
            },
          ],
          stubs: [],
          scaffold: (rendered) => `${rendered('tokotachi_images')}\n`,
        };
  return { header, source, ...(imagesFile ? { images: imagesFile } : {}) };
}
