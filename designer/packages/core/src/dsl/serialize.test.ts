import { describe, expect, it } from 'vitest';
import raw from '../../../../samples/main_window.ttui.json?raw';
import { sampleCopy, SAMPLE } from '../testing.ts';
import { TtuiDocument } from './schema.ts';
import { serializeDocument } from './serialize.ts';

describe('serializeDocument', () => {
  it('見本は決まった形で書かれている', () => {
    expect(serializeDocument(SAMPLE)).toBe(raw.replace(/\r\n/g, '\n'));
  });

  it('キーとプロパティの順を直す', () => {
    const doc = sampleCopy();
    // キーを逆の順にする
    doc.window = Object.fromEntries(Object.entries(doc.window).reverse());
    doc.window.properties = { title: 'x', height: 2, width: 1 };
    const text = serializeDocument(TtuiDocument.parse(doc));
    expect(text.indexOf('"name": "MainWindow"')).toBeLessThan(text.indexOf('"menu_bar"'));
    expect(text).toContain('"width": 1,\n      "height": 2,\n      "title": "x"');
  });

  it('値の型のメンバの順と、レイアウトの項目の widget は最後', () => {
    const doc = sampleCopy();
    doc.window.layout.properties.margins = { bottom: 4, left: 1 };
    const text = serializeDocument(TtuiDocument.parse(doc));
    expect(text).toContain('"margins": { "left": 1, "bottom": 4 }');
    expect(text).toMatch(/\{\n {10}"stretch": 1,\n {10}"widget": \{/);
  });

  it('改行は LF、末尾に改行', () => {
    const text = serializeDocument(SAMPLE);
    expect(text.includes('\r')).toBe(false);
    expect(text.endsWith('}\n')).toBe(true);
  });
});
