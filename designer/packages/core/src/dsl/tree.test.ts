import { describe, expect, it } from 'vitest';
import { SAMPLE } from '../testing.ts';
import { namedNodes, temporaryNames, widgets } from './tree.ts';

describe('tree', () => {
  it('部品を木の順(ツールバー・ステータスバー・中身)に並べる', () => {
    expect(widgets(SAMPLE).map((w) => w.node.name)).toEqual([
      'combo_box_view',
      'label_count',
      'row_search',
      'line_edit_search',
      'button_search',
      'splitter_main',
      'tree_view_folders',
      'tab_widget_files',
      'list_view_files',
      'page_detail',
      'label_name',
      'line_edit_name',
      'label_size',
      'spin_box_size',
      'row_buttons',
      'check_box_hidden',
      'button_close',
    ]);
  });

  it('名前を持つノード', () => {
    const names = namedNodes(SAMPLE).map((n) => `${n.kind}:${n.name}`);
    expect(names.slice(0, 6)).toEqual([
      'action:action_open',
      'action:action_clear_recent',
      'action:action_details',
      'action:action_quit',
      'object:timer_refresh',
      'menu:menu_file',
    ]);
    expect(names).toContain('tab:tab_detail');
  });

  it('一時的な変数の名前', () => {
    expect(temporaryNames(SAMPLE).map((t) => t.name)).toEqual([
      'window_menu_bar',
      'window_tool_bar',
      'window_status_bar',
      'menu_file_menu_2',
      'window_menu_bar_menu_2',
      'window_layout',
      'row_search_layout',
      'page_detail_layout',
      'row_buttons_layout',
      'splitter_main_pane_1',
      'splitter_main_pane_2',
      'tab_widget_files_tab_1',
    ]);
  });
});
