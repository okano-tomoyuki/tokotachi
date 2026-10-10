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
});
