import { describe, expect, it } from 'vitest';
import { SAMPLE } from '../testing.ts';
import { namedNodes, temporaryNames, widgets } from './tree.ts';

describe('tree', () => {
  it('部品を木の順(ツールバー・ステータスバー・中身)に並べる', () => {
    expect(widgets(SAMPLE).map((w) => w.node.name)).toEqual([
      'view_combo',
      'count_label',
      'search_row',
      'search_edit',
      'search_button',
      'split',
      'folder_tree',
      'tabs',
      'file_list',
      'detail_page',
      'name_label',
      'name_edit',
      'size_label',
      'size_spin',
      'button_row',
      'hidden_check',
      'close_button',
    ]);
  });

  it('名前を持つノード', () => {
    const names = namedNodes(SAMPLE).map((n) => `${n.kind}:${n.name}`);
    expect(names.slice(0, 6)).toEqual([
      'action:open_action',
      'action:clear_recent_action',
      'action:details_action',
      'action:quit_action',
      'object:refresh_timer',
      'menu:file_menu',
    ]);
    expect(names).toContain('tab:detail_tab');
  });

  it('一時的な変数の名前', () => {
    expect(temporaryNames(SAMPLE).map((t) => t.name)).toEqual([
      'window_menu_bar',
      'window_tool_bar',
      'window_status_bar',
      'file_menu_menu_2',
      'window_menu_bar_menu_2',
      'window_layout',
      'search_row_layout',
      'detail_page_layout',
      'button_row_layout',
      'split_pane_1',
      'split_pane_2',
      'tabs_tab_1',
    ]);
  });
});
