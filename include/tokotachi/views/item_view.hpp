/// @file
/// tokotachi::ItemView の宣言。
#ifndef TOKOTACHI_VIEWS_ITEM_VIEW_HPP
#define TOKOTACHI_VIEWS_ITEM_VIEW_HPP

#include <map>
#include <set>
#include <string>
#include <vector>

#include "../core/export.hpp"
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

/// 選び方。
enum class SelectionMode
{
    NONE,    ///< 選ばない(current だけが動く)
    SINGLE,  ///< 1 つだけ選ぶ(current と同じ項目)
    MULTI    ///< Ctrl+クリック・Shift+クリック・Shift+矢印・Ctrl+A で複数を選ぶ
};

/// 表(TableView)で選ぶ単位(docs/adr/0050)。
enum class SelectionBehavior
{
    CELLS,  ///< セル(矩形の範囲)を選ぶ
    ROWS    ///< 行を選ぶ(一覧と同じ。キーで動かすセルは current_column)
};

/// 表のセルの矩形の範囲(行・列の番号。両端を含む)。
/// @ingroup views
struct TOKOTACHI_API CellRange
{
    /// 最初の行。
    int first_row;
    /// 最初の列。
    int first_column;
    /// 最後の行(含む)。
    int last_row;
    /// 最後の列(含む)。
    int last_column;

    CellRange() : first_row(0), first_column(0), last_row(-1), last_column(-1) {}
    /// 2 つの隅から作る(どちらの隅が先でもよい)。
    CellRange(int row_a, int column_a, int row_b, int column_b);

    /// セルを含まないか。
    bool is_empty() const { return last_row < first_row || last_column < first_column; }
    /// セル (row, column) を含むか。
    bool contains(int row, int column) const
    {
        return row >= first_row && row <= last_row && column >= first_column && column <= last_column;
    }
    /// 範囲が等しいか。
    ///@{
    bool operator==(const CellRange& o) const
    {
        return first_row == o.first_row && first_column == o.first_column && last_row == o.last_row &&
               last_column == o.last_column;
    }
    bool operator!=(const CellRange& o) const { return !(*this == o); }
    ///@}
};

/// ビューの 1 つの列。ItemView が作り、所有する。
/// @ingroup views
class TOKOTACHI_API Column final : public Object
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
    class Impl;
    Impl* d_;
};

/// 項目の一覧・木・表を表示するビューの共通の部分(ListView・TreeView・TableView の基底。docs/adr/0026)。
///
/// 表示するのは見えている行だけで、そのときにモデル(item_model.hpp)へ文字などを問い合わせる。行の高さはすべて同じ。
/// 列(Column)を加えると見出しを出す。列が無ければ 1 列で、ビューの幅いっぱいに描く。
///
/// @code
/// tokotachi::ListView* list_view_files = new tokotachi::ListView(window_main);
/// list_view_files->add_column("名前", 200);
/// list_view_files->add_column("大きさ", 80)->alignment = tokotachi::Align::END;
/// list_view_files->model = files;
/// list_view_files->selection_mode = tokotachi::SelectionMode::MULTI;
/// list_view_files->on_activate = [=](tokotachi::Object*) { open(list_view_files->current); };
/// @endcode
///
/// @ingroup views
class TOKOTACHI_API ItemView : public Widget
{
public:
    ~ItemView();

    /// 表示するモデル(所有しない)。モデルが破棄されると nullptr になる。
    Property<ItemModel*> model;

    /// 列を最後に加える・除く。
    Column* add_column(const std::string& title, float width = 120);
    /// 列を除いて破棄する。
    void remove_column(Column* column);
    /// 列(並んだ順)。
    ReadOnlyProperty<std::vector<Column*> > columns;
    /// 列があるときに見出しを出すか(既定 true)。
    Property<bool> header_visible;

    /// 見出しを押して並べ替えられるか(既定 false)。押すたびに昇順・降順を切り替え、モデルの sort を呼ぶ。
    Property<bool> sorting_enabled;
    /// 並べ替えている列(-1 なら無し)と向き。代入するとモデルの sort を呼ぶ。
    Property<int> sort_column;
    /// 並べ替えの向き(既定 ASCENDING)。
    Property<SortOrder> sort_order;

    /// 選び方(既定 SINGLE)。
    Property<SelectionMode> selection_mode;
    /// キーで動かす位置の項目(無ければ無効な ModelIndex)。SINGLE では代入すると選ぶ。
    Property<ModelIndex> current;
    /// 選んでいる項目(表示の順。TreeView で閉じた親の中にあるものは最後)。
    ReadOnlyProperty<std::vector<ModelIndex> > selection;

    /// index を選んでいるか。
    bool is_selected(const ModelIndex& index) const;
    /// index を選ぶ・選ぶのをやめる(SINGLE では他の選択を外し、current も移す。NONE では何もしない)。
    void select(const ModelIndex& index, bool selected = true);
    /// 見えているすべての項目を選ぶ(MULTI のときだけ)。
    void select_all();
    /// すべての選択を外す。
    void clear_selection();
    /// index の行が見えるようにスクロールする(TreeView では祖先を開く)。
    void scroll_to(const ModelIndex& index);
    /// 位置 p(ビューの座標)の行の項目(無ければ無効な ModelIndex)。
    ModelIndex index_at(const Point& p) const;

    /// index の column 列目の文字を、その場で編集し始める(モデルの editable が false なら何もしない。docs/adr/0046)。
    /// 利用者は F2・選んでいる項目のクリックでも始められる。Enter・ほかへのフォーカスの移動で確定し(モデルの set_text を呼ぶ)、
    /// Esc でやめる。スクロールすると確定し、モデルの行が変わるとやめる。
    void edit(const ModelIndex& index, int column = 0);
    /// セルを編集しているか(edit から finish_editing まで)。
    ReadOnlyProperty<bool> editing;
    /// 編集を終える(commit が false なら、文字を変えずにやめる)。
    void finish_editing(bool commit = true);

    /// 選んでいる項目をドラッグできるか(既定 false。docs/adr/0047)。文字は 1 列目を改行でつないだもの。
    Property<bool> drag_enabled;
    /// 項目の間・項目の上(TreeView では子にする)に落とせるか(既定 false)。同じモデルの項目はモデルの move_rows で移し、
    /// ほかのデータはモデルの can_drop・drop_data に渡し、モデルが受け取らなければ on_drag_move・on_drop に任せる。
    /// 部品がドロップを受け取るか(Widget::accepts_drops)とは別で、項目の間に落とせるようにする。代入すると、
    /// accepts_drops も同じ値にする。
    Property<bool> drop_enabled;

    /// selection が変わった(利用者の操作・呼び出し・モデルの変更のどれでも)。
    Property<NotifyHandler> on_selection_change;
    /// 項目をダブルクリックした・Enter を押した(対象は current)。
    Property<NotifyHandler> on_activate;

protected:
    /// tree が true なら木として(子を開いて)表示する。table が true なら表として(セルを選んで)表示する。
    ItemView(Widget* parent, const detail::InternalTag& tag, bool tree, bool table = false);

    void paint(Canvas& canvas) override;
    void mouse_down(MouseEvent& e) override;
    void mouse_move(MouseEvent& e) override;
    void mouse_up(MouseEvent& e) override;
    void mouse_leave() override;
    void mouse_wheel(WheelEvent& e) override;
    void key_down(KeyEvent& e) override;
    void text_input(TextInputEvent& e) override;
    void text_editing(TextEditingEvent& e) override;
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

private:
    template <typename>
    friend struct detail::ImplAccess;
    friend class ItemModel;
    friend class Column;
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
