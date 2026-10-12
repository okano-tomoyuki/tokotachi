import { describe, expect, it } from 'vitest';
import { SAMPLE } from '../testing.ts';
import { findOutlineNode, flattenOutline, hiddenOutlineNodes, outline } from './outline.ts';

describe('outline', () => {
  const roots = outline(SAMPLE);

  it('ウィンドウ・Action・オブジェクト・Variable を根に並べる', () => {
    expect(roots.map((r) => `${r.kind}:${r.label}`)).toEqual([
      'window:MainWindow',
      'group:actions',
      'group:objects',
      'group:variables',
    ]);
    expect(roots[3]?.children.map((c) => `${c.kind}:${c.label}`)).toEqual([
      'variable:file_name',
      'variable:show_hidden',
    ]);
    expect(roots[0]?.children.map((c) => c.kind)).toEqual([
      'menu_bar',
      'tool_bar',
      'status_bar',
      'layout',
    ]);
  });

  it('部品・レイアウト・バーには、キャンバスの描画の結果の変数の名前がある', () => {
    const all = flattenOutline(roots);
    const variables = all.flatMap((n) => (n.variable === undefined ? [] : [n.variable]));
    expect(variables).toContain('window_menu_bar');
    expect(variables).toContain('window_layout');
    expect(variables).toContain('row_search_layout');
    expect(variables).toContain('spin_box_size');
    expect(findOutlineNode(roots, 'window/layout/items/0/widget')?.label).toBe('row_search');
  });

  it('タブ・区画・空き・メニューの項目', () => {
    const all = flattenOutline(roots);
    expect(all.find((n) => n.kind === 'tab' && n.label === 'tab_detail')?.children[0]?.label).toBe(
      'page_detail',
    );
    expect(all.filter((n) => n.kind === 'pane')).toHaveLength(2);
    expect(all.find((n) => n.kind === 'spacer')?.label).toBe('stretch 1');
    expect(all.filter((n) => n.kind === 'menu_item').map((n) => n.label)).toEqual([
      'action_open',
      'action_clear_recent',
      'action_quit',
      'action_details',
    ]);
  });

  it('visible を false と書いた部品と、その子孫を非表示とする(ウィンドウは除く)', () => {
    const doc = JSON.parse(JSON.stringify(SAMPLE)) as {
      window: {
        properties?: Record<string, unknown>;
        layout: { items: { widget: { properties?: Record<string, unknown> } }[] };
      };
    };
    doc.window.properties = { ...doc.window.properties, visible: false };
    const row = doc.window.layout.items[0]?.widget;
    if (!row) {
      throw new Error('row_search');
    }
    row.properties = { ...row.properties, visible: false };
    const tree = outline(doc as never);
    const hidden = hiddenOutlineNodes(doc as never, tree);
    expect(hidden.get('window/layout/items/0/widget')).toBe('self');
    const descendants = flattenOutline(
      findOutlineNode(tree, 'window/layout/items/0/widget')?.children ?? [],
    );
    expect(descendants.length).toBeGreaterThan(0);
    expect(descendants.every((n) => hidden.get(n.id) === 'ancestor')).toBe(true);
    expect(hidden.has('window')).toBe(false);
    expect(hiddenOutlineNodes(SAMPLE, outline(SAMPLE)).size).toBe(0);
  });
});
