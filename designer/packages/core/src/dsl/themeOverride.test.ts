import { describe, expect, it } from 'vitest';
import { checkThemeOverride, parseThemeOverride } from './themeOverride.ts';
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
});
