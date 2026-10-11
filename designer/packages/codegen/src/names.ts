/**
 * 出力先・クラス名・名前空間・インクルードガード(docs/designer/codegen-design.md §1・§8)。
 * プロジェクトファイル(*.ttproj.json)を作るまでは、設定は CLI・拡張が CppOptions で渡す。
 */
import { DOCUMENT_EXTENSION, type ThemePresets, type TtuiDocument } from '@tokotachi-designer/core';

/** プロジェクトに登録したテーマの関数を置く共通のヘッダの名前(project-spec.md §4.2)。 */
export const THEMES_HEADER = 'tokotachi_themes.hpp';

/** 生成するコードのコメントの言語。 */
export type CommentLocale = 'en' | 'ja';

/** C++ のコード生成の設定。 */
export interface CppOptions {
  /** 名前空間(`app::ui` なら ["app", "ui"]。無ければ空)。 */
  readonly namespace: readonly string[];
  readonly includeGuardPrefix: string;
  readonly headerExtension: string;
  readonly sourceExtension: string;
  /** 定義ファイルのフォルダからの相対パス(`/` 区切り)。undefined なら定義ファイルと同じフォルダ。 */
  readonly headerDir: string | undefined;
  readonly sourceDir: string | undefined;
  readonly commentLocale: CommentLocale;
  /** プロジェクトに登録したテーマと、その関数を置くヘッダ(定義ファイルのフォルダからの相対パス)。プロジェクトに属さなければ無い。 */
  readonly themes?: { readonly presets: ThemePresets; readonly header: string } | undefined;
}

export const DEFAULT_CPP_OPTIONS: CppOptions = {
  namespace: [],
  includeGuardPrefix: '',
  headerExtension: '.hpp',
  sourceExtension: '.cpp',
  headerDir: undefined,
  sourceDir: undefined,
  commentLocale: 'en',
};

export interface CppTarget {
  readonly className: string;
  /** 定義ファイルのあるフォルダからの相対パス。 */
  readonly header: string;
  readonly source: string;
  /** ソースからヘッダを include するパス。 */
  readonly headerInclude: string;
  readonly namespace: readonly string[];
  /** インクルードガードのマクロの名前。 */
  readonly includeGuard: string;
  /** ソースから、登録したテーマのヘッダを include するパス(プロジェクトに属するときだけ)。 */
  readonly themesInclude?: string;
}

/** パスの最後の要素(ファイル名)。 */
export function fileNameOf(path: string): string {
  return (
    path
      .split(/[\\/]/)
      .filter((s) => s !== '')
      .pop() ?? path
  );
}

/** 定義ファイルの名前(例: "main_window.ttui.json")から拡張子を除いた名前("main_window")。 */
export function baseName(dslFileName: string): string {
  const name = fileNameOf(dslFileName);
  return name.endsWith(DOCUMENT_EXTENSION)
    ? name.slice(0, -DOCUMENT_EXTENSION.length)
    : name.replace(/\.json$/, '');
}

function join(dir: string | undefined, file: string): string {
  return dir === undefined || dir === '' || dir === '.'
    ? file
    : `${dir.replace(/\/+$/, '')}/${file}`;
}

/** from のフォルダから to のファイルへの相対パス(どちらも定義ファイルのフォルダからの相対パス。`/` 区切り)。 */
function relative(fromDir: string | undefined, to: string): string {
  const from = (fromDir ?? '').split('/').filter((s) => s !== '' && s !== '.');
  const target = to.split('/').filter((s) => s !== '' && s !== '.');
  let common = 0;
  while (common < from.length && common < target.length - 1 && from[common] === target[common]) {
    common++;
  }
  return [...Array<string>(from.length - common).fill('..'), ...target.slice(common)].join('/');
}

/** ヘッダ・ソースの出力先(定義ファイルのフォルダからの相対パス。定義ファイルの中身には依らない)。 */
export function cppOutputPaths(
  dslFileName: string,
  options: CppOptions,
): { readonly header: string; readonly source: string } {
  const base = baseName(dslFileName);
  return {
    header: join(options.headerDir, `${base}${options.headerExtension}`),
    source: join(options.sourceDir, `${base}${options.sourceExtension}`),
  };
}

export function resolveCppTarget(
  doc: TtuiDocument,
  dslFileName: string,
  options: CppOptions,
): CppTarget {
  const headerFile = `${baseName(dslFileName)}${options.headerExtension}`;
  const { header, source } = cppOutputPaths(dslFileName, options);
  return {
    className: doc.window.name,
    header,
    source,
    headerInclude: relative(options.sourceDir, header),
    namespace: options.namespace,
    includeGuard: `${options.includeGuardPrefix}${headerFile}`
      .toUpperCase()
      .replace(/[^A-Z0-9]/g, '_'),
    ...(options.themes
      ? { themesInclude: relative(options.sourceDir, options.themes.header) }
      : {}),
  };
}
