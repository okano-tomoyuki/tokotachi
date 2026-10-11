import { describe, expect, it } from 'vitest';
import { hasErrors } from '../dsl/diagnostics.ts';
import type { TtuiDocument } from '../dsl/schema.ts';
import { namedNodes } from '../dsl/tree.ts';
import { validateDocument } from '../dsl/validate.ts';
import { SAMPLE } from '../testing.ts';
import { applyCommand } from './commands.ts';
import { outermostPaths, slotAfter } from './slots.ts';
import { valueAt } from './structure.ts';

function pathOf(doc: TtuiDocument, name: string) {
  const n = namedNodes(doc).find((x) => x.name === name);
  if (!n) {
    throw new Error(name);
  }
  return n.path.slice(0, -1);
}

function moved(doc: TtuiDocument, command: Parameters<typeof applyCommand>[1]): TtuiDocument {
  const r = applyCommand(doc, command);
  if (!r.ok) {
    throw new Error(r.error);
  }
  expect(hasErrors(validateDocument(r.document))).toBe(false);
  return r.document;
}

describe('slots', () => {
  it('選んだもののうち、親が選ばれているものを除き、構造の木の順に並べる', () => {
    const row = pathOf(SAMPLE, 'row_search');
    const button = pathOf(SAMPLE, 'button_search');
    const label = pathOf(SAMPLE, 'label_count');
    // ボタンは row_search の中なので除く。構造の木ではステータスバーがレイアウトより前
    expect(outermostPaths(SAMPLE, [button, row, label])).toEqual([label, row]);
    expect(outermostPaths(SAMPLE, [button, row])).toEqual([row]);
  });

  it('部品のすぐ後ろ(BoxLayout は次の項目)', () => {
    expect(slotAfter(SAMPLE, pathOf(SAMPLE, 'row_search'))).toEqual({
      kind: 'item',
      layout: ['window', 'layout'],
      index: 1,
    });
  });

  it('まとめて移すと、構造の木の順に並べて入れる', () => {
    const doc = moved(SAMPLE, {
      kind: 'moveWidgets',
      // 選んだ順によらず、構造の木の順になる
      targets: [pathOf(SAMPLE, 'button_search'), pathOf(SAMPLE, 'line_edit_search')],
      slot: { kind: 'item', layout: ['window', 'layout'], index: 0 },
    });
    const items = valueAt(doc, ['window', 'layout', 'items']) as { widget?: { name: string } }[];
    expect(items.slice(0, 3).map((i) => i.widget?.name)).toEqual([
      'line_edit_search',
      'button_search',
      'row_search',
    ]);
    // 一時的な印は残さない
    expect(JSON.stringify(doc)).not.toContain('__tokotachi_moving');
  });

  it('自由な配置に移すときは、互いの位置関係を保つ', () => {
    const doc = {
      format_version: 1,
      window: {
        name: 'MainWindow',
        class: 'Window',
        children: [
          {
            name: 'panel',
            class: 'Widget',
            properties: { x: 0, y: 0, width: 200, height: 200 },
            children: [
              { name: 'label_a', class: 'Label', properties: { x: 10, y: 10 } },
              { name: 'label_b', class: 'Label', properties: { x: 40, y: 20 } },
            ],
          },
        ],
      },
    } as unknown as TtuiDocument;
    const out = moved(doc, {
      kind: 'moveWidgets',
      targets: [pathOf(doc, 'label_a'), pathOf(doc, 'label_b')],
      slot: { kind: 'child', parent: ['window'], x: 100, y: 100 },
    });
    const children = valueAt(out, ['window', 'children']) as {
      name: string;
      properties?: Record<string, unknown>;
    }[];
    expect(children.map((c) => [c.name, c.properties?.['x'], c.properties?.['y']])).toEqual([
      ['panel', 0, 0],
      ['label_a', 100, 100],
      ['label_b', 130, 110],
    ]);
  });
});
