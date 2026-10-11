import { describe, expect, it } from 'vitest';
import { parseProject, serializeProject } from './parse.ts';

const valid = {
  format_version: 1,
  codegen: { comment_locale: 'ja', namespace: 'app::ui', header_dir: 'include' },
  main_window: 'main_window.ttui.json',
  windows: ['main_window.ttui.json', 'dialogs/settings_dialog.ttui.json'],
};

const codes = (value: unknown): string[] =>
  parseProject(JSON.stringify(value)).diagnostics.map((d) => d.path.join('.'));

describe('parseProject', () => {
  it('正しいプロジェクト', () => {
    expect(parseProject(JSON.stringify(valid)).diagnostics).toEqual([]);
  });

  it('ウィンドウのパス・重複・メインのウィンドウ・名前空間・フォルダ', () => {
    expect(
      codes({
        format_version: 1,
        codegen: { namespace: 'app::1ui', header_dir: 'C:/abs', include_guard_prefix: '9X' },
        main_window: 'other.ttui.json',
        windows: ['a.ttui.json', './a.ttui.json', 'b.json', 'x/a.ttui.json'],
      }),
    ).toEqual([
      'windows.1',
      'windows.2',
      'main_window',
      'codegen.namespace',
      'codegen.include_guard_prefix',
      'codegen.header_dir',
      'windows.1',
      'windows.3',
    ]);
  });

  it('決まった形で書き出す', () => {
    const text = serializeProject({ ...valid, format_version: 1 } as never);
    expect(text).toBe(`{
  "format_version": 1,
  "codegen": {
    "comment_locale": "ja",
    "namespace": "app::ui",
    "header_dir": "include"
  },
  "main_window": "main_window.ttui.json",
  "windows": [
    "main_window.ttui.json",
    "dialogs/settings_dialog.ttui.json"
  ]
}
`);
  });

  it('テーマ: 配色と上書きを調べ、決まった形で書き出す', () => {
    const theme = {
      color_scheme: 'DARK',
      override: {
        colors: { ACCENT: '#DC2626', FIELD: { light: '#FFFFFF', dark: '#000000' } },
        radius: 0,
      },
    };
    expect(parseProject(JSON.stringify({ ...valid, theme })).diagnostics).toEqual([]);
    expect(codes({ ...valid, theme: { color_scheme: 'AUTO' } })).toEqual(['theme.color_scheme']);
    expect(
      codes({ ...valid, theme: { override: { colors: { NOPE: '#000000' } } } }).length,
    ).toBeGreaterThan(0);
    expect(codes({ ...valid, theme: { override: { radius: -1 } } }).length).toBeGreaterThan(0);
    const text = serializeProject({ format_version: 1, theme } as never);
    expect(text).toBe(`{
  "format_version": 1,
  "theme": {
    "color_scheme": "DARK",
    "override": {
      "colors": {
        "ACCENT": "#DC2626",
        "FIELD": {
          "light": "#FFFFFF",
          "dark": "#000000"
        }
      },
      "radius": 0
    }
  }
}
`);
  });
});
