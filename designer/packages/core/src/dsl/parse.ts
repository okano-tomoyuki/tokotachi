import { migrateFormat, type FormatMigrations } from '../format.ts';
import type { Diagnostic } from './diagnostics.ts';
import { renameOldMembers } from './renamed.ts';
import { FORMAT_VERSION, TtuiDocument } from './schema.ts';
import { validateDocument } from './validate.ts';

export interface ParseResult {
  /** 構造の検証を通った場合だけある(意味の検証でエラーがあってもある)。 */
  readonly document?: TtuiDocument;
  readonly diagnostics: readonly Diagnostic[];
}

/**
 * 画面の定義ファイルの、前の版から移す関数(docs/adr/0064)。形式を変えたら FORMAT_VERSION を上げ、ここに足す。
 * 例: `1: (json) => { ... }` は版 1 のファイルを版 2 の形にする。
 */
export const TTUI_MIGRATIONS: FormatMigrations = {};

/** *.ttui.json のテキストを読み、古い版なら今の版に移し、構造と意味を検証する。 */
export function parseDocument(text: string): ParseResult {
  let json: unknown;
  try {
    json = JSON.parse(text);
  } catch (e) {
    const message = e instanceof Error ? e.message : String(e);
    return { diagnostics: [{ severity: 'error', code: 'json-syntax', message, path: [] }] };
  }

  const migrated = migrateFormat(json, FORMAT_VERSION, TTUI_MIGRATIONS);
  if (migrated.some((d) => d.severity === 'error')) {
    return { diagnostics: migrated };
  }

  // 改名したプロパティ・ハンドラの古い名前は、新しい名前に読み替える
  renameOldMembers(json);
  const result = TtuiDocument.safeParse(json);
  if (!result.success) {
    return {
      diagnostics: result.error.issues.map((issue) => ({
        severity: 'error',
        code: 'schema',
        message: issue.message,
        path: issue.path.map((key) => (typeof key === 'symbol' ? String(key) : key)),
      })),
    };
  }
  return { document: result.data, diagnostics: [...migrated, ...validateDocument(result.data)] };
}
