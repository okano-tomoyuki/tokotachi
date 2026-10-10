/**
 * コード生成の CLI(docs/designer/codegen-design.md。Bethany の beth から流用)。
 *
 *   ttd generate <file.ttui.json> [--force] [--check] [--locale <ja|en>]
 *                [--comment-locale <ja|en>] [--namespace <a::b>] [--header-dir <dir>] [--source-dir <dir>]
 *
 * - 定義ファイルから C++ のヘッダとソースを生成する。既存のファイルはマーカーの区間だけを置き換える。
 * - 手で変えた区間があれば書かずに失敗する(--force で上書き)。
 * - --check は書かず、生成結果が既存のファイルと同じか(最新か)だけを調べる(CI 向け)。
 * - 生成の設定は、プロジェクトファイル(*.ttproj.json)を作るまでは引数で渡す(codegen-design.md §8)。
 * - メッセージの言語は --locale、無ければ環境変数(LC_ALL・LC_MESSAGES・LANG)、無ければ OS の設定で決まる。
 */
import { mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { basename, dirname, resolve } from 'node:path';
import { parseArgs } from 'node:util';
import {
  DEFAULT_CPP_OPTIONS,
  generateCpp,
  type CommentLocale,
  type CppOptions,
} from '@tokotachi-designer/codegen';
import {
  configureL10n,
  hasErrors,
  isValidIdentifier,
  l10n,
  parseDocument,
  type Diagnostic,
} from '@tokotachi-designer/core';

const USAGE =
  'Usage: ttd generate <file.ttui.json> [--force] [--check] [--locale <ja|en>] ' +
  '[--comment-locale <ja|en>] [--namespace <a::b>] [--header-dir <dir>] [--source-dir <dir>]';

/** メッセージの言語(--locale、環境変数、OS の設定の順)。 */
function detectLanguage(option: string | undefined): string {
  const env = process.env;
  return (
    option ??
    (env['LC_ALL'] || env['LC_MESSAGES'] || env['LANG'] || undefined) ??
    Intl.DateTimeFormat().resolvedOptions().locale
  );
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
  // 訳(packages/extension/l10n)は拡張を作るときに用意する。それまでは英語
  configureL10n(detectLanguage(values.locale), undefined);
  const [command, file] = positionals;
  if (command !== 'generate' || !file) {
    console.error(USAGE);
    return 1;
  }
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
  const options: CppOptions = {
    ...DEFAULT_CPP_OPTIONS,
    namespace,
    headerDir: values['header-dir'],
    sourceDir: values['source-dir'],
    commentLocale: commentLocale satisfies CommentLocale,
  };

  const dslPath = resolve(file);
  const outPath = (path: string): string => resolve(dirname(dslPath), path);
  const { document, diagnostics } = parseDocument(readFileSync(dslPath, 'utf8'));
  report(file, diagnostics);
  if (!document || hasErrors(diagnostics)) {
    return 1;
  }
  const generated = generateCpp(
    document,
    basename(dslPath),
    (path) => readIfExists(outPath(path)),
    options,
  );
  for (const warning of generated.warnings) {
    console.error(`warning: ${warning}`);
  }
  let status = 0;
  for (const { path, result } of generated.files) {
    if (!result.ok) {
      console.error(`${path}: ${result.error}`);
      status = 1;
    }
  }
  if (status !== 0) {
    return status;
  }

  const files = generated.files.flatMap(({ path, result }) =>
    result.ok ? [{ path, result }] : [],
  );
  const existing = new Map(files.map((f) => [f.path, readIfExists(outPath(f.path))]));
  const changed = files.filter((f) => existing.get(f.path) !== f.result.text);

  if (values.check) {
    for (const f of changed) {
      console.error(l10n.t('{0}: not up to date (run ttd generate)', f.path));
    }
    return changed.length > 0 ? 1 : 0;
  }

  const modified = files.filter((f) => f.result.modifiedRegions.length > 0);
  if (modified.length > 0 && !values.force) {
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

process.exitCode = main(process.argv.slice(2));
