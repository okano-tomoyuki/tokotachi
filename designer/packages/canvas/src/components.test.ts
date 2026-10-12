import { TtuiDocument } from '@tokotachi-designer/core';
import { describe, expect, it } from 'vitest';
import { canvasSetup } from './components.ts';

const COMPONENT = TtuiDocument.parse({
  format_version: 1,
  actions: [{ name: 'action_find', properties: { icon: 'icons/find.png' } }],
  window: {
    name: 'SearchBar',
    class: 'Widget',
    properties: { width: 300, height: 40 },
    layout: {
      class: 'BoxLayout',
      items: [
        {
          widget: {
            name: 'button_search',
            class: 'Button',
            events: { on_click: 'search' },
          },
        },
      ],
    },
  },
});

const HOST = TtuiDocument.parse({
  format_version: 1,
  window: {
    name: 'MainWindow',
    class: 'Window',
    children: [
      {
        name: 'panel_search',
        class: 'Widget',
        component: 'parts/search_bar.ttui.json',
        properties: { x: 8 },
      },
    ],
  },
});

describe('canvasSetup', () => {
  it('置いた部品ファイルの操作を、名前を付け替えて展開する(部品ファイルの設定が先)', () => {
    const { windowClass, ops } = canvasSetup(
      HOST,
      (path) => (path === 'parts/search_bar.ttui.json' ? { document: COMPONENT } : undefined),
      undefined,
    );
    expect(windowClass).toBe('Window');
    expect(ops.filter((op) => op.op !== 'blank')).toEqual([
      { op: 'new', variable: 'panel_search', class: 'Widget', declare: false, parent: 'this' },
      {
        op: 'new',
        variable: 'panel_search.action_find',
        class: 'Action',
        declare: false,
        parent: 'panel_search',
      },
      // 画像のパスは置いた側のフォルダから
      {
        op: 'set',
        target: 'panel_search.action_find',
        property: 'icon',
        type: { value: 'Image' },
        value: 'parts/icons/find.png',
      },
      {
        op: 'new',
        variable: 'panel_search.window_layout',
        class: 'BoxLayout',
        declare: true,
        parent: 'panel_search',
      },
      {
        op: 'new',
        variable: 'panel_search.button_search',
        class: 'Button',
        declare: false,
        parent: 'panel_search',
      },
      // ハンドラはつながない
      {
        op: 'call',
        target: 'panel_search.window_layout',
        method: 'add',
        args: [{ kind: 'ref', name: 'panel_search.button_search' }],
        result: undefined,
      },
      { op: 'set', target: 'panel_search', property: 'x', type: 'float', value: 8 },
    ]);
  });

  it('部品ファイルを開いたときは、ウィンドウいっぱいに根の Widget を置く', () => {
    const { windowClass, ops } = canvasSetup(COMPONENT, undefined, undefined);
    expect(windowClass).toBe('Window');
    expect(ops.slice(0, 5)).toEqual([
      { op: 'set', property: 'width', type: 'float', value: 300 },
      { op: 'set', property: 'height', type: 'float', value: 40 },
      {
        op: 'new',
        variable: 'tokotachi_component_layout',
        class: 'BoxLayout',
        declare: true,
        parent: 'this',
      },
      { op: 'new', variable: 'SearchBar', class: 'Widget', declare: false, parent: 'this' },
      {
        op: 'call',
        target: 'tokotachi_component_layout',
        method: 'add',
        args: [
          { kind: 'ref', name: 'SearchBar' },
          { kind: 'int', value: 1 },
        ],
      },
    ]);
    expect(ops[5]).toMatchObject({ op: 'new', variable: 'action_find', parent: 'SearchBar' });
  });
});
