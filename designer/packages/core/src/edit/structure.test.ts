import { describe, expect, it } from 'vitest';
import { validateDocument } from '../dsl/validate.ts';
import { SAMPLE } from '../testing.ts';
import { applyCommand, type EditCommand } from './commands.ts';
import { newName, newWidget, snakeCase } from './structure.ts';

const WINDOW_LAYOUT = ['window', 'layout'];
const SEARCH_ROW_LAYOUT = ['window', 'layout', 'items', 0, 'widget', 'layout'];
const SEARCH_EDIT = [...SEARCH_ROW_LAYOUT, 'items', 0, 'widget'];
const SEARCH_BUTTON = [...SEARCH_ROW_LAYOUT, 'items', 1, 'widget'];
const DETAIL_GRID = [
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

function apply(command: EditCommand) {
  const r = applyCommand(SAMPLE, command);
  if (!r.ok) {
    throw new Error(r.error);
  }
  expect(validateDocument(r.document).filter((d) => d.severity === 'error')).toEqual([]);
  return r.document as any;
}

function error(command: EditCommand): string {
  const r = applyCommand(SAMPLE, command);
  expect(r.ok).toBe(false);
  return r.ok ? '' : r.error;
}

const names = (items: any[]): string[] =>
  items.map((i) => i.widget?.name ?? `(${Object.keys(i).join()})`);

describe('名前と既定の内容', () => {
  it('種類を先頭に付けた空いている名前', () => {
    expect(snakeCase('LineEdit')).toBe('line_edit');
    expect(newName(SAMPLE, 'button')).toBe('button_1');
    expect(newName(SAMPLE, 'button', new Set(['button_1']))).toBe('button_2');
  });

  it('文字を持つ部品は名前を文字にし、入れ物には BoxLayout を付ける', () => {
    expect(newWidget(SAMPLE, 'Button')).toEqual({
      name: 'button_1',
      class: 'Button',
      properties: { text: 'button_1' },
    });
    expect(newWidget(SAMPLE, 'Widget')).toEqual({
      name: 'widget_1',
      class: 'Widget',
      layout: { class: 'BoxLayout' },
    });
    expect(newWidget(SAMPLE, 'LineEdit')).toEqual({ name: 'line_edit_1', class: 'LineEdit' });
  });
});

describe('部品の追加・移動・削除', () => {
  it('BoxLayout の位置に足す', () => {
    const doc = apply({
      kind: 'addWidget',
      slot: { kind: 'item', layout: SEARCH_ROW_LAYOUT, index: 1 },
      widget: newWidget(SAMPLE, 'Button'),
    });
    expect(names(doc.window.layout.items[0].widget.layout.items)).toEqual([
      'search_edit',
      'button_1',
      'search_button',
    ]);
  });

  it('GridLayout は空いたセルにだけ足せる', () => {
    const doc = apply({
      kind: 'addWidget',
      slot: { kind: 'item', layout: DETAIL_GRID, index: 4, row: 2, column: 1 },
      widget: newWidget(SAMPLE, 'CheckBox'),
    });
    const items = doc.window.layout.items[1].widget.panes[1].widget.tabs[1].page.layout.items;
    expect(items[4]).toMatchObject({ row: 2, column: 1, widget: { name: 'check_box_1' } });
    error({
      kind: 'addWidget',
      slot: { kind: 'item', layout: DETAIL_GRID, index: 4, row: 0, column: 1 },
      widget: newWidget(SAMPLE, 'CheckBox'),
    });
  });

  it('同じレイアウトの中で後ろへ移す(項目の設定は引き継ぐ)', () => {
    const doc = apply({
      kind: 'moveWidget',
      target: SEARCH_EDIT,
      slot: { kind: 'item', layout: SEARCH_ROW_LAYOUT, index: 2 },
    });
    const items = doc.window.layout.items[0].widget.layout.items;
    expect(names(items)).toEqual(['search_button', 'search_edit']);
    expect(items[1].stretch).toBe(1);
  });

  it('別のレイアウトへ移す', () => {
    const doc = apply({
      kind: 'moveWidget',
      target: SEARCH_BUTTON,
      slot: { kind: 'item', layout: WINDOW_LAYOUT, index: 3 },
    });
    expect(names(doc.window.layout.items)).toEqual([
      'search_row',
      'split',
      'button_row',
      'search_button',
    ]);
    expect(names(doc.window.layout.items[0].widget.layout.items)).toEqual(['search_edit']);
  });

  it('自分の中へは移せない', () => {
    expect(
      error({
        kind: 'moveWidget',
        target: ['window', 'layout', 'items', 0, 'widget'],
        slot: { kind: 'item', layout: SEARCH_ROW_LAYOUT, index: 0 },
      }),
    ).toContain('itself');
  });

  it('自由な配置へ移すと位置を書き、レイアウトへ戻すと消す', () => {
    const doc = apply({
      kind: 'batch',
      commands: [
        { kind: 'setLayout', target: ['window', 'layout', 'items', 2, 'widget'], layout: null },
        {
          kind: 'moveWidget',
          target: SEARCH_BUTTON,
          slot: {
            kind: 'child',
            parent: ['window', 'layout', 'items', 2, 'widget'],
            x: 10.4,
            y: 20,
          },
        },
      ],
    });
    const children = doc.window.layout.items[2].widget.children;
    expect(children.map((c: any) => c.name)).toEqual([
      'hidden_check',
      'close_button',
      'search_button',
    ]);
    expect(children[2].properties).toMatchObject({ x: 10, y: 20, text: '検索' });
  });

  it('タブ・区画として足す', () => {
    const tabs = ['window', 'layout', 'items', 1, 'widget', 'panes', 1, 'widget'];
    const doc = apply({
      kind: 'addWidget',
      slot: { kind: 'tab', parent: tabs, index: 2 },
      widget: newWidget(SAMPLE, 'TextEdit'),
    });
    expect(doc.window.layout.items[1].widget.panes[1].widget.tabs[2]).toEqual({
      properties: { title: 'text_edit_1' },
      page: { name: 'text_edit_1', class: 'TextEdit' },
    });
  });

  it('部品・空き・タブを消す(複数を、後ろから)', () => {
    const doc = apply({
      kind: 'remove',
      targets: [
        SEARCH_EDIT,
        ['window', 'layout', 'items', 2, 'widget', 'layout', 'items', 1],
        SEARCH_BUTTON,
      ],
    });
    expect(doc.window.layout.items[0].widget.layout.items).toBeUndefined();
    expect(names(doc.window.layout.items[2].widget.layout.items)).toEqual([
      'hidden_check',
      'close_button',
    ]);
    expect(error({ kind: 'remove', targets: [['window']] })).toContain('window');
  });

  it('空きを足し、項目の設定を変える', () => {
    const doc = apply({
      kind: 'batch',
      commands: [
        { kind: 'addSpacer', layout: SEARCH_ROW_LAYOUT, index: 1, spacer: { spacing: 8 } },
        {
          kind: 'setItem',
          target: [...SEARCH_ROW_LAYOUT, 'items', 0],
          values: { stretch: null, alignment: 'CENTER' },
        },
      ],
    });
    const items = doc.window.layout.items[0].widget.layout.items;
    expect(items[1]).toEqual({ spacing: 8 });
    expect(items[0].stretch).toBeUndefined();
    expect(items[0].alignment).toBe('CENTER');
  });
});

describe('レイアウトの切り替え', () => {
  const ROW = ['window', 'layout', 'items', 0, 'widget'];

  it('横の BoxLayout から GridLayout へは 1 行に並べ、余白・間隔を引き継ぐ', () => {
    const doc = apply({ kind: 'setLayout', target: ROW, layout: 'GridLayout' });
    const layout = doc.window.layout.items[0].widget.layout;
    expect(layout.class).toBe('GridLayout');
    expect(layout.items.map((i: any) => [i.row, i.column, i.widget.name])).toEqual([
      [0, 0, 'search_edit'],
      [0, 1, 'search_button'],
    ]);
    // orientation は GridLayout に無いので引き継がない
    expect(layout.properties).toBeUndefined();
  });

  it('PlaceLayout・自由な配置へは、描いた位置に置く', () => {
    const positions = { search_edit: { x: 0, y: 0 }, search_button: { x: 476.4, y: 0 } };
    const place = apply({ kind: 'setLayout', target: ROW, layout: 'PlaceLayout', positions });
    expect(place.window.layout.items[0].widget.layout.items[1].placement).toEqual({ x: 476, y: 0 });
    const free = apply({ kind: 'setLayout', target: ROW, layout: null, positions });
    expect(free.window.layout.items[0].widget.layout).toBeUndefined();
    expect(free.window.layout.items[0].widget.children[1].properties).toMatchObject({
      x: 476,
      y: 0,
    });
  });
});
