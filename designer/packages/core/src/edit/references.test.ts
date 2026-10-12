import { describe, expect, it } from 'vitest';
import { hasErrors } from '../dsl/diagnostics.ts';
import type { TtuiDocument } from '../dsl/schema.ts';
import { validateDocument } from '../dsl/validate.ts';
import { copyNodes } from './clipboard.ts';
import { applyCommand, type EditCommand } from './commands.ts';
import { moveSiblings } from './menus.ts';

/** ラベルの buddy で入力欄を指す定義ファイル。 */
const DOC = {
  format_version: 1,
  window: {
    name: 'FormWindow',
    class: 'Window',
    layout: {
      class: 'BoxLayout',
      items: [
        {
          widget: {
            name: 'label_name',
            class: 'Label',
            properties: { text: '&Name', buddy: 'line_edit_name' },
          },
        },
        { widget: { name: 'line_edit_name', class: 'LineEdit' } },
        { widget: { name: 'button_ok', class: 'Button' } },
      ],
    },
  },
  actions: [{ name: 'action_open' }],
} as unknown as TtuiDocument;

const LAYOUT = ['window', 'layout'];
const item = (i: number) => [...LAYOUT, 'items', i, 'widget'];

function apply(command: EditCommand): any {
  const r = applyCommand(DOC, command);
  if (!r.ok) {
    throw new Error(r.error);
  }
  expect(hasErrors(validateDocument(r.document))).toBe(false);
  return r.document;
}

describe('参照のプロパティ', () => {
  it('参照は型の合うノードの名前で書く', () => {
    expect(hasErrors(validateDocument(DOC))).toBe(false);
    const wrong = JSON.parse(JSON.stringify(DOC));
    wrong.window.layout.items[0].widget.properties.buddy = 'action_open';
    expect(validateDocument(wrong).map((d) => d.code)).toContain('reference-type-mismatch');
    wrong.window.layout.items[0].widget.properties.buddy = 'nothing';
    expect(validateDocument(wrong).map((d) => d.code)).toContain('unknown-reference');
  });

  it('指している部品を消すと、参照も消える(ほかのプロパティは残す)', () => {
    const doc = apply({ kind: 'remove', targets: [item(1)] });
    expect(doc.window.layout.items[0].widget.properties).toEqual({ text: '&Name' });
  });

  it('指している部品を改名すると、参照も書き換わる', () => {
    const doc = apply({ kind: 'rename', target: item(1), name: 'line_edit_user' });
    expect(doc.window.layout.items[0].widget.properties.buddy).toBe('line_edit_user');
  });

  it('一緒に貼り付けると写しを指し、ラベルだけなら元の部品を指したままにする', () => {
    const slot = { kind: 'item', layout: LAYOUT, index: 3 } as const;
    const both = copyNodes(DOC, [item(0), item(1)]);
    const label = copyNodes(DOC, [item(0)]);
    if (!both || !label) {
      throw new Error('nothing copied');
    }
    const doc = apply({ kind: 'paste', items: both.items, slot });
    expect(doc.window.layout.items[3].widget.name).toBe('label_name_2');
    expect(doc.window.layout.items[3].widget.properties.buddy).toBe('line_edit_name_2');
    const one = apply({ kind: 'paste', items: label.items, slot });
    expect(one.window.layout.items[3].widget.properties.buddy).toBe('line_edit_name');
  });
});

describe('まとめて並べ替える', () => {
  const order = (doc: any) => doc.window.layout.items.map((i: any) => i.widget.name);

  it('選んだものを、それぞれ 1 つずつ動かす', () => {
    const doc = apply({ kind: 'moveSiblings', targets: [item(1), item(2)], delta: -1 });
    expect(order(doc)).toEqual(['line_edit_name', 'button_ok', 'label_name']);
  });

  it('端に着いたものと、それに続くものは動かさない', () => {
    const doc = apply({ kind: 'moveSiblings', targets: [item(0), item(1)], delta: -1 });
    expect(order(doc)).toEqual(['label_name', 'line_edit_name', 'button_ok']);
    const down = apply({ kind: 'moveSiblings', targets: [item(0), item(2)], delta: 1 });
    expect(order(down)).toEqual(['line_edit_name', 'label_name', 'button_ok']);
  });
});

describe('moveSiblings の動かした後の位置', () => {
  it('入れ物と、その中のものを一緒に動かしても、それぞれの新しい位置を返す', () => {
    const doc = {
      format_version: 1,
      window: {
        name: 'W',
        class: 'Window',
        layout: {
          class: 'BoxLayout',
          items: [
            { widget: { name: 'button_x', class: 'Button' } },
            {
              widget: {
                name: 'row',
                class: 'Widget',
                layout: {
                  class: 'BoxLayout',
                  items: [
                    { widget: { name: 'button_a', class: 'Button' } },
                    { widget: { name: 'button_b', class: 'Button' } },
                  ],
                },
              },
            },
          ],
        },
      },
    } as any;
    const row = ['window', 'layout', 'items', 1, 'widget'];
    const b = [...row, 'layout', 'items', 1, 'widget'];
    const moved = moveSiblings(doc, [row, b], -1);
    expect(moved).toEqual([
      ['window', 'layout', 'items', 0, 'widget'],
      ['window', 'layout', 'items', 0, 'widget', 'layout', 'items', 0, 'widget'],
    ]);
    expect(doc.window.layout.items[0].widget.layout.items[0].widget.name).toBe('button_b');
  });
});
