import { describe, expect, it } from 'vitest';
import { validateDocument } from '../dsl/validate.ts';
import { SAMPLE } from '../testing.ts';
import { applyCommand, type EditCommand } from './commands.ts';

const SEARCH_BUTTON = ['window', 'layout', 'items', 0, 'widget', 'layout', 'items', 1, 'widget'];

function apply(command: EditCommand) {
  const r = applyCommand(SAMPLE, command);
  if (!r.ok) {
    throw new Error(r.error);
  }
  expect(validateDocument(r.document)).toEqual([]);
  return r.document as any;
}

function error(command: EditCommand): string {
  const r = applyCommand(SAMPLE, command);
  expect(r.ok).toBe(false);
  return r.ok ? '' : r.error;
}

const LIST_VIEW = [
  'window',
  'layout',
  'items',
  1,
  'widget',
  'panes',
  1,
  'widget',
  'tabs',
  0,
  'page',
];

describe('列の編集', () => {
  it('一覧の部品に列を足し、消し、並べ替える', () => {
    const doc = apply({ kind: 'addColumn', target: LIST_VIEW, title: '種類', index: 1 });
    const view = doc.window.layout.items[1].widget.panes[1].widget.tabs[0].page;
    expect(view.columns.map((c: any) => c.properties.title)).toEqual([
      '名前',
      '種類',
      '大きさ',
      '更新日時',
    ]);
    const removed = apply({
      kind: 'remove',
      targets: [0, 1, 2].map((i) => [...LIST_VIEW, 'columns', i]),
    });
    const v2 = removed.window.layout.items[1].widget.panes[1].widget.tabs[0].page;
    expect(v2.columns).toBeUndefined();
    const moved = apply({ kind: 'moveSibling', target: [...LIST_VIEW, 'columns', 0], delta: 1 });
    expect(
      moved.window.layout.items[1].widget.panes[1].widget.tabs[0].page.columns[0].properties.title,
    ).toBe('大きさ');
  });

  it('一覧でない部品には足せない', () => {
    expect(error({ kind: 'addColumn', target: SEARCH_BUTTON, title: 'x' })).toContain('columns');
  });

  it('列のプロパティを設定する', () => {
    const doc = apply({
      kind: 'setProperties',
      target: [...LIST_VIEW, 'columns', 2],
      properties: { width: 160 },
    });
    expect(
      doc.window.layout.items[1].widget.panes[1].widget.tabs[0].page.columns[2].properties,
    ).toEqual({ title: '更新日時', width: 160 });
  });
});
