/**
 * 定義ファイルの形式の版(format_version)の移し替え(docs/adr/0064)。
 *
 * 形式を変えるときは版を 1 つ上げ、前の版から移す関数を MIGRATIONS に足す(項目を足すだけなら何もしない関数でよい)。
 * 読むときに、ファイルの版から今の版まで順に移してから検証する。移す関数は JSON のまま扱う。
 */
import { l10n } from './l10n.ts';
import type { Diagnostic } from './dsl/diagnostics.ts';

/** 版 n のファイルの JSON を、版 n + 1 の形にその場で直す。 */
export type FormatMigration = (json: Record<string, unknown>) => void;

/** 版 n から n + 1 へ移す関数(キーは移す前の版)。 */
export type FormatMigrations = Readonly<Record<number, FormatMigration>>;

/**
 * json の format_version を current まで移す(その場で直す)。
 * - 版が無い・数でないときは何もしない(構造の検証が知らせる)。
 * - current より新しい・移せない(古すぎる)版ならエラーを返す(json は変えない)。
 * - 移したら、そのことを知らせる警告を返す(保存すると今の版で書き出す)。
 */
export function migrateFormat(
  json: unknown,
  current: number,
  migrations: FormatMigrations,
): Diagnostic[] {
  if (typeof json !== 'object' || json === null || Array.isArray(json)) {
    return [];
  }
  const doc = json as Record<string, unknown>;
  const version = doc['format_version'];
  if (typeof version !== 'number' || version === current) {
    return [];
  }
  if (Number.isInteger(version) && version > current) {
    return [
      unsupported(
        l10n.t(
          'This file was made by a newer Tokotachi Designer (format_version {0}). Update the designer to open it (supported: up to {1}).',
          String(version),
          current,
        ),
      ),
    ];
  }
  // 古い版: 移す関数がそろっているかを先に確かめる(途中まで直したまま返さない)
  const steps: FormatMigration[] = [];
  for (let v = version; v < current; ++v) {
    const step = Number.isInteger(v) ? migrations[v] : undefined;
    if (step === undefined) {
      return [
        unsupported(
          l10n.t('format_version {0} is not supported (supported: {1})', String(version), current),
        ),
      ];
    }
    steps.push(step);
  }
  steps.forEach((step, i) => {
    step(doc);
    doc['format_version'] = version + i + 1;
  });
  return [
    {
      severity: 'warning',
      code: 'migrated-format',
      message: l10n.t(
        'Converted from format_version {0}. Saving writes format_version {1}, which older designers cannot open.',
        String(version),
        current,
      ),
      path: ['format_version'],
    },
  ];
}

function unsupported(message: string): Diagnostic {
  return { severity: 'error', code: 'unsupported-version', message, path: ['format_version'] };
}
