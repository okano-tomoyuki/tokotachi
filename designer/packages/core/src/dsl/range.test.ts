import { describe, expect, it } from 'vitest';
import { findClass } from '../catalog/catalog.ts';
import { parseDocument } from './parse.ts';
import { clampToRange, describeRange, propertyRange } from './range.ts';

function range(cls: string, name: string, node: unknown) {
  const info = findClass(cls)?.properties[name];
  if (!info) {
    throw new Error(`${cls}.${name}`);
  }
  return propertyRange(cls, info, node);
}

describe('range', () => {
  it('数値・同じ部品のプロパティ(書いていなければ既定値)・配列の数で端を決める', () => {
    expect(range('SpinBox', 'decimals', {})).toEqual({ min: 0, max: 10 });
    expect(range('Slider', 'value', {})).toEqual({ min: 0, max: 100 });
    expect(range('Slider', 'value', { properties: { minimum: 10, maximum: 20 } })).toEqual({
      min: 10,
      max: 20,
    });
    // minimum を maximum より大きくすると、実行するとき maximum も同じ値になる
    expect(range('Slider', 'value', { properties: { minimum: 30, maximum: 20 } })).toEqual({
      min: 30,
      max: 30,
    });
    // Variable と結び付けていれば、その端は決まらない
    expect(range('Slider', 'value', { properties: { maximum: { bind: 'v' } } })).toEqual({
      min: 0,
    });
    expect(range('ComboBox', 'current_index', { properties: { items: ['a', 'b'] } })).toEqual({
      min: -1,
      max: 1,
    });
    expect(range('ComboBox', 'current_index', {})).toEqual({ min: -1, max: -1 });
    expect(range('TabWidget', 'current_index', { tabs: [{}, {}, {}] })).toEqual({
      min: -1,
      max: 2,
    });
    expect(range('Label', 'text', {})).toBeUndefined();
  });

  it('値を範囲に収め、範囲を説明する', () => {
    expect(clampToRange(12, { min: 0, max: 10 })).toBe(10);
    expect(clampToRange(-3, { min: 0 })).toBe(0);
    expect(clampToRange(5, undefined)).toBe(5);
    expect(describeRange({ min: 0, max: 10 })).toBe('0 to 10');
    expect(describeRange({ min: 8 })).toBe('8 or more');
    expect(describeRange({})).toBeUndefined();
  });

  it('範囲の外の値を警告する', () => {
    const doc = {
      format_version: 1,
      window: {
        name: 'MainWindow',
        class: 'Window',
        layout: {
          class: 'BoxLayout',
          items: [
            {
              widget: {
                name: 'slider_volume',
                class: 'Slider',
                properties: { maximum: 50, value: 80 },
              },
            },
            {
              widget: {
                name: 'combo_box_kind',
                class: 'ComboBox',
                properties: { items: ['a'], current_index: 0 },
              },
            },
          ],
        },
      },
    };
    const { diagnostics } = parseDocument(JSON.stringify(doc));
    const ranges = diagnostics.filter((d) => d.code === 'out-of-range');
    expect(ranges.map((d) => [d.severity, d.path.join('.')])).toEqual([
      ['warning', 'window.layout.items.0.widget.properties.value'],
    ]);
  });
});
