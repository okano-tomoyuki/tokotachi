import { describe, expect, it } from 'vitest';
import {
  checkThemeOverride,
  parseThemeOverride,
  replaceThemePreset,
  resolveThemeOverride,
  resolveThemePresets,
  themePresetReferences,
} from './themeOverride.ts';
import { checkValue } from './values.ts';

describe('themeOverride', () => {
  it('役割ごとの色(同じ・ライトとダーク)と寸法を書ける', () => {
    const value = {
      colors: { FIELD: { light: '#FFFFFF', dark: '#000000' }, ACCENT: '#DC2626' },
      radius: 0,
      field_padding: 8,
    };
    expect(checkThemeOverride(value)).toBeNull();
    expect(checkValue({ value: 'ThemeOverride' }, value)).toBeNull();
    expect(checkThemeOverride({})).toBeNull();
    // 色は役割の順(ACCENT が FIELD より先)、寸法は決まった順
    expect(parseThemeOverride(value)).toEqual({
      colors: [
        { role: 'ACCENT', light: '#DC2626', dark: '#DC2626' },
        { role: 'FIELD', light: '#FFFFFF', dark: '#000000' },
      ],
      dimensions: [
        { name: 'radius', value: 0 },
        { name: 'field_padding', value: 8 },
      ],
    });
  });

  it('書けないもの', () => {
    expect(checkThemeOverride('ACCENT')).not.toBeNull();
    expect(checkThemeOverride({ colors: { NOPE: '#000000' } })).not.toBeNull();
    expect(checkThemeOverride({ colors: { ACCENT: 'red' } })).not.toBeNull();
    expect(checkThemeOverride({ colors: { ACCENT: { light: '#000000' } } })).not.toBeNull();
    expect(checkThemeOverride({ radius: -1 })).not.toBeNull();
    expect(checkThemeOverride({ spacing: 4 })).not.toBeNull();
  });

  it('登録したテーマの参照(preset)は部品だけに書け、展開すると部品に書いたものが重なる', () => {
    expect(checkThemeOverride({ preset: 'danger', radius: 2 })).toBeNull();
    expect(checkThemeOverride({ preset: 'danger' }, false)).not.toBeNull();
    expect(checkThemeOverride({ preset: '1x' })).not.toBeNull();
    const presets = { danger: { colors: { ACCENT: '#DC2626', FIELD: '#FEF2F2' }, radius: 0 } };
    expect(
      resolveThemeOverride(
        { preset: 'danger', colors: { ACCENT: '#B91C1C' }, field_padding: 4 },
        presets,
      ),
    ).toEqual({ colors: { ACCENT: '#B91C1C', FIELD: '#FEF2F2' }, radius: 0, field_padding: 4 });
    expect(resolveThemeOverride({ preset: 'none' }, presets)).toBeUndefined();
    expect(parseThemeOverride({ preset: 'danger' }).preset).toBe('danger');
  });

  it('定義ファイルの中の参照を探し、キャンバスのために展開する', () => {
    const doc = {
      format_version: 1,
      window: {
        name: 'MainWindow',
        class: 'Window',
        children: [
          {
            name: 'button_ok',
            class: 'Button',
            properties: { theme_override: { preset: 'danger' } },
          },
          {
            name: 'button_no',
            class: 'Button',
            properties: { theme_override: { preset: 'none', radius: 3 } },
          },
        ],
      },
    } as never;
    expect(themePresetReferences(doc).map((r) => [r.name, r.at.join('.')])).toEqual([
      ['danger', 'window.children.0.properties.theme_override.preset'],
      ['none', 'window.children.1.properties.theme_override.preset'],
    ]);
    const resolved = resolveThemePresets(doc, { danger: { radius: 0 } }) as never as {
      window: { children: { properties: Record<string, unknown> }[] };
    };
    expect(resolved.window.children[0]?.properties['theme_override']).toEqual({ radius: 0 });
    // 登録していない名前は、参照を外して部品に書いたものだけで描く
    expect(resolved.window.children[1]?.properties['theme_override']).toEqual({ radius: 3 });
  });

  it('参照を改名する・外す(外して何も残らなければ theme_override ごと消す)', () => {
    const doc = {
      format_version: 1,
      window: {
        name: 'MainWindow',
        class: 'Window',
        children: [
          {
            name: 'button_ok',
            class: 'Button',
            properties: { text: 'OK', theme_override: { preset: 'danger' } },
          },
          {
            name: 'button_no',
            class: 'Button',
            properties: { theme_override: { preset: 'danger', radius: 3 } },
          },
          {
            name: 'button_other',
            class: 'Button',
            properties: { theme_override: { preset: 'x' } },
          },
        ],
      },
    } as never;
    type Doc = { window: { children: { properties: Record<string, unknown> }[] } };
    const renamed = replaceThemePreset(doc, 'danger', 'alert');
    expect(renamed.count).toBe(2);
    expect(themePresetReferences(renamed.document).map((r) => r.name)).toEqual([
      'alert',
      'alert',
      'x',
    ]);
    // 元は変えない
    expect(themePresetReferences(doc).map((r) => r.name)).toEqual(['danger', 'danger', 'x']);
    const removed = replaceThemePreset(doc, 'danger', undefined).document as never as Doc;
    expect(removed.window.children[0]?.properties).toEqual({ text: 'OK' });
    expect(removed.window.children[1]?.properties['theme_override']).toEqual({ radius: 3 });
    expect(replaceThemePreset(doc, 'nothing', 'y')).toEqual({ document: doc, count: 0 });
  });
});
