/**
 * コード生成の CLI(docs/designer/codegen-design.md。Bethany の beth から流用)。
 *
 *   ttd generate <file.ttui.json | file.ttproj.json> [--force] [--check] [--locale <ja|en>]
 *                [--comment-locale <ja|en>] [--namespace <a::b>] [--header-dir <dir>] [--source-dir <dir>]
 *
 * - 定義ファイルから C++ のヘッダとソースを生成する。既存のファイルはマーカーの区間だけを置き換える。
 *   ウィンドウがプロジェクトに属していれば(定義ファイルのフォルダから上へたどってプロジェクトファイルを探す)、プロジェクトの設定で生成する。
 *   属していなければ、引数の設定で生成する。
 * - プロジェクトファイルなら、そのすべてのウィンドウと、起動部分(main.cpp・CMakeLists.txt)を生成する(project-spec.md §5)。
 * - 手で変えた区間があれば書かずに失敗する(--force で上書き)。
 * - --check は書かず、生成結果が既存のファイルと同じか(最新か)だけを調べる(CI 向け)。
 * - メッセージの言語は --locale、無ければ環境変数(LC_ALL・LC_MESSAGES・LANG)、無ければ OS の設定で決まる。
 */
import { mkdirSync, readdirSync, readFileSync, writeFileSync } from 'node:fs';
import { basename, dirname, relative, resolve } from 'node:path';
import { parseArgs } from 'node:util';
import {
  DEFAULT_CPP_OPTIONS,
  generateCpp,
  generateProject,
  windowCppOptions,
  type CommentLocale,
  type CppOptions,
  type GenerateResult,
  type ProjectWindow,
} from '@tokotachi-designer/codegen';
import {
  configureL10n,
  hasErrors,
  isComponentDocument,
  isJapanese,
  isValidIdentifier,
  l10n,
  parseDocument,
  parseProject,
  PROJECT_EXTENSION,
  readCustomClasses,
  setCustomClasses,
  sameWindowPath,
  type Diagnostic,
  type TtprojDocument,
  type TtuiDocument,
} from '@tokotachi-designer/core';
import ja from '../../extension/l10n/bundle.l10n.ja.json' with { type: 'json' };

const USAGE =
  'Usage: ttd generate <file.ttui.json | file.ttproj.json> [--force] [--check] [--locale <ja|en>] ' +
  '[--comment-locale <ja|en>] [--namespace <a::b>] [--header-dir <dir>] [--source-dir <dir>]';

interface Flags {
  readonly force: boolean;
  readonly check: boolean;
}

/** メッセージの言語(--locale、環境変数、OS の設定の順)。 */
function detectLanguage(option: string | undefined): string {
  const env = process.env;
  return (
    option ??
    (env['LC_ALL'] || env['LC_MESSAGES'] || env['LANG'] || undefined) ??
    Intl.DateTimeFormat().resolvedOptions().locale
  );
}

/** プロジェクトのフォルダからの相対パス(区切りは /)。 */
function posixRelative(from: string, to: string): string {
  return relative(from, to).split('\\').join('/');
}

/** 定義ファイルのフォルダから上へたどり、そのウィンドウを含むプロジェクトファイルを探す。 */
function projectContaining(
  dslPath: string,
): { doc: TtprojDocument; path: string; windowPath: string } | undefined {
  for (let dir = dirname(dslPath); ; dir = dirname(dir)) {
    let names: string[] = [];
    try {
      names = readdirSync(dir).filter((n) => n.endsWith(PROJECT_EXTENSION));
    } catch {
      // 読めないフォルダは飛ばす
    }
    for (const name of names) {
      const text = readIfExists(resolve(dir, name));
      const doc = text === undefined ? undefined : parseProject(text).project;
      const windowPath = posixRelative(dir, dslPath);
      if (doc && (doc.windows ?? []).some((w) => sameWindowPath(w, windowPath))) {
        return { doc, path: resolve(dir, name), windowPath };
      }
    }
    if (dirname(dir) === dir) {
      return undefined;
    }
  }
}

/**
 * 置いた部品ファイル(docs/adr/0043)を読む。部品ファイルでない・誤りがあれば undefined(生成はエラーになる)。
 * プロジェクトに属していればプロジェクトの設定、属していなければ置いた側と同じ設定で生成したヘッダを include する。
 */
function readComponent(
  componentPath: string,
  options: CppOptions,
): { document: TtuiDocument; options: CppOptions } | undefined {
  const parsed = parseDocument(readIfExists(componentPath) ?? '');
  if (!parsed.document || hasErrors(parsed.diagnostics) || !isComponentDocument(parsed.document)) {
    return undefined;
  }
  const owner = projectContaining(componentPath);
  return {
    document: parsed.document,
    options: owner ? windowCppOptions(owner.doc, owner.windowPath) : options,
  };
}

/**
 * プロジェクトに登録した自作部品(docs/adr/0044)を読んでカタログに足す。読めないヘッダ・プロパティは警告にする。
 * CLI は 1 つのプロジェクトだけを扱うので、カタログに足したままにする。
 */
function useProjectCustomClasses(projectPath: string, project: TtprojDocument): void {
  const dir = dirname(projectPath);
  const headers: { path: string; text: string }[] = [];
  for (const path of project.custom_widgets ?? []) {
    const text = readIfExists(resolve(dir, path));
    if (text === undefined) {
      console.error(`${projectPath}: warning: ${l10n.t('The header "{0}" is not found', path)}`);
    } else {
      headers.push({ path, text });
    }
  }
  const { classes, problems } = readCustomClasses(headers);
  for (const p of problems) {
    console.error(`${resolve(dir, p.header)}:${String(p.line)}: warning: ${p.message}`);
  }
  setCustomClasses(classes);
}

/** ウィンドウのコードを生成して書く。 */
function generateWindow(dslPath: string, options: CppOptions, flags: Flags): number {
  const text = readIfExists(dslPath);
  if (text === undefined) {
    console.error(l10n.t('"{0}" is not found', dslPath));
    return 1;
  }
  const { document, diagnostics } = parseDocument(text);
  report(dslPath, diagnostics);
  if (!document || hasErrors(diagnostics)) {
    return 1;
  }
  const outDir = dirname(dslPath);
  const generated = generateCpp(
    document,
    basename(dslPath),
    (path) => readIfExists(resolve(outDir, path)),
    options,
    (path) => readBinaryIfExists(resolve(outDir, path)),
    (path) => readComponent(resolve(outDir, path), options),
  );
  return write(generated, outDir, flags);
}

/** プロジェクトのすべてのウィンドウと、起動部分を生成して書く。 */
function generateWholeProject(projectPath: string, flags: Flags): number {
  const text = readIfExists(projectPath);
  if (text === undefined) {
    console.error(l10n.t('"{0}" is not found', projectPath));
    return 1;
  }
  const { project, diagnostics } = parseProject(text);
  report(projectPath, diagnostics);
  if (!project || hasErrors(diagnostics)) {
    return 1;
  }
  const dir = dirname(projectPath);
  useProjectCustomClasses(projectPath, project);
  let status = 0;
  const windows: ProjectWindow[] = [];
  for (const path of project.windows ?? []) {
    const dslPath = resolve(dir, path);
    status = Math.max(status, generateWindow(dslPath, windowCppOptions(project, path), flags));
    const parsed = parseDocument(readIfExists(dslPath) ?? '');
    if (parsed.document) {
      windows.push({ path, doc: parsed.document });
    }
  }
  const startup = generateProject(project, basename(projectPath), windows, (path) =>
    readIfExists(resolve(dir, path)),
  );
  if ('error' in startup) {
    console.error(`${projectPath}: ${startup.error}`);
    return 1;
  }
  return Math.max(status, write(startup, dir, flags));
}

function main(argv: readonly string[]): number {
  const { positionals, values } = parseArgs({
    args: [...argv],
    allowPositionals: true,
    options: {
      force: { type: 'boolean' },
      check: { type: 'boolean' },
      locale: { type: 'string' },
      'comment-locale': { type: 'string' },
      namespace: { type: 'string' },
      'header-dir': { type: 'string' },
      'source-dir': { type: 'string' },
    },
  });
  // 訳は拡張と共有する(packages/extension/l10n)
  const language = detectLanguage(values.locale);
  configureL10n(language, isJapanese(language) ? ja : undefined);
  const [command, file] = positionals;
  if (command !== 'generate' || !file) {
    console.error(USAGE);
    return 1;
  }
  const flags: Flags = { force: values.force === true, check: values.check === true };
  const path = resolve(file);
  if (path.endsWith(PROJECT_EXTENSION)) {
    return generateWholeProject(path, flags);
  }
  const owner = projectContaining(path);
  if (owner) {
    useProjectCustomClasses(owner.path, owner.doc);
    return generateWindow(path, windowCppOptions(owner.doc, owner.windowPath), flags);
  }
  // プロジェクトに属さなければ、引数の設定で生成する
  const commentLocale = values['comment-locale'] ?? 'en';
  if (commentLocale !== 'en' && commentLocale !== 'ja') {
    console.error(USAGE);
    return 1;
  }
  const namespace = values.namespace === undefined ? [] : values.namespace.split('::');
  if (namespace.some((n) => isValidIdentifier(n) !== null)) {
    console.error(l10n.t('Invalid namespace: {0}', values.namespace ?? ''));
    return 1;
  }
  return generateWindow(
    path,
    {
      ...DEFAULT_CPP_OPTIONS,
      namespace,
      headerDir: values['header-dir'],
      sourceDir: values['source-dir'],
      commentLocale: commentLocale satisfies CommentLocale,
    },
    flags,
  );
}

/** 生成した結果を書く(--check なら比べるだけ)。 */
function write(generated: GenerateResult, outDir: string, flags: Flags): number {
  for (const warning of generated.warnings) {
    console.error(`warning: ${warning}`);
  }
  for (const { path, result } of generated.files) {
    if (!result.ok) {
      console.error(`${path}: ${result.error}`);
      return 1;
    }
  }
  const outPath = (path: string): string => resolve(outDir, path);
  const files = generated.files.flatMap(({ path, result }) =>
    result.ok ? [{ path, result }] : [],
  );
  const existing = new Map(files.map((f) => [f.path, readIfExists(outPath(f.path))]));
  const changed = files.filter((f) => existing.get(f.path) !== f.result.text);

  if (flags.check) {
    for (const f of changed) {
      console.error(l10n.t('{0}: not up to date (run ttd generate)', f.path));
    }
    return changed.length > 0 ? 1 : 0;
  }

  const modified = files.filter((f) => f.result.modifiedRegions.length > 0);
  if (modified.length > 0 && !flags.force) {
    for (const f of modified) {
      console.error(
        l10n.t(
          '{0}: generated regions ({1}) have been edited by hand. Specify --force to overwrite them',
          f.path,
          f.result.modifiedRegions.join(', '),
        ),
      );
    }
    return 2;
  }

  for (const f of files) {
    if (!changed.includes(f)) {
      console.log(l10n.t('{0}: up to date', f.path));
      continue;
    }
    mkdirSync(dirname(outPath(f.path)), { recursive: true });
    writeFileSync(outPath(f.path), f.result.text);
    const stubs =
      f.result.addedStubs.length > 0
        ? l10n.t(' (added handler stubs: {0})', f.result.addedStubs.join(', '))
        : '';
    console.log(
      existing.get(f.path) === undefined
        ? l10n.t('{0}: created{1}', f.path, stubs)
        : l10n.t('{0}: updated{1}', f.path, stubs),
    );
  }
  return 0;
}

function report(file: string, diagnostics: readonly Diagnostic[]): void {
  for (const d of diagnostics) {
    console.error(`${file}: ${d.severity}: ${d.path.join('.') || '(root)'}: ${d.message}`);
  }
}

function readIfExists(path: string): string | undefined {
  try {
    return readFileSync(path, 'utf8');
  } catch {
    return undefined;
  }
}

function readBinaryIfExists(path: string): Uint8Array | undefined {
  try {
    return readFileSync(path);
  } catch {
    return undefined;
  }
}

process.exitCode = main(process.argv.slice(2));
