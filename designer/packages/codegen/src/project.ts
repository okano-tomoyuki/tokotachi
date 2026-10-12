/**
 * プロジェクトのコード生成(docs/designer/project-spec.md §5)。
 * - ウィンドウの生成の設定を、ウィンドウが属するプロジェクトの codegen から決める。
 * - 起動部分(main.cpp)と CMakeLists.txt を生成する。CMakeLists.txt は最初に雛形を作り、その後はソースの一覧の区間だけを置き換える
 *   (利用者が書いた部分は変えない)。
 */
import {
  dirname,
  imageReferences,
  normalizePath,
  relativePath,
  resolvePath,
  type TtprojDocument,
  type TtuiDocument,
  PROJECT_EXTENSION,
  parseThemeOverride,
} from '@tokotachi-designer/core';
import { cppSyntax } from './cpp/syntax.ts';
import { namespaceBegin, namespaceEnd } from './cpp/emit.ts';
import { cppString, enumValue, themeFunction, themeOverrideValue } from './cpp/values.ts';
import type { GenerateResult, OutputFile } from './index.ts';
import {
  DEFAULT_CPP_OPTIONS,
  fileNameOf,
  resolveCppTarget,
  THEMES_HEADER,
  type CppOptions,
} from './names.ts';
import { createFile, mergeFile, type GeneratedCode, type LanguageSyntax } from './region.ts';

/** 仮の絶対パスの根(プロジェクトのフォルダ)。core のパスの計算は絶対パスを扱う。 */
const ROOT = '/project';

/** プロジェクトのフォルダから見たパスを、ウィンドウのフォルダから見たパスにする(同じなら undefined)。 */
function fromWindow(windowPath: string, dir: string | undefined): string | undefined {
  if (dir === undefined || normalizePath(dir) === '') {
    return undefined;
  }
  const relative = relativePath(dirname(resolvePath(ROOT, windowPath)), resolvePath(ROOT, dir));
  return relative === '' ? undefined : relative;
}

/**
 * プロジェクトに属するウィンドウの、コード生成の設定。
 * @param windowPath プロジェクトのファイルのフォルダからの、ウィンドウの定義ファイルの相対パス
 */
export function windowCppOptions(project: TtprojDocument, windowPath: string): CppOptions {
  const c = project.codegen ?? {};
  const namespace = c.namespace?.trim() ?? '';
  return {
    ...DEFAULT_CPP_OPTIONS,
    namespace: namespace === '' ? [] : namespace.split('::'),
    includeGuardPrefix: c.include_guard_prefix ?? '',
    headerDir: fromWindow(windowPath, c.header_dir),
    sourceDir: fromWindow(windowPath, c.source_dir),
    commentLocale: c.comment_locale ?? 'en',
    projectDir: relativePath(dirname(resolvePath(ROOT, windowPath)), ROOT),
    themes: {
      presets: project.theme?.presets ?? {},
      header: relativePath(
        dirname(resolvePath(ROOT, windowPath)),
        resolvePath(ROOT, themesHeaderPath(project)),
      ),
    },
  };
}

/** 登録したテーマの関数を置くヘッダの、プロジェクトのフォルダからのパス(header_dir か、プロジェクトのフォルダ)。 */
export function themesHeaderPath(project: TtprojDocument): string {
  const dir = normalizePath(project.codegen?.header_dir ?? '');
  return dir === '' ? THEMES_HEADER : `${dir}/${THEMES_HEADER}`;
}

/** generateProject が出力するファイル(プロジェクトのフォルダからの相対パス)。テーマを登録していなければテーマのヘッダは無い。 */
export function projectOutputPaths(project: TtprojDocument): string[] {
  return [
    'main.cpp',
    'CMakeLists.txt',
    ...(Object.keys(project.theme?.presets ?? {}).length > 0 ? [themesHeaderPath(project)] : []),
  ];
}

/** 登録したテーマの関数のヘッダ(project-spec.md §4.2)。テーマを登録していなければ作らない。 */
function themesCode(project: TtprojDocument, comments: Comments): GeneratedCode | undefined {
  const presets = Object.entries(project.theme?.presets ?? {});
  if (presets.length === 0) {
    return undefined;
  }
  const c = project.codegen ?? {};
  const namespace =
    (c.namespace?.trim() ?? '') === '' ? [] : (c.namespace ?? '').trim().split('::');
  const guard = `${c.include_guard_prefix ?? ''}${THEMES_HEADER}`
    .toUpperCase()
    .replace(/[^A-Z0-9]/g, '_');
  const functions = presets.map(([name, value]) =>
    [
      `inline tokotachi::ThemeOverride ${themeFunction(name)}`,
      '{',
      `    return ${themeOverrideValue(value)};`,
      '}',
    ].join('\n'),
  );
  return {
    regions: [
      {
        id: 'tokotachi_themes',
        indent: 0,
        content: [
          `#ifndef ${guard}`,
          `#define ${guard}`,
          '',
          '#include <tokotachi/tokotachi.hpp>',
          '',
          ...(namespace.length > 0 ? [namespaceBegin(namespace), ''] : []),
          functions.join('\n\n'),
          ...(namespace.length > 0 ? ['', namespaceEnd(namespace)] : []),
          '',
          `#endif  // ${guard}`,
        ].join('\n'),
      },
    ],
    stubs: [],
    scaffold: (rendered) => [`// ${comments.themes}`, rendered('tokotachi_themes'), ''].join('\n'),
  };
}

/** プロジェクトの名前(ファイルの名前から。`my_app.ttproj.json` → `my_app`)。 */
export function projectName(projectFileName: string): string {
  const name = fileNameOf(projectFileName);
  return name.endsWith(PROJECT_EXTENSION) ? name.slice(0, -PROJECT_EXTENSION.length) : name;
}

/** プロジェクトのウィンドウ(パスはプロジェクトのフォルダからの相対パス)。 */
export interface ProjectWindow {
  readonly path: string;
  readonly doc: TtuiDocument;
}

/** ウィンドウのヘッダ・ソースの、プロジェクトのフォルダからのパス。 */
function windowFiles(
  project: TtprojDocument,
  w: ProjectWindow,
): { header: string; source: string; images: string | undefined } {
  const target = resolveCppTarget(w.doc, fileNameOf(w.path), windowCppOptions(project, w.path));
  const dir = dirname(resolvePath(ROOT, w.path));
  return {
    header: relativePath(ROOT, resolvePath(dir, target.header)),
    source: relativePath(ROOT, resolvePath(dir, target.source)),
    // 画像を参照するウィンドウは、画像のソースも生成する(docs/adr/0053)
    images:
      imageReferences(w.doc).length > 0
        ? relativePath(ROOT, resolvePath(dir, target.images))
        : undefined,
  };
}

const CMAKE_SYNTAX: LanguageSyntax = {
  comment: '#',
  indentUnit: '    ',
  blankLinesBeforeAppended: 1,
  hasHandler: () => true,
  fallbackStubLine: (lines) => lines.length,
};

interface Comments {
  readonly main: string;
  readonly findPackage: readonly string[];
  readonly sources: string;
  readonly themes: string;
}

const COMMENTS: Readonly<Record<'en' | 'ja', Comments>> = {
  en: {
    main: 'Startup code created with the Tokotachi designer. Regions enclosed in markers are overwritten when regenerated.',
    findPackage: [
      '# Tokotachi (pass the install location with -DCMAKE_PREFIX_PATH=<prefix>,',
      '# or use FetchContent / add_subdirectory instead)',
    ],
    sources: '# Sources generated by the Tokotachi designer (overwritten when regenerated)',
    themes:
      'Themes registered in the project, created with the Tokotachi designer. Regions enclosed in markers are overwritten when regenerated.',
  },
  // l10n-ignore(生成するコードのコメントの定型文)
  ja: {
    // l10n-ignore
    main: 'Tokotachi のデザイナーで作った起動部分。マーカーで囲んだ区間は、生成し直すと上書きされる。',
    findPackage: [
      // l10n-ignore
      '# Tokotachi(インストールした場所は -DCMAKE_PREFIX_PATH=<prefix> で渡す。',
      // l10n-ignore
      '# FetchContent・add_subdirectory に書き換えてもよい)',
    ],
    // l10n-ignore
    sources: '# デザイナーが生成するソース(生成し直すと上書きされる)',
    themes:
      // l10n-ignore
      'Tokotachi のデザイナーで作った、プロジェクトに登録したテーマ。マーカーで囲んだ区間は、生成し直すと上書きされる。',
  },
};

/**
 * 起動部分(main.cpp)と CMakeLists.txt を生成する。
 * @param readExisting プロジェクトのフォルダからのパスの、既存の内容(無ければ undefined)
 */
/** プロジェクトのテーマを Application に設定する文(project-spec.md §4.1)。 */
function themeStatements(project: TtprojDocument): string[] {
  const out: string[] = [];
  const scheme = project.theme?.color_scheme;
  if (scheme !== undefined) {
    out.push(`    app.color_scheme = ${enumValue('ColorScheme', scheme)};`);
  }
  const override = project.theme?.override;
  const parsed = parseThemeOverride(override);
  if (parsed.colors.length > 0 || parsed.dimensions.length > 0) {
    // 標準のライト・ダークに同じ上書きを重ねる(色はライト・ダークで別にできる)
    out.push(
      `    const tokotachi::ThemeOverride theme_override = ${themeOverrideValue(override)};`,
    );
    for (const kind of ['light', 'dark'] as const) {
      out.push(
        `    tokotachi::Theme ${kind}_theme = tokotachi::Theme::${kind}();`,
        `    theme_override.apply_to(${kind}_theme);`,
        `    app.${kind}_theme = ${kind}_theme;`,
      );
    }
  }
  return out;
}

export function generateProject(
  project: TtprojDocument,
  projectFileName: string,
  windows: readonly ProjectWindow[],
  readExisting: (path: string) => string | undefined,
): GenerateResult | { readonly error: string } {
  const name = projectName(projectFileName);
  const comments = COMMENTS[project.codegen?.comment_locale ?? 'en'];
  const main = windows.find(
    (w) =>
      project.main_window !== undefined &&
      normalizePath(w.path) === normalizePath(project.main_window),
  );
  if (!main) {
    return { error: 'Set main_window in the project file' };
  }
  if (main.doc.window.class !== 'Window') {
    return { error: 'The main window must be a Window (not a Dialog or a component file)' };
  }
  const options = windowCppOptions(project, main.path);
  const qualified = [...options.namespace, main.doc.window.name].join('::');
  const files = windows.map((w) => windowFiles(project, w));
  const mainFiles = windowFiles(project, main);

  const mainCode: GeneratedCode = {
    regions: [
      {
        id: 'tokotachi_includes',
        indent: 0,
        content: [
          '#include <tokotachi/tokotachi.hpp>',
          '',
          `#include ${cppString(mainFiles.header)}`,
        ].join('\n'),
      },
      {
        id: 'tokotachi_main',
        indent: 1,
        content: [
          '    tokotachi::Application app(argc, argv);',
          ...themeStatements(project),
          `    ${qualified} window;`,
          '    window.visible = true;',
          '    return app.exec();',
        ].join('\n'),
      },
    ],
    stubs: [],
    scaffold: (rendered) =>
      [
        `// ${comments.main}`,
        rendered('tokotachi_includes'),
        '',
        'int main(int argc, char** argv)',
        '{',
        rendered('tokotachi_main'),
        '}',
        '',
      ].join('\n'),
  };

  const includeDirs = [...new Set(files.map((f) => dirname(resolvePath(ROOT, f.header))))].map(
    (d) => (d === ROOT ? '.' : relativePath(ROOT, d)),
  );
  const cmakeCode: GeneratedCode = {
    regions: [
      {
        id: 'tokotachi_sources',
        indent: 0,
        content: [
          comments.sources,
          'set(TOKOTACHI_DESIGNER_SOURCES',
          '    main.cpp',
          ...files.flatMap((f) => [
            `    ${f.source}`,
            ...(f.images === undefined ? [] : [`    ${f.images}`]),
          ]),
          ')',
          'set(TOKOTACHI_DESIGNER_INCLUDE_DIRS',
          ...includeDirs.map((d) => `    ${d}`),
          ')',
        ].join('\n'),
      },
    ],
    stubs: [],
    scaffold: (rendered) =>
      [
        'cmake_minimum_required(VERSION 3.16)',
        `project(${name} LANGUAGES CXX)`,
        '',
        ...comments.findPackage,
        'find_package(tokotachi 0.1 REQUIRED)',
        '',
        rendered('tokotachi_sources'),
        '',
        `add_executable(${name} \${TOKOTACHI_DESIGNER_SOURCES})`,
        `target_include_directories(${name} PRIVATE \${TOKOTACHI_DESIGNER_INCLUDE_DIRS})`,
        `target_compile_features(${name} PRIVATE cxx_std_11)`,
        `target_link_libraries(${name} PRIVATE tokotachi::tokotachi)`,
        '',
      ].join('\n'),
  };

  const themes = themesCode(project, comments);
  const generate = (path: string, code: GeneratedCode, syntax: LanguageSyntax): OutputFile => {
    const existing = readExisting(path);
    return {
      path,
      result:
        existing === undefined
          ? { ok: true, text: createFile(code, syntax), modifiedRegions: [], addedStubs: [] }
          : mergeFile(existing, code, syntax),
    };
  };
  return {
    files: [
      generate('main.cpp', mainCode, cppSyntax(main.doc.window.name)),
      generate('CMakeLists.txt', cmakeCode, CMAKE_SYNTAX),
      ...(themes ? [generate(themesHeaderPath(project), themes, cppSyntax(''))] : []),
    ],
    warnings: [],
  };
}
