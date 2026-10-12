/**
 * コード生成(docs/designer/codegen-design.md)。
 * 文字列を受け取り文字列を返すだけで、ファイルの入出力は呼び出し側(拡張・CLI)が行う。
 */
import {
  componentReferences,
  findClass,
  namedNodes,
  imageReferences,
  l10n,
  normalizePath,
  relativePath,
  resolvePath,
  themePresetReferences,
  type TtuiDocument,
} from '@tokotachi-designer/core';
import { emitCpp, type EmittedComponent } from './cpp/emit.ts';
import { embedImages } from './cpp/images.ts';
import { CPP_NAMES, cppSyntax } from './cpp/syntax.ts';
import {
  cppOutputPaths,
  DEFAULT_CPP_OPTIONS,
  fileNameOf,
  resolveCppTarget,
  type CppOptions,
} from './names.ts';
import { createFile, mergeFile, type GeneratedCode, type MergeResult } from './region.ts';
import { findStaleNames, staleNamesWarning } from './stale.ts';

export { regionHash } from './hash.ts';
export {
  generateProject,
  projectName,
  projectOutputPaths,
  windowCppOptions,
  type ProjectWindow,
} from './project.ts';
export { findHandler, type HandlerPosition } from './locate.ts';
export {
  baseName,
  cppOutputPaths,
  DEFAULT_CPP_OPTIONS,
  resolveCppTarget,
  type CommentLocale,
  type CppOptions,
  type CppTarget,
} from './names.ts';
export type { MergeResult } from './region.ts';
export { cppString } from './cpp/values.ts';
export {
  buildSetup,
  COMPONENT_ROOT_SKIP,
  inlineOps,
  type Arg,
  type ComponentResolver,
  type ResolvedComponent,
  type SetupOptions,
  type Handler,
  type Op,
  type Result,
  type SetupModel,
} from './model.ts';

export type { EmittedComponent } from './cpp/emit.ts';

/** 出力する 1 ファイル分の生成結果。 */
export interface OutputFile {
  /** 定義ファイルのあるフォルダからの相対パス。 */
  readonly path: string;
  readonly result: MergeResult;
}

/** 置いた部品ファイル(docs/adr/0043)の中身と、そのコード生成の設定。 */
export interface ComponentSource {
  readonly document: TtuiDocument;
  readonly options: CppOptions;
}

/** 仮の絶対パスの根(core のパスの計算は絶対パスを扱う。`..` をたどれるよう深くする)。 */
const ROOT = '/r/r/r/r/r/r/r/r/r/r/r/r/r/r/r/r';

/** ドキュメントが使う自作部品のヘッダ(プロジェクトのフォルダからのパス。最初に現れた順)。 */
function customHeaders(doc: TtuiDocument): string[] {
  const headers = new Set<string>();
  for (const n of namedNodes(doc)) {
    const header = findClass(n.class)?.custom?.header;
    if (header !== undefined) {
      headers.add(header);
    }
  }
  return [...headers];
}

/** 部品ファイルのヘッダの、置いた定義ファイルのフォルダからのパス。 */
function componentHeader(path: string, options: CppOptions): string {
  const dir = resolvePath(ROOT, `${normalizePath(path)}/..`);
  return relativePath(ROOT, resolvePath(dir, cppOutputPaths(fileNameOf(path), options).header));
}

export interface GenerateResult {
  readonly files: readonly OutputFile[];
  readonly warnings: readonly string[];
}

/**
 * C++ のヘッダとソースを生成する。既存の内容があればマーカーの区間だけを置き換え、無ければ新しいファイルを作る。
 * @param doc 検証を通ったドキュメント
 * @param dslFileName 定義ファイルの名前(出力先と、生成物の説明に使う)
 * @param readExisting 出力先の既存の内容を返す(無ければ undefined)
 * @param readImage 定義ファイルが参照する画像の中身を返す(パスは定義ファイルのフォルダからの相対パス。無ければ undefined)。
 *   読めない画像があれば、ソースを生成せずにエラーにする
 * @param readComponent 置いた部品ファイル(パスは定義ファイルに書いたまま)の中身と設定を返す。読めなければ undefined で、
 *   ソースを生成せずにエラーにする
 */
export function generateCpp(
  doc: TtuiDocument,
  dslFileName: string,
  readExisting: (path: string) => string | undefined,
  options: CppOptions = DEFAULT_CPP_OPTIONS,
  readImage: (path: string) => Uint8Array | undefined = () => undefined,
  readComponent: (path: string) => ComponentSource | undefined = () => undefined,
): GenerateResult {
  const target = resolveCppTarget(doc, dslFileName, options);
  const fail = (error: string): GenerateResult => ({
    files: [
      { path: target.header, result: { ok: false, error } },
      { path: target.source, result: { ok: false, error } },
    ],
    warnings: [],
  });
  // プロジェクトに登録したテーマの参照は、プロジェクトに属するウィンドウだけが使え、登録した名前でなければならない
  for (const ref of themePresetReferences(doc)) {
    if (!options.themes) {
      return fail(l10n.t('The theme "{0}" can be used only in a window of a project', ref.name));
    }
    if (!Object.hasOwn(options.themes.presets, ref.name)) {
      return fail(l10n.t('The theme "{0}" is not registered in the project', ref.name));
    }
  }
  const paths = imageReferences(doc).map((r) => normalizePath(r.path));
  const data = new Map<string, Uint8Array>();
  for (const path of paths) {
    const bytes = data.has(path) ? data.get(path) : readImage(path);
    if (bytes === undefined) {
      return fail(l10n.t('Cannot read the image file "{0}"', path));
    }
    data.set(path, bytes);
  }
  const images = embedImages(paths, data);
  // 置いた自作部品(docs/adr/0044)のヘッダ。自作部品はプロジェクトに登録したもので、プロジェクトに属するウィンドウでだけ使える
  const customIncludes: string[] = [];
  for (const header of customHeaders(doc)) {
    if (options.projectDir === undefined) {
      return fail(l10n.t('Custom widgets can be used only in a window of a project'));
    }
    customIncludes.push(
      relativePath(
        resolvePath(ROOT, `${target.header}/..`),
        resolvePath(ROOT, `${options.projectDir}/${header}`),
      ),
    );
  }
  const components = new Map<string, EmittedComponent>();
  for (const ref of componentReferences(doc)) {
    if (components.has(ref.path)) {
      continue;
    }
    const source = readComponent(ref.path);
    if (!source) {
      return fail(l10n.t('Cannot read the component file "{0}"', ref.path));
    }
    // ヘッダから部品ファイルのヘッダへの相対パス
    const headerDir = resolvePath(ROOT, `${target.header}/..`);
    components.set(ref.path, {
      document: source.document,
      include: relativePath(
        headerDir,
        resolvePath(ROOT, componentHeader(ref.path, source.options)),
      ),
    });
  }
  const code = emitCpp(
    doc,
    fileNameOf(dslFileName),
    target,
    options.commentLocale,
    images,
    components,
    customIncludes,
  );
  const syntax = cppSyntax(target.className);
  const generate = (generated: GeneratedCode, existing: string | undefined): MergeResult =>
    existing === undefined
      ? { ok: true, text: createFile(generated, syntax), modifiedRegions: [], addedStubs: [] }
      : mergeFile(existing, generated, syntax);

  const existingHeader = readExisting(target.header);
  const header = generate(code.header, existingHeader);
  const source = generate(code.source, readExisting(target.source));
  const files: OutputFile[] = [
    { path: target.header, result: header },
    { path: target.source, result: source },
  ];
  // 画像は別のソースに書く(docs/adr/0053)。プロジェクトなら CMakeLists.txt のソースの一覧に入るが、そうでなければ利用者がビルドに加える
  const existingImages = readExisting(target.images);
  const notices: string[] = [];
  if (code.images) {
    files.push({ path: target.images, result: generate(code.images, existingImages) });
    if (existingImages === undefined && options.projectDir === undefined) {
      notices.push(l10n.t('{0} was created. Add it to the sources of your build.', target.images));
    }
  } else if (existingImages !== undefined) {
    notices.push(
      l10n.t(
        '{0} is no longer used because the window refers to no images. You can delete it (and remove it from your build).',
        target.images,
      ),
    );
  }

  const warnings = [
    ...files.flatMap((f) =>
      f.result.ok ? (f.result.warnings ?? []).map((w) => `${f.path}: ${w}`) : [],
    ),
    ...notices,
  ];
  if (existingHeader !== undefined && header.ok && source.ok) {
    // 生成したメンバはヘッダの宣言の区間から取り出し、使われているかはヘッダとソースの両方で調べる
    const stale = staleNamesWarning(
      findStaleNames({
        rules: CPP_NAMES,
        before: existingHeader,
        after: header.text,
        files: [
          { path: target.header, text: header.text },
          { path: target.source, text: source.text },
        ],
      }),
    );
    if (stale) {
      warnings.push(stale);
    }
  }
  return { files, warnings };
}
