// ItemView の実装(内部。item_view.cpp・table_view.cpp・item_model.cpp が使う。docs/adr/0057)。
#ifndef TOKOTACHI_SRC_VIEWS_ITEM_VIEW_IMPL_HPP
#define TOKOTACHI_SRC_VIEWS_ITEM_VIEW_IMPL_HPP

#include <set>
#include <string>
#include <vector>
#include <tokotachi/views/item_view.hpp>
#include "../widgets/widget_access.hpp"
#include "../core/impl.hpp"

namespace tokotachi
{

class ItemView::Impl : public detail::WidgetHooks
{
    TOKOTACHI_IMPL(ItemView);

    Impl(ItemView* object, bool tree, bool table) : q(object), tree_(tree), table_(table) {}

    /// TreeView のための操作。
    void expand(const ModelIndex& index);
    void collapse(const ModelIndex& index);
    bool is_expanded(const ModelIndex& index) const;
    void expand_all();
    void collapse_all();
    float get_indentation() const;
    void set_indentation(const float& v);
    /// TableView のための操作(docs/adr/0050)。
    SelectionBehavior get_selection_behavior() const;
    void set_selection_behavior(const SelectionBehavior& v);
    int get_current_column() const;
    void set_current_column(const int& v);
    std::vector<CellRange> get_selected_ranges() const;
    bool cell_selected(int row, int column) const;
    void select_cells(const CellRange& range, bool add);
    bool get_grid_visible() const;
    void set_grid_visible(const bool& v);
    bool get_row_header_visible() const;
    void set_row_header_visible(const bool& v);
    /// 選んでいるセルの文字(行は改行、列はタブで区切る)。
    std::string selected_cells_text() const;
    void copy_cells();
    void paste_cells();
    void clear_cells();
    /// current_column・選んでいるセルの範囲が変わった(TableView がプロパティの変化を知らせる)。
    void cells_changed(bool current_column_changed, bool ranges_changed);

    // 内部のフック(docs/adr/0058)
    bool handles_tab_key() const override;

    struct Row
    {
        ModelIndex index;
        int depth;
        bool has_children;
    };

    ItemModel* get_model() const { return model_; }
    void set_model(ItemModel* const& v);
    std::vector<Column*> get_columns() const { return columns_; }
    bool get_header_visible() const { return header_visible_; }
    void set_header_visible(const bool& v);
    bool get_sorting_enabled() const { return sorting_enabled_; }
    void set_sorting_enabled(const bool& v) { sorting_enabled_ = v; }
    int get_sort_column() const { return sort_column_; }
    void set_sort_column(const int& v);
    SortOrder get_sort_order() const { return sort_order_; }
    void set_sort_order(const SortOrder& v);
    SelectionMode get_selection_mode() const { return selection_mode_; }
    void set_selection_mode(const SelectionMode& v);
    bool get_drag_enabled() const { return drag_enabled_; }
    void set_drag_enabled(const bool& v) { drag_enabled_ = v; }
    bool get_drop_enabled() const { return drop_enabled_; }
    void set_drop_enabled(const bool& v);
    ModelIndex get_current() const { return current_; }
    void set_current(const ModelIndex& v);
    std::vector<ModelIndex> get_selection() const;

    /// モデルからの知らせ。
    void model_rows_inserted(const ModelIndex& parent, int first, int count);
    void model_rows_removing(const ModelIndex& parent, int first, int count);
    void model_data_changed(const ModelIndex& index);
    void model_layout_changed();
    void model_reset();
    void model_destroyed();
    /// モデルの構造が変わった: 行を作り直し、配置をやり直す。
    void structure_changed();
    void forget_state();
    void column_changed();
    void apply_sort();

    /// 見えている行(TreeView では開いた子を含む)。
    void ensure_rows() const;
    int row_total() const;
    Row row_at(int row) const;
    int row_of(const ModelIndex& index) const;
    bool row_enabled(int row) const;

    /// 寸法(dp)。
    float row_height() const;
    float header_height() const;
    Rect viewport() const;
    float content_width() const;
    /// 列 c の左端(中身の座標)と、描く幅(最後の列は余りまで広げる)。
    float column_x(int c) const;
    float column_draw_width(int c) const;
    int column_count() const;
    /// 行の中の、木の印・チェックボックス・画像・文字の位置(中身の座標の x)。
    float content_start(int depth) const;
    Rect check_rect(int row) const;
    Rect arrow_rect(int row) const;
    /// 位置 p(ビューの座標)の行(無ければ -1)。見出しの上なら -1。
    int row_at_point(const Point& p) const;
    /// 見出しの上で、境目(幅を変えられる)にあたる列(無ければ -1)・押した列。
    int header_divider_at(const Point& p) const;
    int header_column_at(const Point& p) const;

    const TextLayout& text_layout(const std::string& text, const Font& font = Font()) const;
    void paint_header(Canvas& canvas, const Rect& view);
    void paint_row(Canvas& canvas, int row, const Rect& view);

    /// 選択と current を変える。変わったら知らせ、ハンドラを呼ぶ。
    void change_current(const ModelIndex& index);
    void change_selection(const std::set<ModelIndex>& next);
    /// 利用者が行 row へ動いた(クリック・キー)。修飾キーで選び方を変える。
    void move_to(int row, Modifiers modifiers, bool toggle);
    /// from から step の向きで、選べる行を探す(無ければ -1)。
    int next_enabled_row(int from, int step) const;
    void reveal_row(int row);
    void toggle_expanded(int row);
    void activate();
    /// 押した文字で始まる項目へ動く。文字のキーでなければ false。
    bool search_key(const KeyEvent& e);
    /// 中身の座標 x の列(列が無ければ 0。どの列でもなければ -1)。
    int column_at(float x) const;
    /// row 行 column 列目の文字の範囲(ビューの座標。編集の欄を置く)。
    Rect cell_text_rect(int row, int column) const;
    /// index の、編集できる最初の列(無ければ -1)。
    int first_editable_column(const ModelIndex& index) const;

    // ---- 表(table_view.cpp) ----
    float row_header_width() const;
    void paint_row_header(Canvas& canvas, const Rect& view);
    void paint_grid(Canvas& canvas, const Rect& view);
    void table_mouse_down(MouseEvent& e);
    void table_mouse_move(MouseEvent& e);
    void table_key_down(KeyEvent& e);
    /// 行 row・列 column のセルへ動く(shift なら起点からの矩形に広げ、add なら新しい範囲として加える)。
    void move_to_cell(int row, int column, bool shift, bool add);
    void set_ranges(const std::vector<CellRange>& ranges);
    void change_current_column(int column);
    void reveal_column(int column);
    /// 位置 p(ビューの座標)のセル。行の見出しの上なら column は -1。
    bool cell_at_point(const Point& p, int& row, int& column) const;
    /// 行の数が変わった・並びが変わった: セルの範囲を current のセルだけにする。
    void reset_ranges();
    /// current のセルを編集し始め、text を打ち込んだことにする(文字のキーで始めたとき)。
    void start_typing_edit();

    /// 落とす位置(docs/adr/0047)。BEFORE は row 行の前、ONTO は row 行の項目の子の終わり、END は一番上の終わり。
    enum class DropPlace
    {
        NONE,
        BEFORE,
        ONTO,
        END
    };
    struct DropTarget
    {
        DropPlace place;
        int row;            ///< 印を描く行(BEFORE はこの行の上の線、ONTO はこの行の枠)
        int depth;          ///< 入れる深さ(線の字下げ)
        ModelIndex parent;  ///< モデルの親
        int position;       ///< 親の子の位置(-1 なら終わり)
    };
    /// 位置 p(ビューの座標)に落とすときの位置。
    DropTarget drop_target_at(const Point& p) const;
    /// e のデータを target に落とせるか。同じモデルの項目なら、移す項目を indexes に入れる。
    bool can_accept(const DragEvent& e, const DropTarget& target, std::vector<ModelIndex>& indexes) const;
    void paint_drop_indicator(Canvas& canvas, const Rect& view);
    /// 古い ModelIndex の行をずらした値。
    static ModelIndex shifted(const ModelIndex& index, int delta);

    ItemModel* model_;
    std::vector<Column*> columns_;
    bool header_visible_;
    bool sorting_enabled_;
    int sort_column_;
    SortOrder sort_order_;
    SelectionMode selection_mode_;
    ModelIndex current_;
    ModelIndex anchor_;  ///< Shift で範囲を選ぶときの起点
    std::set<ModelIndex> selected_;
    NotifyHandler on_selection_change_;
    NotifyHandler on_activate_;

    ScrollBar* vertical_;
    ScrollBar* horizontal_;
    int hover_row_;
    int resizing_column_;  ///< 幅をドラッグしている列(-1 なら無し)
    float resize_origin_;  ///< ドラッグを始めた x と幅
    float resize_width_;
    int pressed_column_;  ///< 押した見出し(-1 なら無し)
    std::string search_text_;  ///< 文字での検索で、続けて押した文字(小文字)
    double search_time_;       ///< 最後に押した時刻(秒)
    LineEdit* editor_;         ///< セルの編集の欄(初めて編集するときに作り、使い回す)
    ModelIndex edit_index_;    ///< 編集している項目(していなければ無効)
    int edit_column_;
    Timer* edit_timer_;        ///< 選んでいる項目のクリックから編集を始めるまで待つ
    ModelIndex pending_edit_;
    int pending_column_;
    bool drag_enabled_;
    bool drop_enabled_;
    int press_row_;            ///< 押した行(ドラッグを始める候補。-1 なら無し)
    Point press_pos_;
    bool press_deferred_;      ///< 選んでいる項目を押したので、選択は離したときに変える
    Modifiers press_modifiers_;
    DropTarget drop_target_;   ///< 今の落とす位置(描く)

    const bool tree_;
    const bool table_;
    SelectionBehavior behavior_;
    int current_column_;
    int anchor_column_;              ///< Shift で範囲を選ぶときの起点の列
    std::vector<CellRange> ranges_;  ///< 選んでいるセルの範囲(CELLS)
    bool grid_visible_;
    bool row_header_visible_;
    int selecting_;                  ///< マウスで範囲を選んでいる(0: していない、1: セル、2: 行の見出し)
    float indentation_;
    std::set<void*> expanded_;  ///< 開いている項目の識別子
    mutable std::vector<Row> rows_;
    mutable std::map<void*, int> row_map_;  ///< 識別子から行へ
    mutable bool rows_dirty_;
    /// 文字とフォント(大きさ・太さと傾きの組)ごとのレイアウト
    typedef std::pair<std::string, std::pair<float, int> > LayoutKey;
    mutable std::map<LayoutKey, TextLayout> layouts_;
    mutable float layouts_scale_;
};

}  // namespace tokotachi

#endif
