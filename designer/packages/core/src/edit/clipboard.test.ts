import { describe, expect, it } from 'vitest';
import { hasErrors } from '../dsl/diagnostics.ts';
import type { TtuiDocument } from '../dsl/schema.ts';
import { namedNodes } from '../dsl/tree.ts';
import { validateDocument } from '../dsl/validate.ts';
import { SAMPLE } from '../testing.ts';
import { copyNodes, parseClipboard, preparePaste, serializeClipboard } from './clipboard.ts';
import { applyCommand } from './commands.ts';
import { valueAt } from './structure.ts';

function pathOf(doc: TtuiDocument, name: string) {
  const n = namedNodes(doc).find((x) => x.name === name);
  if (!n) {
    throw new Error(name);
  }
  // 名前の欄の位置から、ノードの位置にする
  return n.path.slice(0, -1);
}

/** コピーする(できなければテストを失敗させる)。 */
function copied(doc: TtuiDocument, paths: Parameters<typeof copyNodes>[1]) {
  const data = copyNodes(doc, paths);
  if (!data) {
    throw new Error('nothing copied');
  }
  return data;
}

function names(doc: TtuiDocument): string[] {
  return namedNodes(doc).map((n) => n.name);
}

/** ウィンドウと空のレイアウトだけの定義ファイル(ほかのファイルへの貼り付け)。 */
const EMPTY = {
  format_version: 1,
  window: { name: 'OtherWindow', class: 'Window', layout: { class: 'BoxLayout' } },
} as unknown as TtuiDocument;

describe('clipboard', () => {
  it('部品を子孫ごとコピーし、文字にして読み戻せる', () => {
    const data = copyNodes(SAMPLE, [pathOf(SAMPLE, 'row_search')]);
    expect(data?.items.map((i) => [i.kind, i.node.name])).toEqual([['widget', 'row_search']]);
    expect(data && parseClipboard(serializeClipboard(data))).toEqual(data);
    // デザイナーがコピーしたものでなければ読まない
    expect(parseClipboard('{"items": []}')).toBeUndefined();
    expect(parseClipboard('hello')).toBeUndefined();
    // レイアウト・ウィンドウはコピーしない
    expect(copyNodes(SAMPLE, [['window'], ['window', 'layout']])).toBeUndefined();
  });

  it('同じファイルに貼り付けると、重なる名前を子孫まで改名する', () => {
    const data = copied(SAMPLE, [pathOf(SAMPLE, 'row_search')]);
    const prepared = preparePaste(SAMPLE, data.items);
    const pasted = names({
      format_version: 1,
      window: { name: 'W', class: 'Window', children: prepared.map((i) => i.node) },
    } as unknown as TtuiDocument).filter((n) => n !== 'W');
    expect(pasted).toEqual(['row_search_2', 'line_edit_search_2', 'button_search_2']);
    const r = applyCommand(SAMPLE, {
      kind: 'paste',
      items: data.items,
      slot: { kind: 'item', layout: ['window', 'layout'], index: 1 },
    });
    expect(r.ok).toBe(true);
    if (r.ok) {
      expect(valueAt(r.document, ['window', 'layout', 'items', 1, 'widget', 'name'])).toBe(
        'row_search_2',
      );
      expect(hasErrors(validateDocument(r.document))).toBe(false);
    }
  });

  it('Action・Variable は一覧の終わりに足し、名前の数字を進める', () => {
    const data = copied(SAMPLE, [
      ['actions', 0],
      ['variables', 0],
    ]);
    const r = applyCommand(SAMPLE, { kind: 'paste', items: data.items });
    expect(r.ok).toBe(true);
    if (r.ok) {
      expect(r.document.actions?.at(-1)?.name).toBe('action_open_2');
      expect(r.document.variables?.at(-1)?.name).toBe('file_name_2');
    }
  });

  it('ほかのファイルに貼り付けると、そこに無い Variable・Action の参照を外す', () => {
    const data = copied(SAMPLE, [
      pathOf(SAMPLE, 'check_box_hidden'),
      pathOf(SAMPLE, 'button_close'),
    ]);
    const r = applyCommand(EMPTY, {
      kind: 'paste',
      items: data.items,
      slot: { kind: 'item', layout: ['window', 'layout'], index: 0 },
    });
    expect(r.ok).toBe(true);
    if (r.ok) {
      const items = valueAt(r.document, ['window', 'layout', 'items']) as {
        widget: { name: string; properties?: Record<string, unknown>; actions?: unknown };
      }[];
      // コピーした順に並ぶ
      expect(items.map((i) => i.widget.name)).toEqual(['check_box_hidden', 'button_close']);
      expect(items[0]?.widget.properties?.['checked']).toBeUndefined();
      expect(items[0]?.widget.properties?.['text']).toBe('隠しファイルも表示する');
      expect(items[1]?.widget.actions ?? []).toEqual([]);
      expect(hasErrors(validateDocument(r.document))).toBe(false);
    }
  });

  it('部品を貼り付ける場所が無ければ失敗する', () => {
    const data = copied(SAMPLE, [pathOf(SAMPLE, 'row_search')]);
    expect(applyCommand(SAMPLE, { kind: 'paste', items: data.items }).ok).toBe(false);
  });
});
