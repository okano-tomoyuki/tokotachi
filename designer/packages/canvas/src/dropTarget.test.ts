/**
 * 入れる場所の計算(webview の dropTarget)を、実際のキャンバスの描画の結果(部品の位置)で確かめる。
 * WebAssembly のモジュールが無ければ(pnpm canvas:build の前)飛ばす。
 */
import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { outline, parseDocument, type TtuiDocument } from '@tokotachi-designer/core';
import { beforeAll, describe, expect, it } from 'vitest';
import { dropTarget } from '../../webview/src/dropTarget.ts';
import { Canvas, isBuilt, type RenderResult } from './index.ts';

const sample = fileURLToPath(new URL('../../../samples/main_window.ttui.json', import.meta.url));

describe.skipIf(!isBuilt())('dropTarget', () => {
  let doc: TtuiDocument;
  let frame: RenderResult;
  beforeAll(async () => {
    const parsed = parseDocument(readFileSync(sample, 'utf8'));
    if (!parsed.document) {
      throw new Error('sample');
    }
    doc = parsed.document;
    frame = (await Canvas.load()).render(doc, { placeholders: true });
  });

  const at = (name: string) => {
    const r = frame.rects[name];
    if (!r) {
      throw new Error(name);
    }
    return r;
  };

  it('横の BoxLayout では、点に近い項目の間に入れる', () => {
    const button = at('search_button');
    // 検索のボタンの左半分 → 検索欄と検索のボタンの間(1)
    const t = dropTarget(doc, outline(doc), frame, button.x + 2, button.y + button.height / 2, {
      kind: 'new',
      class: 'Button',
    });
    expect(t?.slot).toEqual({
      kind: 'item',
      layout: ['window', 'layout', 'items', 0, 'widget', 'layout'],
      index: 1,
    });
    expect(t?.indicator.shape).toBe('line');
  });

  it('表示していないタブの中には入れない', () => {
    // 詳細のタブは表示していないので、隠れた部品の上には入れる場所が無い(外側の Splitter の区画になる)
    const edit = at('name_edit');
    const t = dropTarget(doc, outline(doc), frame, edit.x + 4, edit.y + 4, {
      kind: 'new',
      class: 'Button',
    });
    expect(t?.slot?.kind).not.toBe('item');
  });

  it('GridLayout では、空いたセルにだけ入れられる(タブを表示して)', async () => {
    const shown = (await Canvas.load()).render(doc, { placeholders: true, tabs: { tabs: 1 } });
    const grid = [
      'window',
      'layout',
      'items',
      1,
      'widget',
      'panes',
      1,
      'widget',
      'tabs',
      1,
      'page',
      'layout',
    ];
    const edit = shown.rects['name_edit'];
    const spin = shown.rects['size_spin'];
    if (!edit || !spin) {
      throw new Error('rects');
    }
    // 名前の欄の上は使われている
    const used = dropTarget(doc, outline(doc), shown, edit.x + 4, edit.y + 4, {
      kind: 'new',
      class: 'Button',
    });
    expect(used?.slot).toBeUndefined();
    expect(used?.indicator.shape).toBe('box');
    // 最後の行の下は新しい行(2 行目)
    const below = dropTarget(doc, outline(doc), shown, edit.x + 4, spin.y + spin.height + 12, {
      kind: 'new',
      class: 'Button',
    });
    expect(below?.slot).toEqual({ kind: 'item', layout: grid, index: 4, row: 2, column: 1 });
  });

  it('自分の中へは移せない', () => {
    const row = at('search_row');
    const t = dropTarget(doc, outline(doc), frame, row.x + 2, row.y + 2, {
      kind: 'move',
      path: ['window', 'layout', 'items', 0, 'widget'],
      id: 'window/layout/items/0/widget',
    });
    // search_row の中ではなく、ウィンドウのレイアウトの位置になる
    expect(t?.slot).toMatchObject({ kind: 'item', layout: ['window', 'layout'] });
  });

  it('空きは BoxLayout にだけ入れられる', () => {
    const tree = at('folder_tree');
    const t = dropTarget(doc, outline(doc), frame, tree.x + 4, tree.y + 4, {
      kind: 'spacer',
      spacer: { stretch: 1 },
    });
    expect(t?.slot).toBeUndefined();
  });
});
