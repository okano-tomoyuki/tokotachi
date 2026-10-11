import { describe, expect, it } from 'vitest';
import { validateDocument } from '../dsl/validate.ts';
import { SAMPLE } from '../testing.ts';
import { applyCommand, defaultHandlerName, type EditCommand } from './commands.ts';

const SEARCH_BUTTON = ['window', 'layout', 'items', 0, 'widget', 'layout', 'items', 1, 'widget'];

function apply(command: EditCommand) {
  const r = applyCommand(SAMPLE, command);
  if (!r.ok) {
    throw new Error(r.error);
  }
  // どのコマンドの後も、検証のエラーを増やさない
  expect(validateDocument(r.document)).toEqual([]);
  return r.document as any;
}

function error(command: EditCommand): string {
  const r = applyCommand(SAMPLE, command);
  expect(r.ok).toBe(false);
  return r.ok ? '' : r.error;
}

describe('applyCommand', () => {
  it('プロパティを設定・消す。元のドキュメントは変えない', () => {
    const doc = apply({
      kind: 'setProperties',
      target: SEARCH_BUTTON,
      properties: { text: '探す', style: 'SECONDARY' },
    });
    const original = (SAMPLE as any).window.layout.items[0].widget.layout.items[1].widget
      .properties;
    expect(doc.window.layout.items[0].widget.layout.items[1].widget.properties).toEqual({
      ...original,
      text: '探す',
      style: 'SECONDARY',
    });
    expect(original.text).toBe('検索');
    const cleared = apply({
      kind: 'setProperties',
      target: SEARCH_BUTTON,
      properties: { text: null, theme_override: null },
    });
    expect(cleared.window.layout.items[0].widget.layout.items[1].widget.properties).toBeUndefined();
  });

  it('ウィンドウ・Action・タブのプロパティ', () => {
    expect(
      apply({ kind: 'setProperties', target: ['window'], properties: { title: 'x' } }).window
        .properties.title,
    ).toBe('x');
    expect(
      apply({ kind: 'setProperties', target: ['actions', 0], properties: { enabled: false } })
        .actions[0].properties.enabled,
    ).toBe(false);
  });

  it('ハンドラを設定・消す。型の違う同じ名前は拒む', () => {
    const doc = apply({
      kind: 'setEvent',
      target: SEARCH_BUTTON,
      event: 'on_click',
      handler: 'button_search_click',
    });
    expect(doc.window.layout.items[0].widget.layout.items[1].widget.events).toEqual({
      on_click: 'button_search_click',
    });
    const removed = apply({
      kind: 'setEvent',
      target: SEARCH_BUTTON,
      event: 'on_click',
      handler: null,
    });
    expect(removed.window.layout.items[0].widget.layout.items[1].widget.events).toBeUndefined();
    // main_window_close は CloseHandler
    expect(
      error({
        kind: 'setEvent',
        target: SEARCH_BUTTON,
        event: 'on_click',
        handler: 'main_window_close',
      }),
    ).toContain('CloseHandler');
    expect(
      error({
        kind: 'setEvent',
        target: SEARCH_BUTTON,
        event: 'on_click',
        handler: 'line_edit_search',
      }),
    ).toContain('line_edit_search');
    expect(
      error({ kind: 'setEvent', target: SEARCH_BUTTON, event: 'on_nothing', handler: 'x' }),
    ).toContain('on_nothing');
  });

  it('名前を変え、参照も書き換える', () => {
    const doc = apply({ kind: 'rename', target: ['actions', 0], name: 'open_file_action' });
    expect(doc.actions[0].name).toBe('open_file_action');
    expect(doc.window.menu_bar.menus[0].items[0].action).toBe('open_file_action');
    expect(doc.window.tool_bar.items[0].action).toBe('open_file_action');
    const quit = apply({ kind: 'rename', target: ['actions', 3], name: 'exit_action' });
    expect(quit.window.layout.items[2].widget.layout.items[2].widget.actions).toEqual([
      'exit_action',
    ]);
    expect(error({ kind: 'rename', target: SEARCH_BUTTON, name: 'line_edit_search' })).toContain(
      'line_edit_search',
    );
    expect(error({ kind: 'rename', target: SEARCH_BUTTON, name: 'close' })).toContain('close');
    expect(apply({ kind: 'rename', target: ['window'], name: 'FileWindow' }).window.name).toBe(
      'FileWindow',
    );
  });

  it('batch はどれかが失敗したら何も変えない', () => {
    const r = applyCommand(SAMPLE, {
      kind: 'batch',
      commands: [
        { kind: 'setProperties', target: SEARCH_BUTTON, properties: { text: 'x' } },
        { kind: 'rename', target: SEARCH_BUTTON, name: '1bad' },
      ],
    });
    expect(r.ok).toBe(false);
  });

  it('パスが指すものが無ければ失敗する', () => {
    expect(
      error({ kind: 'setProperties', target: ['window', 'nothing'], properties: {} }),
    ).toContain('window.nothing');
  });

  it('ハンドラの既定の名前', () => {
    expect(defaultHandlerName('button_search', 'on_click')).toBe('button_search_click');
    expect(defaultHandlerName(undefined, 'on_close')).toBe('window_close');
  });
});
