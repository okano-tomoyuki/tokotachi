/// @file
/// tokotachi::TableView の宣言。
#ifndef TOKOTACHI_VIEWS_TABLE_VIEW_HPP
#define TOKOTACHI_VIEWS_TABLE_VIEW_HPP

#include <string>
#include <vector>

#include "item_view.hpp"

namespace tokotachi
{

/// 表: 行と列のセルを表示し、セルを選んで編集するビュー(docs/adr/0050)。モデルの一番上の行を、列(Column)ごとに表示する。
///
/// @image html widget_table_view.png
///
/// @code
/// tokotachi::ListModel* scores = new tokotachi::ListModel(&window);
/// tokotachi::Item* row = scores->add_item("佐藤");
/// row->column_texts = std::vector<std::string>{"82", "75"};  // 2 列目以降
/// row->editable = true;
/// tokotachi::TableView* table_view_scores = new tokotachi::TableView(&window);
/// table_view_scores->add_column("名前", 120);
/// table_view_scores->add_column("国語", 80)->alignment = tokotachi::Align::END;
/// table_view_scores->add_column("数学", 80)->alignment = tokotachi::Align::END;
/// table_view_scores->model = scores;
/// table_view_scores->selection_mode = tokotachi::SelectionMode::MULTI;  // セルの矩形を複数選べる
/// @endcode
///
/// 操作(表計算の流儀):
/// - クリックでセルを選び、Shift+クリック・Shift+矢印・ドラッグで矩形に広げ、Ctrl+クリックで範囲を加える。行の見出しを押すと行を選ぶ。
/// - 矢印で動き、Ctrl+矢印で端へ。Home・End は行の始め・終わり(Ctrl なら表の始め・終わり)。Tab・Shift+Tab で右・左(行をまたぐ)。
/// - 編集できるセル(モデルの editable)では、文字を打つと上書きで、F2・ダブルクリックで今の文字の続きから編集を始める。
///   Enter・Tab で確定して下・右へ動き、Esc でやめる。Enter は(編集していなくても)下へ、Shift+Enter は上へ動く。
/// - Delete で選んでいるセルを空にする。Ctrl+C・Ctrl+X で選んでいるセルを(行は改行、列はタブで区切って)写し、
///   Ctrl+V で貼る(1 つの値なら選んでいるすべてのセルへ、表なら current のセルから)。
/// - 編集できないセルのダブルクリックで on_activate を呼ぶ。
///
/// selection(ItemView)は、選んでいるセルのある行の項目。行の数・並びが変わると、セルの範囲は current のセルだけになる。
///
/// @ingroup views
class TableView : public ItemView
{
public:
    explicit TableView(Widget* parent = nullptr);

    /// セルを選ぶか行を選ぶか(既定 CELLS)。
    Property<SelectionBehavior> selection_behavior;
    /// キーで動かすセルの列(行は current)。
    Property<int> current_column;
    /// 選んでいるセルの範囲(CELLS のとき。選んだ順)。
    ReadOnlyProperty<std::vector<CellRange> > selected_ranges;
    /// セルの間に線を引くか(既定 true)。
    Property<bool> grid_visible;
    /// 左に行の番号(行の見出し)を出すか(既定 true)。
    Property<bool> row_header_visible;

    bool is_cell_selected(int row, int column) const { return cell_selected(row, column); }
    /// range のセルを選ぶ(add なら今の選択に加える。SINGLE では範囲の最初のセルだけ)。
    void select_range(const CellRange& range, bool add = false) { select_cells(range, add); }
    /// 選んでいるセルの文字(行は改行、列はタブで区切る)。写す・貼るは、Ctrl+C・Ctrl+V と同じ。
    std::string selected_text() const { return selected_cells_text(); }
    void copy() { copy_cells(); }
    void paste() { paste_cells(); }
    /// 選んでいるセル(編集できるもの)を空にする。
    void clear_selected_cells() { clear_cells(); }

protected:
    void cells_changed(bool current_column_changed, bool ranges_changed) override;

private:
    SelectionBehavior get_selection_behavior() const { return ItemView::get_selection_behavior(); }
    void set_selection_behavior(const SelectionBehavior& v) { ItemView::set_selection_behavior(v); }
    int get_current_column() const { return ItemView::get_current_column(); }
    void set_current_column(const int& v) { ItemView::set_current_column(v); }
    std::vector<CellRange> get_selected_ranges() const { return ItemView::get_selected_ranges(); }
    bool get_grid_visible() const { return ItemView::get_grid_visible(); }
    void set_grid_visible(const bool& v) { ItemView::set_grid_visible(v); }
    bool get_row_header_visible() const { return ItemView::get_row_header_visible(); }
    void set_row_header_visible(const bool& v) { ItemView::set_row_header_visible(v); }
};

}  // namespace tokotachi

#endif
