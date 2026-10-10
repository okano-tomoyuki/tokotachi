import { describe, expect, it } from 'vitest';
import { validateDocument } from '../dsl/validate.ts';
import { SAMPLE } from '../testing.ts';
import { applyCommand, type EditCommand } from './commands.ts';
import { newName } from './structure.ts';

const FILE_MENU = ['window', 'menu_bar', 'menus', 0];

function apply(command: EditCommand) {
  const r = applyCommand(SAMPLE, command);
  if (!r.ok) {
    throw new Error(r.error);
  }
  expect(validateDocument(r.document).filter((d) => d.severity === 'error')).toEqual([]);
  return r.document as any;
}

describe('メニュー・ツールバー・Action', () => {
  it('新しい Action を作って、メニューの項目から参照する', () => {
    const name = newName(SAMPLE, 'action');
    const doc = apply({
      kind: 'batch',
      commands: [
        { kind: 'addAction', name },
        { kind: 'addEntry', container: FILE_MENU, index: 1, entry: { action: name } },
      ],
    });
    expect(doc.actions.at(-1)).toEqual({ name: 'action_1', properties: { text: 'action_1' } });
    expect(doc.window.menu_bar.menus[0].items[1]).toEqual({ action: 'action_1' });
  });

  it('区切り・サブメニュー・メニューを足す', () => {
    const doc = apply({
      kind: 'batch',
      commands: [
        {
          kind: 'addEntry',
          container: ['window', 'tool_bar'],
          index: 0,
          entry: { separator: true },
        },
        { kind: 'addMenu', parent: FILE_MENU, index: 0, title: '新規(&N)' },
        { kind: 'addMenu', parent: ['window', 'menu_bar'], index: 2, title: 'ヘルプ(&H)' },
      ],
    });
    expect(doc.window.tool_bar.items[0]).toEqual({ separator: true });
    expect(doc.window.menu_bar.menus[0].items[0]).toEqual({ menu: { title: '新規(&N)' } });
    expect(doc.window.menu_bar.menus[2]).toEqual({ title: 'ヘルプ(&H)' });
  });

  it('見出し・参照する Action を変え、前後に動かす', () => {
    const doc = apply({
      kind: 'batch',
      commands: [
        { kind: 'setMenuTitle', target: FILE_MENU, title: 'File' },
        { kind: 'setEntryAction', target: [...FILE_MENU, 'items', 0], action: 'action_details' },
        { kind: 'moveSibling', target: [...FILE_MENU, 'items', 3], delta: -1 },
        { kind: 'moveSibling', target: ['actions', 0], delta: 1 },
      ],
    });
    const items = doc.window.menu_bar.menus[0].items;
    expect(doc.window.menu_bar.menus[0].title).toBe('File');
    expect(items[0]).toEqual({ action: 'action_details' });
    expect(items.map((i: any) => Object.keys(i)[0])).toEqual([
      'action',
      'menu',
      'action',
      'separator',
    ]);
    expect(doc.actions.map((a: any) => a.name).slice(0, 2)).toEqual([
      'action_clear_recent',
      'action_open',
    ]);
  });

  it('部品の中身を指しても、それを入れている項目を動かす', () => {
    const doc = apply({
      kind: 'moveSibling',
      target: ['window', 'layout', 'items', 0, 'widget'],
      delta: 1,
    });
    expect(doc.window.layout.items.map((i: any) => i.widget.name)).toEqual([
      'splitter_main',
      'row_search',
      'row_buttons',
    ]);
  });

  it('Action を消すと、参照する項目・部品の actions も消える', () => {
    const doc = apply({ kind: 'remove', targets: [['actions', 3]] });
    expect(doc.actions.map((a: any) => a.name)).not.toContain('action_quit');
    expect(doc.window.menu_bar.menus[0].items.map((i: any) => i.action ?? '-')).toEqual([
      'action_open',
      '-',
      '-',
    ]);
    expect(doc.window.layout.items[2].widget.layout.items[2].widget.actions).toBeUndefined();
  });

  it('サブメニュー・メニュー・バーを消し、バーを付ける', () => {
    const doc = apply({
      kind: 'batch',
      commands: [
        {
          kind: 'remove',
          targets: [
            [...FILE_MENU, 'items', 1, 'menu'],
            ['window', 'menu_bar', 'menus', 1],
          ],
        },
        { kind: 'remove', targets: [['window', 'status_bar']] },
      ],
    });
    expect(doc.window.menu_bar.menus).toHaveLength(1);
    expect(doc.window.menu_bar.menus[0].items).toHaveLength(3);
    expect(doc.window.status_bar).toBeUndefined();
    const added = applyCommand(doc, { kind: 'addBar', bar: 'status_bar' });
    expect(added.ok && (added.document as any).window.status_bar).toEqual({});
  });
});
