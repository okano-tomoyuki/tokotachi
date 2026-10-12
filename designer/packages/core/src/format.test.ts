import { describe, expect, it } from 'vitest';
import { parseDocument } from './dsl/parse.ts';
import { FORMAT_VERSION } from './dsl/schema.ts';
import { migrateFormat, type FormatMigrations } from './format.ts';
import { parseProject } from './project/parse.ts';
import { PROJECT_FORMAT_VERSION } from './project/schema.ts';

// 仮の移し替え: 版 1 → 2 で title を text に改め、版 2 → 3 で items を足す。
const MIGRATIONS: FormatMigrations = {
  1: (json) => {
    json['text'] = json['title'];
    delete json['title'];
  },
  2: (json) => {
    json['items'] = [];
  },
};

describe('migrateFormat', () => {
  it('古い版から今の版まで順に移し、移したことを警告で知らせる', () => {
    const json: Record<string, unknown> = { format_version: 1, title: 'a' };
    const diagnostics = migrateFormat(json, 3, MIGRATIONS);
    expect(json).toEqual({ format_version: 3, text: 'a', items: [] });
    expect(diagnostics.map((d) => [d.severity, d.code])).toEqual([['warning', 'migrated-format']]);
  });

  it('今の版なら何もしない', () => {
    const json = { format_version: 3, text: 'a' };
    expect(migrateFormat(json, 3, MIGRATIONS)).toEqual([]);
    expect(json).toEqual({ format_version: 3, text: 'a' });
  });

  it('新しい版は開かず、デザイナーの更新を促す', () => {
    const json = { format_version: 4 };
    const diagnostics = migrateFormat(json, 3, MIGRATIONS);
    expect(diagnostics.map((d) => [d.severity, d.code])).toEqual([
      ['error', 'unsupported-version'],
    ]);
    expect(diagnostics[0]?.message).toContain('newer');
    expect(json).toEqual({ format_version: 4 });
  });

  it('移す関数の無い古い版は開かない(途中まで直さない)', () => {
    const json = { format_version: 0, title: 'a' };
    const diagnostics = migrateFormat(json, 3, MIGRATIONS);
    expect(diagnostics.map((d) => d.code)).toEqual(['unsupported-version']);
    expect(json).toEqual({ format_version: 0, title: 'a' });
  });

  it('版が数でなければ、構造の検証に任せる', () => {
    expect(migrateFormat({ format_version: '1' }, 3, MIGRATIONS)).toEqual([]);
    expect(migrateFormat({}, 3, MIGRATIONS)).toEqual([]);
    expect(migrateFormat(null, 3, MIGRATIONS)).toEqual([]);
  });
});

describe('定義ファイルの版', () => {
  it('新しい版の画面の定義ファイル・プロジェクトファイルは開かない', () => {
    const ttui = parseDocument(
      JSON.stringify({ format_version: FORMAT_VERSION + 1, window: { class: 'Window' } }),
    );
    expect(ttui.document).toBeUndefined();
    expect(ttui.diagnostics.map((d) => d.code)).toEqual(['unsupported-version']);
    const ttproj = parseProject(JSON.stringify({ format_version: PROJECT_FORMAT_VERSION + 1 }));
    expect(ttproj.project).toBeUndefined();
    expect(ttproj.diagnostics.map((d) => d.code)).toEqual(['unsupported-version']);
  });
});
