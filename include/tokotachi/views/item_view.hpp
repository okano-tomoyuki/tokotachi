/// @file
/// 項目の一覧・木を表示するビューの共通の部分(ListView・TreeView の基底。docs/adr/0026)。
///
/// 表示するのは見えている行だけで、そのときにモデル(item_model.hpp)へ文字などを問い合わせる。行の高さはすべて同じ。
/// 列(Column)を加えると見出しを出す。列が無ければ 1 列で、ビューの幅いっぱいに描く。
///
/// @code
/// tokotachi::ListView* list_view_files = new tokotachi::ListView(&window);
/// list_view_files->add_column("名前", 200);
/// list_view_files->add_column("大きさ", 80)->alignment = tokotachi::Align::END;
/// list_view_files->model = files;
/// list_view_files->selection_mode = tokotachi::SelectionMode::MULTI;
/// list_view_files->on_activate = [=](tokotachi::Object*) { open(list_view_files->current); };
/// @endcode
#ifndef TOKOTACHI_VIEWS_ITEM_VIEW_HPP
#define TOKOTACHI_VIEWS_ITEM_VIEW_HPP

#include <map>
#include <set>
#include <string>
#include <vector>

#include "../layout/alignment.hpp"
#include "item_model.hpp"
#include "../graphics/text_layout.hpp"
#include "../widgets/widget.hpp"

namespace tokotachi
{

class ItemView;
class LineEdit;
class ScrollBar;
class Timer;

enum class SelectionMode
{
    NONE,    ///< 選ばない(current だけが動く)
    SINGLE,  ///< 1 つだけ選ぶ(current と同じ項目)
    MULTI    ///< Ctrl+クリック・Shift+クリック・Shift+矢印・Ctrl+A で複数を選ぶ
};

/// ビューの 1 つの列。ItemView が作り、所有する。
class Column : public Object
{
public:
    /// 見出しの文字。
    Property<std::string> title;
    /// 幅(dp。24 以上)。利用者は見出しの境目のドラッグでも変えられる。最後の列は、ビューに余りがあれば広げて描く。
    Property<float> width;
    /// セルの中の文字のそろえ方(START・CENTER・END。FILL は START と同じ)。
    Property<Align> alignment;

private:
    friend class ItemView;

    Column(ItemView* view, const std::string& title, float width);
    ~Column();

    void set_title(const std::string& v);
    void set_width(const float& v);
    void set_alignment(const Align& v);

    ItemView* view_;
    std::string title_;
    float width_;
    Align alignment_;
    mutable TextLayout layout_;
};

class ItemView : public Widget
{
public:
    ~ItemView();

    /// 表示するモデル(所有しない)。モデルが破棄されると nullptr になる。
    Property<ItemModel*> model;

    /// 列を最後に加える・除く。
    Column* add_column(const std::string& title, float width = 120);
    void remove_column(Column* column);
    ReadOnlyProperty<std::vector<Column*> > columns;
    /// 列があるときに見出しを出すか(既定 true)。
    Property<bool> header_visible;

    /// 見出しを押して並べ替えられるか(既定 false)。押すたびに昇順・降順を切り替え、モデルの sort を呼ぶ。
    Property<bool> sorting_enabled;
    /// 並べ替えている列(-1 なら無し)と向き。代入するとモデルの sort を呼ぶ。
    Property<int> sort_column;
    Property<SortOrder> sort_order;

    /// 選び方(既定 SINGLE)。
    Property<SelectionMode> selection_mode;
    /// キーで動かす位置の項目(無ければ無効な ModelIndex)。SINGLE では代入すると選ぶ。
    Property<ModelIndex> current;
    /// 選んでいる項目(表示の順。TreeView で閉じた親の中にあるものは最後)。
    ReadOnlyProperty<std::vector<ModelIndex> > selection;

    bool is_selected(const ModelIndex& index) const;
    /// index を選ぶ・選ぶのをやめる(SINGLE では他の選択を外し、current も移す。NONE では何もしない)。
    void select(const ModelIndex& index, bool selected = true);
    /// 見えているすべての項目を選ぶ(MULTI のときだけ)。
    void select_all();
    void clear_selection();
    /// index の行が見えるようにスクロールする(TreeView では祖先を開く)。
    void scroll_to(const ModelIndex& index);
    /// 位置 p(ビューの座標)の行の項目(無ければ無効な ModelIndex)。
    ModelIndex index_at(const Point& p) const;

    /// index の column 列目の文字を、その場で編集し始める(モデルの editable が false なら何もしない。docs/adr/0046)。
    /// 利用者は F2・選んでいる項目のクリックでも始められる。Enter・ほかへのフォーカスの移動で確定し(モデルの set_text を呼ぶ)、
    /// Esc でやめる。スクロールすると確定し、モデルの行が変わるとやめる。
    void edit(const ModelIndex& index, int column = 0);
    bool is_editing() const;
    /// 編集を終える(commit が false なら、文字を変えずにやめる)。
    void finish_editing(bool commit = true);

    /// 選んでいる項目をドラッグできるか(既定 false。docs/adr/0047)。文字は 1 列目を改行でつないだもの。
    Property<bool> drag_enabled;
    /// 項目の間・項目の上(TreeView では子にする)に落とせるか(既定 false)。同じモデルの項目はモデルの move_rows で移し、
    /// ほかのデータはモデルの can_drop・drop_data に渡し、モデルが受け取らなければ on_drag_move・on_drop に任せる。
    Property<bool> drop_enabled;

    /// selection が変わった(利用者の操作・呼び出し・モデルの変更のどれでも)。
    Property<NotifyHandler> on_selection_change;
    /// 項目をダブルクリックした・Enter を押した(対象は current)。
    Property<NotifyHandler> on_activate;

protected:
    /// tree が true なら木として(子を開いて)表示する。
    ItemView(Widget* parent, bool tree);

    void paint(Canvas& canvas) override;
    void mouse_down(MouseEvent& e) override;
    void mouse_move(MouseEvent& e) override;
    void mouse_up(MouseEvent& e) override;
    void mouse_leave() override;
    void mouse_wheel(WheelEvent& e) override;
    void key_down(KeyEvent& e) override;
    void focus_in() override;
    void focus_out() override;
    void drag_move(DragEvent& e) override;
    void drag_leave() override;
    void drop(DragEvent& e) override;
    void scale_changed() override;
    void theme_changed() override;
    void arrange_children() override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

    /// TreeView のための操作。
    void expand(const ModelIndex& index);
    void collapse(const ModelIndex& index);
    bool is_expanded(const ModelIndex& index) const;
    void expand_all();
    void collapse_all();
    float get_indentation() const { return indentation_; }
    void set_indentation(const float& v);

private:
    friend class ItemModel;
    friend class Column;

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
