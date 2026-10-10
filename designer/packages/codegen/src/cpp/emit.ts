/**
 * C++ の書き出し(docs/designer/codegen-design.md §2〜§5)。
 *
 * - ヘッダ: 区間 tokotachi_header_begin(インクルードガードと <tokotachi/tokotachi.hpp>)、tokotachi_namespace_begin、
 *   生成するクラス(区間 declarations にメンバとハンドラの宣言)、tokotachi_namespace_end、tokotachi_header_end。
 * - ソース: 区間 tokotachi_source_begin(ヘッダの include)、tokotachi_namespace_begin、コンストラクタ、区間 tokotachi_setup_ui、
 *   ハンドラの雛形、tokotachi_namespace_end。
 * - 生成したコードは tokotachi:: を省かずに書き、using namespace を書かない(N2)。
 */
import { getCatalog, namedNodes, type TtuiDocument } from '@tokotachi-designer/core';
import { generatedComments } from '../comments.ts';
import { buildSetup, type Arg, type Op } from '../model.ts';
import type { CppTarget, CommentLocale } from '../names.ts';
import type { GeneratedCode, HandlerStub, Region } from '../region.ts';
import { cppString, cppValue, enumValue, placementStatements } from './values.ts';

const INDENT = '    ';

export interface CppFiles {
  readonly header: GeneratedCode;
  readonly source: GeneratedCode;
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

/** 名前があればメンバへの代入、無ければ一時的な変数の宣言。 */
function assignee(variable: string, cls: string, declare: boolean): string {
  return declare ? `tokotachi::${cls}* ${variable}` : variable;
}

/** 操作の並びを tokotachi_setup_ui の中身の行にする(空行は '')。 */
function printOps(ops: readonly Op[]): string[] {
  return ops.flatMap((op): string[] => {
    switch (op.op) {
      case 'new':
        return [
          `${assignee(op.variable, op.class, op.declare)} = new tokotachi::${op.class}(${op.parent ?? ''});`,
        ];
      case 'set':
        return [`${access(op.target, op.property)} = ${cppValue(op.type, op.value)};`];
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
function namespaceBegin(namespace: readonly string[]): string {
  return namespace.map((n) => `namespace ${n}\n{`).join('\n');
}

function namespaceEnd(namespace: readonly string[]): string {
  return [...namespace]
    .reverse()
    .map((n) => `}  // namespace ${n}`)
    .join('\n');
}

/**
 * @param sourceName 定義ファイルの名前(生成物の説明に使う)
 */
export function emitCpp(
  doc: TtuiDocument,
  sourceName: string,
  target: CppTarget,
  locale: CommentLocale,
): CppFiles {
  const comments = generatedComments(locale);
  const { className, includeGuard, namespace } = target;
  const base = doc.window.class;
  const setup = buildSetup(doc);
  const body = printOps(setup.ops);
  const members = namedNodes(doc);

  const declarations: Region = {
    id: 'declarations',
    indent: 1,
    content: [
      ...members.map((m) => `${INDENT}tokotachi::${m.class}* ${m.name};`),
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
  const constructor =
    base === 'Dialog' ? `explicit ${className}(tokotachi::Window* parent);` : `${className}();`;

  const header: GeneratedCode = {
    regions: [headerBegin, nsBegin, declarations, nsEnd, headerEnd],
    stubs: [],
    scaffold: (rendered) =>
      [
        rendered('tokotachi_header_begin'),
        '',
        rendered('tokotachi_namespace_begin'),
        '',
        `/// ${comments.classDoc(sourceName)}`,
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
    content: `#include "${target.headerInclude}"`,
  };
  const init =
    base === 'Dialog'
      ? `${INDENT}: tokotachi::Dialog(parent, std::string())`
      : `${INDENT}: tokotachi::Window(std::string())`;
  const source: GeneratedCode = {
    regions: [sourceBegin, nsBegin, setupRegion, nsEnd],
    stubs,
    scaffold: (rendered) =>
      [
        rendered('tokotachi_source_begin'),
        '',
        rendered('tokotachi_namespace_begin'),
        '',
        `${className}::${base === 'Dialog' ? `${className}(tokotachi::Window* parent)` : `${className}()`}`,
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
  return { header, source };
}
