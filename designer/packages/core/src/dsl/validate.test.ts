import { describe, expect, it } from 'vitest';
import { sampleCopy } from '../testing.ts';
import { parseDocument } from './parse.ts';

function diagnostics(doc: unknown) {
  return parseDocument(JSON.stringify(doc)).diagnostics;
}

function codes(doc: unknown): string[] {
  return diagnostics(doc).map((d) => d.code);
}

/** 見本の中身のレイアウトの項目。 */
function items(doc: any): any[] {
  return doc.window.layout.items;
}

describe('validateDocument', () => {
  it('見本には問題が無い', () => {
    expect(diagnostics(sampleCopy())).toEqual([]);
  });

  it('JSON・版・構造の誤り', () => {
    expect(parseDocument('{').diagnostics[0]?.code).toBe('json-syntax');
    const doc = sampleCopy();
    doc.format_version = 2;
    expect(codes(doc)).toEqual(['unsupported-version']);
    const extra = sampleCopy();
    extra.window.unknown_key = 1;
    expect(codes(extra)).toEqual(['schema']);
  });

  it('名前の重複・識別子・予約の名前', () => {
    const doc = sampleCopy();
    doc.actions[1].name = 'action_open';
    // Window のメンバ
    doc.objects[0].name = 'close';
    items(doc)[0].widget.name = 'tokotachi_row';
    // action_clear_recent が無くなるので、メニューの参照も見つからない
    expect(codes(doc)).toEqual([
      'duplicate-name',
      'invalid-identifier',
      'invalid-identifier',
      'unknown-reference',
    ]);
  });

  it('一時的な変数の名前と重なる名前は使えない', () => {
    const doc = sampleCopy();
    // row_search のレイアウトの一時的な変数
    doc.objects[0].name = 'row_search_layout';
    const d = diagnostics(doc);
    expect(d.map((x) => x.code)).toEqual(['reserved-name']);
    expect(d[0]?.path).toEqual(['objects', 0, 'name']);
  });

  it('名前の書き方は警告', () => {
    const doc = sampleCopy();
    doc.window.name = 'main_window';
    doc.actions[2].name = 'ActionDetails';
    doc.window.menu_bar.menus[1].items[0].action = 'ActionDetails';
    const d = diagnostics(doc);
    expect(d.map((x) => [x.severity, x.code])).toEqual([
      ['warning', 'naming-style'],
      ['warning', 'naming-style'],
    ]);
  });

  it('クラス・プロパティ・値の型', () => {
    const doc = sampleCopy();
    const button = items(doc)[0].widget.layout.items[1].widget;
    button.properties.text = 1;
    button.properties.style = 'LARGE';
    button.properties.no_such = true;
    button.properties.bounds = { x: 0 };
    doc.objects[0].class = 'Button';
    expect(codes(doc)).toEqual([
      'wrong-class-kind',
      'invalid-property-value',
      'invalid-property-value',
      'unknown-property',
      'unknown-property',
    ]);
  });

  it('位置と大きさは、ウィンドウと自由な配置の子だけ', () => {
    const doc = sampleCopy();
    items(doc)[0].widget.properties = { width: 100 };
    doc.window.properties.x = 10;
    expect(codes(doc)).toEqual(['geometry-not-allowed', 'geometry-not-allowed']);
    const free = sampleCopy();
    delete free.window.layout;
    free.window.children = [
      { name: 'free_button', class: 'Button', properties: { x: 10, y: 20, width: 80 } },
    ];
    expect(codes(free)).toEqual([]);
  });

  it('値の型(Margins・ThemeColor・Shortcut)', () => {
    const doc = sampleCopy();
    doc.window.layout.properties.margins = { left: 1, middle: 2 };
    doc.actions[0].properties.shortcut = 'Ctrl+Hyper+S';
    doc.window.status_bar.widgets[0].properties.color = '#12345';
    expect(codes(doc)).toEqual([
      'invalid-property-value',
      'invalid-property-value',
      'invalid-property-value',
    ]);
    const ok = sampleCopy();
    ok.window.status_bar.widgets[0].properties.color = '#FF800080';
    ok.actions[0].properties.shortcut = 'ctrl+shift+F5';
    expect(codes(ok)).toEqual([]);
  });

  it('ハンドラ・参照', () => {
    const doc = sampleCopy();
    doc.actions[0].events.on_click = 'x';
    doc.window.tool_bar.items[0].action = 'missing_action';
    // search は NotifyHandler(検索欄と検索のボタン)。先に CloseHandler に使うと、2 つとも型が合わない
    doc.window.events.on_close = 'search';
    expect(codes(doc)).toEqual([
      'unknown-reference',
      'unknown-event',
      'handler-signature-conflict',
      'handler-signature-conflict',
    ]);
    const member = sampleCopy();
    member.objects[0].events.on_timer = 'line_edit_search';
    expect(codes(member)).toEqual(['duplicate-name']);
  });

  it('子を持てない部品・専用の子のキー・Dialog のボタン', () => {
    const doc = sampleCopy();
    const button = items(doc)[0].widget.layout.items[1].widget;
    button.layout = { class: 'BoxLayout' };
    button.tabs = [];
    doc.window.buttons = [{ button: 'OK' }];
    expect(codes(doc)).toEqual([
      'buttons-not-allowed',
      'children-not-allowed',
      'child-key-not-allowed',
    ]);
    const dialog = sampleCopy();
    dialog.window.class = 'Dialog';
    dialog.window.buttons = [{ button: 'OK' }, { button: 'NONE' }];
    expect(codes(dialog)).toEqual(['invalid-dialog-button']);
  });

  it('レイアウトの項目', () => {
    const doc = sampleCopy();
    const box = doc.window.layout;
    box.items.push({ stretch: 1, spacing: 4 });
    box.items.push({ row: 0, widget: { name: 'w1', class: 'Button' } });
    box.rows = { '0': { stretch: 1 } };
    const grid = items(doc)[1].widget.panes[1].widget.tabs[1].page.layout;
    // (1, 1) と重なる
    grid.items[1].row = 1;
    grid.items.push({ row: 0, widget: { name: 'w2', class: 'Button' } });
    expect(codes(doc)).toEqual([
      'tracks-not-allowed',
      'invalid-layout-item',
      'invalid-layout-item',
      'grid-cell-overlap',
      'invalid-layout-item',
    ]);
  });

  it('メニュー・ツールバーの項目は 1 つの種類だけ', () => {
    const doc = sampleCopy();
    doc.window.menu_bar.menus[0].items[0].separator = true;
    doc.window.tool_bar.items[1] = {};
    expect(codes(doc)).toEqual(['invalid-tool-item', 'invalid-menu-item']);
  });
});
