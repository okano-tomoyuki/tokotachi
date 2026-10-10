/**
 * コード生成(docs/designer/codegen-design.md)。
 * 文字列を受け取り文字列を返すだけで、ファイルの入出力は呼び出し側(拡張・CLI)が行う。
 */
import { imageReferences, l10n, normalizePath, type TtuiDocument } from '@tokotachi-designer/core';
import { emitCpp } from './cpp/emit.ts';
import { embedImages } from './cpp/images.ts';
import { CPP_NAMES, cppSyntax } from './cpp/syntax.ts';
import { DEFAULT_CPP_OPTIONS, fileNameOf, resolveCppTarget, type CppOptions } from './names.ts';
import { createFile, mergeFile, type GeneratedCode, type MergeResult } from './region.ts';
import { findStaleNames, staleNamesWarning } from './stale.ts';

export { regionHash } from './hash.ts';
export { generateProject, projectName, windowCppOptions, type ProjectWindow } from './project.ts';
export { findHandler, type HandlerPosition } from './locate.ts';
export {
  baseName,
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
  type Arg,
  type Handler,
  type Op,
  type Result,
  type SetupModel,
} from './model.ts';

/** 出力する 1 ファイル分の生成結果。 */
export interface OutputFile {
  /** 定義ファイルのあるフォルダからの相対パス。 */
  readonly path: string;
  readonly result: MergeResult;
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
 */
export function generateCpp(
  doc: TtuiDocument,
  dslFileName: string,
  readExisting: (path: string) => string | undefined,
  options: CppOptions = DEFAULT_CPP_OPTIONS,
  readImage: (path: string) => Uint8Array | undefined = () => undefined,
): GenerateResult {
  const target = resolveCppTarget(doc, dslFileName, options);
  const paths = imageReferences(doc).map((r) => normalizePath(r.path));
  const data = new Map<string, Uint8Array>();
  for (const path of paths) {
    const bytes = data.has(path) ? data.get(path) : readImage(path);
    if (bytes === undefined) {
      const error = l10n.t('Cannot read the image file "{0}"', path);
      return {
        files: [
          { path: target.header, result: { ok: false, error } },
          { path: target.source, result: { ok: false, error } },
        ],
        warnings: [],
      };
    }
    data.set(path, bytes);
  }
  const images = embedImages(paths, data);
  const code = emitCpp(doc, fileNameOf(dslFileName), target, options.commentLocale, images);
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

  const warnings = files.flatMap((f) =>
    f.result.ok ? (f.result.warnings ?? []).map((w) => `${f.path}: ${w}`) : [],
  );
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
