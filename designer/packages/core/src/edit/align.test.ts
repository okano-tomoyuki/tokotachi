import { describe, expect, it } from 'vitest';
import type { TtuiDocument } from '../dsl/schema.ts';
import { alignCommands, canAlign, type AlignTarget } from './align.ts';
import { applyCommand } from './commands.ts';
import { valueAt } from './structure.ts';

/** 自由な配置の子 3 つと、PlaceLayout の項目 1 つ、BoxLayout の項目 1 つ。 */
const DOC = {
  format_version: 1,
  window: {
    name: 'MainWindow',
    class: 'Window',
    children: [
      { name: 'label_a', class: 'Label', properties: { x: 10, y: 10 } },
      { name: 'label_b', class: 'Label', properties: { x: 50, y: 40, width: 60 } },
      { name: 'label_c', class: 'Label', properties: { x: 200, y: 70 } },
      {
        name: 'area_place',
        class: 'Widget',
        properties: { x: 0, y: 200, width: 300, height: 100 },
        layout: {
          class: 'PlaceLayout',
          items: [
            {
              placement: { relative_x: 1, x: -8, anchor: 'TOP_RIGHT' },
              widget: { name: 'label_d', class: 'Label' },
            },
          ],
        },
      },
      {
        name: 'area_box',
        class: 'Widget',
        properties: { x: 0, y: 320, width: 300, height: 40 },
        layout: { class: 'BoxLayout', items: [{ widget: { name: 'label_e', class: 'Label' } }] },
      },
    ],
  },
} as unknown as TtuiDocument;

const A: AlignTarget = {
  path: ['window', 'children', 0],
  rect: { x: 10, y: 10, width: 40, height: 20 },
};
const B: AlignTarget = {
  path: ['window', 'children', 1],
  rect: { x: 50, y: 40, width: 60, height: 20 },
};
const C: AlignTarget = {
  path: ['window', 'children', 2],
  rect: { x: 200, y: 70, width: 40, height: 20 },
};
// PlaceLayout の右上に置いた部品(幅 300 の入れ物の右端から 8)
const D: AlignTarget = {
  path: ['window', 'children', 3, 'layout', 'items', 0, 'widget'],
  rect: { x: 252, y: 200, width: 40, height: 20 },
};
const E: AlignTarget = {
  path: ['window', 'children', 4, 'layout', 'items', 0, 'widget'],
  rect: { x: 0, y: 320, width: 40, height: 20 },
};

function apply(commands: ReturnType<typeof alignCommands>): TtuiDocument {
  const r = applyCommand(DOC, { kind: 'batch', commands });
  if (!r.ok) {
    throw new Error(r.error);
  }
  return r.document;
}

function props(doc: TtuiDocument, i: number): unknown {
  return valueAt(doc, ['window', 'children', i, 'properties']);
}

describe('align', () => {
  it('自由な配置の部品が 2 つ以上(等間隔は 3 つ以上)なら揃えられる', () => {
    expect(canAlign(DOC, [A.path, B.path])).toBe(true);
    expect(canAlign(DOC, [A.path, B.path], 'distribute_horizontal')).toBe(false);
    expect(canAlign(DOC, [A.path, E.path])).toBe(false);
    expect(canAlign(DOC, [A.path, D.path])).toBe(true);
  });

  it('主に選んでいる部品に左・右・中央を揃える', () => {
    const left = apply(alignCommands(DOC, [A, B, C], 1, 'left'));
    expect(props(left, 0)).toEqual({ x: 50, y: 10 });
    expect(props(left, 1)).toEqual({ x: 50, y: 40, width: 60 });
    expect(props(left, 2)).toEqual({ x: 50, y: 70 });
    const right = apply(alignCommands(DOC, [A, B], 1, 'right'));
    expect(props(right, 0)).toEqual({ x: 70, y: 10 });
    const center = apply(alignCommands(DOC, [A, B], 1, 'center_vertical'));
    expect(props(center, 0)).toEqual({ x: 10, y: 40 });
  });

  it('両端をそのままにして等間隔に並べ、大きさを揃える', () => {
    const doc = apply(alignCommands(DOC, [C, A, B], 0, 'distribute_horizontal'));
    // 全体 10〜240、部品の幅の合計 140、空き 90 を 2 つに分ける
    expect(props(doc, 0)).toEqual({ x: 10, y: 10 });
    expect(props(doc, 1)).toEqual({ x: 95, y: 40, width: 60 });
    expect(props(doc, 2)).toEqual({ x: 200, y: 70 });
    const width = apply(alignCommands(DOC, [A, B], 1, 'same_width'));
    // 書いていなかった幅は、揃えた幅をそのまま書く
    expect(props(width, 0)).toEqual({ x: 10, y: 10, width: 60 });
  });

  it('PlaceLayout の部品は、基準点・相対の位置によらず動かす量を x・y に足す', () => {
    const doc = apply(alignCommands(DOC, [A, D], 0, 'left'));
    expect(valueAt(doc, ['window', 'children', 3, 'layout', 'items', 0, 'placement'])).toEqual({
      relative_x: 1,
      x: -250,
      anchor: 'TOP_RIGHT',
    });
  });
});
