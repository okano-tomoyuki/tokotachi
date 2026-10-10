import { describe, expect, it } from 'vitest';
import { SAMPLE } from '../testing.ts';
import { findOutlineNode, flattenOutline, outline } from './outline.ts';

describe('outline', () => {
  const roots = outline(SAMPLE);

  it('ウィンドウ・Action・オブジェクトを根に並べる', () => {
    expect(roots.map((r) => `${r.kind}:${r.label}`)).toEqual([
      'window:MainWindow',
      'group:actions',
      'group:objects',
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
    expect(variables).toContain('search_row_layout');
    expect(variables).toContain('size_spin');
    expect(findOutlineNode(roots, 'window/layout/items/0/widget')?.label).toBe('search_row');
  });

  it('タブ・区画・空き・メニューの項目', () => {
    const all = flattenOutline(roots);
    expect(all.find((n) => n.kind === 'tab' && n.label === 'detail_tab')?.children[0]?.label).toBe(
      'detail_page',
    );
    expect(all.filter((n) => n.kind === 'pane')).toHaveLength(2);
    expect(all.find((n) => n.kind === 'spacer')?.label).toBe('stretch 1');
    expect(all.filter((n) => n.kind === 'menu_item').map((n) => n.label)).toEqual([
      'open_action',
      'clear_recent_action',
      'quit_action',
      'details_action',
    ]);
  });
});
