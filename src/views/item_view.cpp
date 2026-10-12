#include <tokotachi/views/item_view.hpp>

#include <algorithm>
#include <chrono>
#include <climits>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/core/timer.hpp>
#include <tokotachi/widgets/line_edit.hpp>
#include <tokotachi/widgets/scroll_bar.hpp>

#include "item_model_impl.hpp"
#include "column_impl.hpp"
#include "item_view_impl.hpp"
#include "../core/impl.hpp"
#include "../widgets/theme.hpp"
#include "../widgets/widget_access.hpp"

namespace tokotachi
{

namespace
{
const float BORDER = 1;            // 枠の太さ(dp)
const float CELL_PADDING = 8;      // セルの左右の余白
const float TREE_PADDING = 4;      // 木の印の左の余白
const float ROW_PADDING_Y = 4;     // 行の上下の余白
const float ICON_SIZE = 16;        // 画像・チェックボックスの大きさ
const float GAP = 6;               // チェックボックス・画像・文字の間
const float MIN_EDITOR_WIDTH = 40; // セルの編集の欄の最小の幅
const int EDIT_DELAY_MS = 500;
const float DRAG_THRESHOLD = 4;    // 押してからドラッグを始めるまでに動かす距離(dp)     // 選んでいる項目のクリックから編集を始めるまで(ダブルクリックと区別する)
const float MIN_COLUMN = 24;       // 列の最小の幅
const float DIVIDER_GRAB = 4;      // 見出しの境目をつかめる幅(左右それぞれ)
const float DEFAULT_INDENT = 20;   // 木の 1 段の字下げ
const std::size_t LAYOUT_CACHE_LIMIT = 2000;  // 覚えておく文字のレイアウトの数

void paint_check(Canvas& canvas, const Rect& box, bool checked, bool enabled)
{
    if (checked)
    {
        canvas.fill_round_rect(box, 3, enabled ? theme::accent() : theme::accent_disabled());
        Path mark;
        mark.move_to(box.x + box.width * 0.22f, box.y + box.height * 0.52f);
        mark.line_to(box.x + box.width * 0.42f, box.y + box.height * 0.72f);
        mark.line_to(box.x + box.width * 0.78f, box.y + box.height * 0.30f);
        canvas.stroke_path(mark, 2, theme::on_accent(), LineCap::ROUND);
    }
    else
    {
        canvas.fill_round_rect(box, 3, enabled ? theme::field() : theme::field_disabled());
        canvas.stroke_round_rect(box, 3, 1, theme::border());
    }
}

// 木の開閉の印(閉じていれば右向き、開いていれば下向きの山形)。
void paint_arrow(Canvas& canvas, const Rect& r, bool expanded, const Color& color)
{
    const float cx = r.x + r.width / 2, cy = r.y + r.height / 2;
    Path mark;
    if (expanded)
    {
        mark.move_to(cx - 4, cy - 2);
        mark.line_to(cx, cy + 2);
        mark.line_to(cx + 4, cy - 2);
    }
    else
    {
        mark.move_to(cx - 2, cy - 4);
        mark.line_to(cx + 2, cy);
        mark.line_to(cx - 2, cy + 4);
    }
    canvas.stroke_path(mark, 1.5f, color, LineCap::ROUND);
}

}  // namespace

// ---- Column ----

Column::Column(ItemView* view, const std::string& initial_title, float initial_width)
    : Object(view),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, title),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, float, width),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, Align, alignment),
      d_(new Impl(this))
{
    d_->view_ = view;
    d_->title_ = initial_title;
    d_->width_ = std::max(MIN_COLUMN, initial_width);
    d_->alignment_ = Align::START;
}

Column::~Column() {
    delete d_;
}

void Column::Impl::set_title(const std::string& v)
{
    title_ = v;
    layout_ = TextLayout();
    if (view_)
    {
        view_->invalidate();
    }
}

void Column::Impl::set_width(const float& v)
{
    width_ = std::max(MIN_COLUMN, v);
    if (view_)
    {
        view_->d_->column_changed();
    }
}

void Column::Impl::set_alignment(const Align& v)
{
    alignment_ = v;
    if (view_)
    {
        view_->invalidate();
    }
}

// ---- ItemView ----

bool ItemView::Impl::handles_tab_key() const { return table_; }

float ItemView::Impl::get_indentation() const { return indentation_; }

SelectionBehavior ItemView::Impl::get_selection_behavior() const { return behavior_; }

int ItemView::Impl::get_current_column() const { return current_column_; }

std::vector<CellRange> ItemView::Impl::get_selected_ranges() const { return ranges_; }

bool ItemView::Impl::get_grid_visible() const { return grid_visible_; }

bool ItemView::Impl::get_row_header_visible() const { return row_header_visible_; }

ItemView::ItemView(Widget* parent_widget, const detail::InternalTag&, bool tree, bool table)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, ItemModel*, model),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::vector<Column*>, columns),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, header_visible),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, sorting_enabled),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, int, sort_column),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, SortOrder, sort_order),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, SelectionMode, selection_mode),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, ModelIndex, current),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::vector<ModelIndex>, selection),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, drag_enabled),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, drop_enabled),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_selection_change),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_activate),
      d_(new Impl(this, tree, table))
{
    d_->install(this);
    d_->model_ = nullptr;
    d_->header_visible_ = true;
    d_->sorting_enabled_ = false;
    d_->sort_column_ = -1;
    d_->sort_order_ = SortOrder::ASCENDING;
    d_->selection_mode_ = SelectionMode::SINGLE;
    d_->vertical_ = new ScrollBar(this, Orientation::VERTICAL);
    d_->horizontal_ = new ScrollBar(this, Orientation::HORIZONTAL);
    d_->hover_row_ = -1;
    d_->resizing_column_ = -1;
    d_->resize_origin_ = 0;
    d_->resize_width_ = 0;
    d_->pressed_column_ = -1;
    d_->search_time_ = 0;
    d_->editor_ = nullptr;
    d_->edit_column_ = 0;
    d_->edit_timer_ = new Timer(this);
    d_->pending_column_ = 0;
    d_->drag_enabled_ = false;
    d_->drop_enabled_ = false;
    d_->press_row_ = -1;
    d_->press_deferred_ = false;
    d_->press_modifiers_ = MODIFIER_NONE;
    d_->behavior_ = SelectionBehavior::CELLS;
    d_->current_column_ = 0;
    d_->anchor_column_ = 0;
    d_->grid_visible_ = true;
    d_->row_header_visible_ = true;
    d_->selecting_ = 0;
    d_->indentation_ = DEFAULT_INDENT;
    d_->rows_dirty_ = true;
    d_->layouts_scale_ = 0;
    focus_policy = FocusPolicy::TAB_AND_CLICK;
    if (d_->table_)
    {
        accepts_text_input = true;  // 文字を打つとセルの編集を始める(IME の変換も受ける)
    }
    d_->drop_target_.place = Impl::DropPlace::NONE;
    d_->drop_target_.row = -1;
    d_->drop_target_.depth = 0;
    d_->drop_target_.position = -1;
    d_->vertical_->visible = false;
    d_->horizontal_->visible = false;
    const NotifyHandler moved = [this](Object*)
    {
        // スクロールしたら、編集の欄が行からずれるので確定する
        finish_editing(true);
        d_->hover_row_ = -1;
        invalidate();
    };
    d_->vertical_->on_change = moved;
    d_->horizontal_->on_change = moved;
    // 選んでいる項目をもう一度押したら、ダブルクリックでないと分かってから編集する
    d_->edit_timer_->interval = EDIT_DELAY_MS;
    d_->edit_timer_->repeat = false;
    d_->edit_timer_->on_timer = [this](Object*)
    {
        if (d_->pending_edit_.is_valid() && d_->pending_edit_ == d_->current_)
        {
            edit(d_->pending_edit_, d_->pending_column_);
        }
        d_->pending_edit_ = ModelIndex();
    };
}

ItemView::~ItemView()
{
    if (d_->model_)
    {
        std::vector<ItemView*>& views = d_->model_->d_->views_;
        views.erase(std::remove(views.begin(), views.end(), this), views.end());
    }
    for (std::size_t i = 0; i < d_->columns_.size(); ++i)
    {
        d_->columns_[i]->d_->view_ = nullptr;
    }
    delete d_;
}

// ---- モデル ----

void ItemView::Impl::set_model(ItemModel* const& v)
{
    if (v == model_)
    {
        return;
    }
    if (model_)
    {
        std::vector<ItemView*>& views = model_->d_->views_;
        views.erase(std::remove(views.begin(), views.end(), q), views.end());
    }
    model_ = v;
    if (model_)
    {
        model_->d_->views_.push_back(q);
    }
    vertical_->value = 0;
    horizontal_->value = 0;
    forget_state();
    apply_sort();
}

void ItemView::Impl::forget_state()
{
    // 選択などを捨てる(モデルを替えた・すべてが変わった)。
    expanded_.clear();
    anchor_ = ModelIndex();
    const bool had_current = current_.is_valid();
    current_ = ModelIndex();
    structure_changed();
    if (had_current)
    {
        q->notify_changed(q->current);
    }
    change_selection(std::set<ModelIndex>());
}

void ItemView::Impl::structure_changed()
{
    // 行が変わったら、編集している位置が分からなくなるのでやめる
    q->finish_editing(false);
    rows_dirty_ = true;
    hover_row_ = -1;
    if (table_)
    {
        reset_ranges();
    }
    detail::WidgetAccess::mark_layout_dirty(q);
    q->invalidate();
}

ModelIndex ItemView::Impl::shifted(const ModelIndex& index, int delta) { return ModelIndex(index.row() + delta, index.id()); }

void ItemView::Impl::model_rows_inserted(const ModelIndex& parent, int first, int count)
{
    // 同じ親の、後ろの行の番号をずらす。
    const ModelIndex old_current = current_;
    struct Shift
    {
        const ItemModel* model;
        ModelIndex parent;
        int first, count;
        ModelIndex operator()(const ModelIndex& i) const
        {
            return i.is_valid() && i.row() >= first && model->parent_of(i) == parent ? shifted(i, count) : i;
        }
    };
    const Shift shift = {model_, parent, first, count};
    current_ = shift(current_);
    anchor_ = shift(anchor_);
    std::set<ModelIndex> next;
    for (std::set<ModelIndex>::const_iterator it = selected_.begin(); it != selected_.end(); ++it)
    {
        next.insert(shift(*it));
    }
    selected_.swap(next);
    structure_changed();
    if (current_ != old_current)
    {
        q->notify_changed(q->current);
    }
    q->notify_changed(q->selection);  // 番号が変わった
}

void ItemView::Impl::model_rows_removing(const ModelIndex& parent, int first, int count)
{
    // 除く項目(とその子孫)を忘れ、同じ親の後ろの行の番号をずらす。モデルはまだ除く前の状態。
    struct Fate
    {
        const ItemModel* model;
        ModelIndex parent;
        int first, count;
        // 除かれるなら false。残るなら index に新しい値を入れる。
        bool operator()(ModelIndex& index) const
        {
            if (!index.is_valid())
            {
                return true;
            }
            ModelIndex p = index;
            while (p.is_valid())
            {
                const ModelIndex pp = model->parent_of(p);
                if (pp == parent)
                {
                    if (p.row() >= first && p.row() < first + count)
                    {
                        return false;
                    }
                    if (p == index && p.row() >= first + count)
                    {
                        index = shifted(index, -count);
                    }
                    return true;
                }
                p = pp;
            }
            return true;
        }
    };
    const Fate fate = {model_, parent, first, count};

    // 開いている項目のうち、除くものを忘れる(同じアドレスに新しい項目が作られても開いて見えないように)。
    if (!expanded_.empty())
    {
        std::vector<ModelIndex> stack;
        for (int r = first; r < first + count; ++r)
        {
            stack.push_back(model_->index(r, parent));
        }
        while (!stack.empty())
        {
            const ModelIndex i = stack.back();
            stack.pop_back();
            if (!i.is_valid())
            {
                continue;
            }
            expanded_.erase(i.id());
            const int n = model_->row_count(i);
            for (int r = 0; r < n; ++r)
            {
                stack.push_back(model_->index(r, i));
            }
        }
    }

    const ModelIndex old_current = current_;
    if (!fate(current_))
    {
        current_ = ModelIndex();
    }
    if (!fate(anchor_))
    {
        anchor_ = ModelIndex();
    }
    std::set<ModelIndex> next;
    bool lost = false;
    for (std::set<ModelIndex>::const_iterator it = selected_.begin(); it != selected_.end(); ++it)
    {
        ModelIndex i = *it;
        if (fate(i))
        {
            next.insert(i);
        }
        else
        {
            lost = true;
        }
    }
    selected_.swap(next);
    structure_changed();
    if (current_ != old_current)
    {
        q->notify_changed(q->current);
    }
    q->notify_changed(q->selection);
    if (lost)
    {
        Object* self = q;
        detail::fire(on_selection_change_, self);
    }
}

void ItemView::Impl::model_data_changed(const ModelIndex&)
{
    q->invalidate();
}

void ItemView::Impl::model_layout_changed()
{
    // 並びが変わった: 識別子から今の位置を引き直す。
    const ModelIndex old_current = current_;
    current_ = model_->locate(current_);
    anchor_ = model_->locate(anchor_);
    std::set<ModelIndex> next;
    for (std::set<ModelIndex>::const_iterator it = selected_.begin(); it != selected_.end(); ++it)
    {
        const ModelIndex i = model_->locate(*it);
        if (i.is_valid())
        {
            next.insert(i);
        }
    }
    const bool lost = next.size() != selected_.size();
    selected_.swap(next);
    structure_changed();
    if (current_ != old_current)
    {
        q->notify_changed(q->current);
    }
    q->notify_changed(q->selection);
    if (lost)
    {
        Object* self = q;
        detail::fire(on_selection_change_, self);
    }
}

void ItemView::Impl::model_reset() { forget_state(); }

void ItemView::Impl::model_destroyed()
{
    std::vector<ItemView*>& views = model_->d_->views_;
    views.erase(std::remove(views.begin(), views.end(), q), views.end());
    model_ = nullptr;
    q->notify_changed(q->model);
    forget_state();
}

// ---- 列と並べ替え ----

Column* ItemView::add_column(const std::string& title_value, float width_value)
{
    Column* c = new Column(this, title_value, width_value);
    d_->columns_.push_back(c);
    notify_changed(columns);
    request_layout();
    d_->column_changed();
    return c;
}

void ItemView::remove_column(Column* column)
{
    const std::vector<Column*>::iterator it = std::find(d_->columns_.begin(), d_->columns_.end(), column);
    if (it == d_->columns_.end())
    {
        return;
    }
    const int c = static_cast<int>(it - d_->columns_.begin());
    d_->columns_.erase(it);
    column->d_->view_ = nullptr;
    delete column;
    if (d_->sort_column_ == c)
    {
        d_->sort_column_ = -1;
        notify_changed(sort_column);
    }
    else if (d_->sort_column_ > c)
    {
        --d_->sort_column_;
        notify_changed(sort_column);
    }
    notify_changed(columns);
    request_layout();
    d_->column_changed();
}

void ItemView::Impl::column_changed()
{
    detail::WidgetAccess::mark_layout_dirty(q);
    q->invalidate();
}

void ItemView::Impl::set_header_visible(const bool& v)
{
    header_visible_ = v;
    q->request_layout();
    column_changed();
}

void ItemView::Impl::set_sort_column(const int& v)
{
    sort_column_ = std::max(-1, v);
    apply_sort();
    q->invalidate();
}

void ItemView::Impl::set_sort_order(const SortOrder& v)
{
    sort_order_ = v;
    apply_sort();
    q->invalidate();
}

void ItemView::Impl::apply_sort()
{
    if (model_ && sort_column_ >= 0)
    {
        model_->sort(sort_column_, sort_order_);
    }
}

// ---- 選択 ----

void ItemView::Impl::set_selection_mode(const SelectionMode& v)
{
    selection_mode_ = v;
    if (table_ && behavior_ == SelectionBehavior::CELLS)
    {
        std::vector<CellRange> next = ranges_;
        if (v == SelectionMode::NONE)
        {
            next.clear();
        }
        else if (v == SelectionMode::SINGLE)
        {
            const int r = row_of(current_);
            next.clear();
            if (r >= 0)
            {
                next.push_back(CellRange(r, current_column_, r, current_column_));
            }
        }
        set_ranges(next);
        return;
    }
    std::set<ModelIndex> next = selected_;
    if (v == SelectionMode::NONE)
    {
        next.clear();
    }
    else if (v == SelectionMode::SINGLE && next.size() > 1)
    {
        next.clear();
        if (current_.is_valid() && selected_.count(current_))
        {
            next.insert(current_);
        }
    }
    change_selection(next);
}

void ItemView::Impl::set_current(const ModelIndex& v)
{
    if (table_ && behavior_ == SelectionBehavior::CELLS)
    {
        const int r = row_of(v);
        if (r >= 0)
        {
            move_to_cell(r, current_column_, false, false);
        }
        else
        {
            anchor_ = ModelIndex();
            change_current(ModelIndex());
            set_ranges(std::vector<CellRange>());
        }
        return;
    }
    if (selection_mode_ == SelectionMode::SINGLE)
    {
        if (v.is_valid())
        {
            q->select(v);
            return;
        }
        q->clear_selection();
    }
    anchor_ = v;
    change_current(v);
}

void ItemView::Impl::change_current(const ModelIndex& index)
{
    if (index == current_)
    {
        return;
    }
    current_ = index;
    q->invalidate();
    q->notify_changed(q->current);
}

void ItemView::Impl::change_selection(const std::set<ModelIndex>& next)
{
    if (next == selected_)
    {
        return;
    }
    selected_ = next;
    q->invalidate();
    q->notify_changed(q->selection);
    Object* self = q;
    detail::fire(on_selection_change_, self);
}

std::vector<ModelIndex> ItemView::Impl::get_selection() const
{
    if (table_ && behavior_ == SelectionBehavior::CELLS)
    {
        // 選んでいるセルのある行(表示の順)
        std::vector<ModelIndex> rows;
        const int n = row_total();
        for (int r = 0; r < n; ++r)
        {
            for (std::size_t k = 0; k < ranges_.size(); ++k)
            {
                if (r >= ranges_[k].first_row && r <= ranges_[k].last_row)
                {
                    rows.push_back(row_at(r).index);
                    break;
                }
            }
        }
        return rows;
    }
    // 表示の順に並べる(見えていないものは最後)。
    std::vector<std::pair<int, ModelIndex> > order;
    for (std::set<ModelIndex>::const_iterator it = selected_.begin(); it != selected_.end(); ++it)
    {
        const int r = row_of(*it);
        order.push_back(std::make_pair(r < 0 ? INT_MAX : r, *it));
    }
    std::stable_sort(order.begin(), order.end(),
                     [](const std::pair<int, ModelIndex>& a, const std::pair<int, ModelIndex>& b)
                     { return a.first < b.first; });
    std::vector<ModelIndex> out;
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        out.push_back(order[i].second);
    }
    return out;
}

bool ItemView::is_selected(const ModelIndex& index) const
{
    if (d_->table_ && d_->behavior_ == SelectionBehavior::CELLS)
    {
        const int r = d_->row_of(index);
        for (std::size_t k = 0; k < d_->ranges_.size() && r >= 0; ++k)
        {
            if (r >= d_->ranges_[k].first_row && r <= d_->ranges_[k].last_row)
            {
                return true;
            }
        }
        return false;
    }
    return d_->selected_.count(index) != 0;
}

void ItemView::select(const ModelIndex& index, bool selected)
{
    if (!index.is_valid() || d_->selection_mode_ == SelectionMode::NONE)
    {
        return;
    }
    if (d_->table_ && d_->behavior_ == SelectionBehavior::CELLS)
    {
        // 表: 行のすべてのセル
        const int r = d_->row_of(index);
        if (r < 0)
        {
            return;
        }
        const CellRange whole(r, 0, r, d_->column_count() - 1);
        if (!selected)
        {
            std::vector<CellRange> next;
            for (std::size_t k = 0; k < d_->ranges_.size(); ++k)
            {
                if (d_->ranges_[k] != whole)
                {
                    next.push_back(d_->ranges_[k]);
                }
            }
            d_->set_ranges(next);
            return;
        }
        if (d_->selection_mode_ == SelectionMode::SINGLE)
        {
            d_->move_to_cell(r, d_->current_column_, false, false);
            return;
        }
        d_->select_cells(whole, true);
        return;
    }
    std::set<ModelIndex> next = d_->selected_;
    if (d_->selection_mode_ == SelectionMode::SINGLE)
    {
        next.clear();
        if (selected)
        {
            next.insert(index);
            d_->anchor_ = index;
            d_->change_current(index);
        }
    }
    else if (selected)
    {
        next.insert(index);
    }
    else
    {
        next.erase(index);
    }
    d_->change_selection(next);
}

void ItemView::select_all()
{
    if (d_->selection_mode_ != SelectionMode::MULTI)
    {
        return;
    }
    if (d_->table_ && d_->behavior_ == SelectionBehavior::CELLS)
    {
        std::vector<CellRange> all;
        if (d_->row_total() > 0)
        {
            all.push_back(CellRange(0, 0, d_->row_total() - 1, d_->column_count() - 1));
        }
        d_->set_ranges(all);
        return;
    }
    std::set<ModelIndex> next;
    const int n = d_->row_total();
    for (int r = 0; r < n; ++r)
    {
        if (d_->row_enabled(r))
        {
            next.insert(d_->row_at(r).index);
        }
    }
    d_->change_selection(next);
}

void ItemView::clear_selection()
{
    if (d_->table_ && d_->behavior_ == SelectionBehavior::CELLS)
    {
        d_->set_ranges(std::vector<CellRange>());
        return;
    }
    d_->change_selection(std::set<ModelIndex>());
}

void ItemView::Impl::move_to(int row, Modifiers modifiers, bool toggle)
{
    if (row < 0 || row >= row_total())
    {
        return;
    }
    const ModelIndex index = row_at(row).index;
    const bool shift = (modifiers & MODIFIER_SHIFT) != 0;
    const bool ctrl = (modifiers & MODIFIER_CONTROL) != 0;
    reveal_row(row);
    if (selection_mode_ == SelectionMode::NONE)
    {
        anchor_ = index;
        change_current(index);
        return;
    }
    if (selection_mode_ == SelectionMode::SINGLE || (!shift && !ctrl))
    {
        anchor_ = index;
        change_current(index);
        std::set<ModelIndex> next;
        next.insert(index);
        change_selection(next);
        return;
    }
    if (shift)
    {
        // 起点からこの行までを選ぶ(Ctrl も押していれば今の選択に加える)。
        int from = row_of(anchor_);
        if (from < 0)
        {
            from = row;
            anchor_ = index;
        }
        std::set<ModelIndex> next;
        if (ctrl)
        {
            next = selected_;
        }
        const int lo = std::min(from, row), hi = std::max(from, row);
        for (int r = lo; r <= hi; ++r)
        {
            if (row_enabled(r))
            {
                next.insert(row_at(r).index);
            }
        }
        change_current(index);
        change_selection(next);
        return;
    }
    // Ctrl だけ: クリック・Ctrl+Space なら選ぶかを切り替え、矢印なら current だけを動かす。
    anchor_ = index;
    change_current(index);
    if (toggle)
    {
        std::set<ModelIndex> next = selected_;
        if (next.count(index))
        {
            next.erase(index);
        }
        else
        {
            next.insert(index);
        }
        change_selection(next);
    }
}

int ItemView::Impl::next_enabled_row(int from, int step) const
{
    const int n = row_total();
    for (int r = from; r >= 0 && r < n; r += step)
    {
        if (row_enabled(r))
        {
            return r;
        }
    }
    return -1;
}

void ItemView::Impl::activate()
{
    if (current_.is_valid())
    {
        Object* self = q;
        detail::fire(on_activate_, self);
    }
}

// ---- 行 ----

void ItemView::Impl::ensure_rows() const
{
    if (!tree_ || !rows_dirty_)
    {
        return;
    }
    rows_dirty_ = false;
    rows_.clear();
    row_map_.clear();
    if (!model_)
    {
        return;
    }
    // 開いている項目の子を、深さ優先で並べる。
    struct Frame
    {
        ModelIndex parent;
        int next;
        int count;
        int depth;
    };
    std::vector<Frame> stack;
    const Frame top = {ModelIndex(), 0, model_->row_count(), 0};
    stack.push_back(top);
    while (!stack.empty())
    {
        Frame& f = stack.back();
        if (f.next >= f.count)
        {
            stack.pop_back();
            continue;
        }
        const ModelIndex i = model_->index(f.next++, f.parent);
        const int depth = f.depth;
        if (!i.is_valid())
        {
            continue;
        }
        const int children = model_->row_count(i);
        const Row row = {i, depth, children > 0};
        if (i.id())
        {
            row_map_[i.id()] = static_cast<int>(rows_.size());
        }
        rows_.push_back(row);
        if (children > 0 && i.id() && expanded_.count(i.id()))
        {
            const Frame child = {i, 0, children, depth + 1};
            stack.push_back(child);  // f は無効になるので、この後は使わない
        }
    }
}

int ItemView::Impl::row_total() const
{
    if (!model_)
    {
        return 0;
    }
    if (!tree_)
    {
        return model_->row_count();
    }
    ensure_rows();
    return static_cast<int>(rows_.size());
}

ItemView::Impl::Row ItemView::Impl::row_at(int row) const
{
    if (!tree_)
    {
        const Row r = {model_->index(row), 0, false};
        return r;
    }
    ensure_rows();
    return rows_[row];
}

int ItemView::Impl::row_of(const ModelIndex& index) const
{
    if (!model_ || !index.is_valid())
    {
        return -1;
    }
    if (!tree_ || !index.id())
    {
        // 一覧: 一番上の行の番号がそのまま行。
        if (tree_)
        {
            ensure_rows();
        }
        return index.row() < row_total() && !model_->parent_of(index).is_valid() ? index.row() : -1;
    }
    ensure_rows();
    const std::map<void*, int>::const_iterator it = row_map_.find(index.id());
    return it != row_map_.end() ? it->second : -1;
}

bool ItemView::Impl::row_enabled(int row) const { return model_ && model_->enabled(row_at(row).index); }

// ---- 寸法 ----

float ItemView::Impl::row_height() const
{
    const float text_h = std::ceil(TextLayout("Ag", Font(), q->display_scale).height());
    return std::max(text_h, ICON_SIZE) + ROW_PADDING_Y * 2;
}

float ItemView::Impl::header_height() const { return header_visible_ && !columns_.empty() ? row_height() : 0; }

Rect ItemView::Impl::viewport() const
{
    const Rect r = q->local_rect;
    const float bar = ScrollBar::THICKNESS;
    const float top = BORDER + header_height();
    const float left = BORDER + row_header_width();
    const float w = r.width - BORDER - left - (vertical_->visible ? bar : 0);
    const float h = r.height - BORDER - top - (horizontal_->visible ? bar : 0);
    return Rect(left, top, std::max(0.0f, w), std::max(0.0f, h));
}

int ItemView::Impl::column_count() const { return columns_.empty() ? 1 : static_cast<int>(columns_.size()); }

float ItemView::Impl::content_width() const
{
    if (columns_.empty())
    {
        return viewport().width;
    }
    float w = 0;
    for (std::size_t i = 0; i < columns_.size(); ++i)
    {
        w += columns_[i]->d_->width_;
    }
    return w;
}

float ItemView::Impl::column_x(int c) const
{
    float x = 0;
    for (int i = 0; i < c && i < static_cast<int>(columns_.size()); ++i)
    {
        x += columns_[i]->d_->width_;
    }
    return x;
}

float ItemView::Impl::column_draw_width(int c) const
{
    const float view_w = viewport().width;
    if (columns_.empty())
    {
        return view_w;
    }
    const float w = columns_[c]->d_->width_;
    if (c == static_cast<int>(columns_.size()) - 1)
    {
        return std::max(w, view_w - column_x(c));
    }
    return w;
}

float ItemView::Impl::content_start(int depth) const
{
    return tree_ ? TREE_PADDING + (depth + 1) * indentation_ : CELL_PADDING;
}

Rect ItemView::Impl::arrow_rect(int row) const
{
    // 行の中の座標(x は中身の左端から、y は行の上端から)。
    const Row r = row_at(row);
    return Rect(TREE_PADDING + r.depth * indentation_, 0, indentation_, row_height());
}

Rect ItemView::Impl::check_rect(int row) const
{
    const Row r = row_at(row);
    return Rect(content_start(r.depth), std::floor((row_height() - ICON_SIZE) / 2), ICON_SIZE, ICON_SIZE);
}

int ItemView::Impl::row_at_point(const Point& p) const
{
    const Rect view = viewport();
    if (!view.contains(p))
    {
        return -1;
    }
    const int r = static_cast<int>(std::floor((p.y - view.y + vertical_->value.get()) / row_height()));
    return r >= 0 && r < row_total() ? r : -1;
}

ModelIndex ItemView::index_at(const Point& p) const
{
    const int r = d_->row_at_point(p);
    return r >= 0 ? d_->row_at(r).index : ModelIndex();
}

int ItemView::Impl::header_column_at(const Point& p) const
{
    const float hh = header_height();
    const Rect view = viewport();
    if (hh <= 0 || p.y < BORDER || p.y >= BORDER + hh || p.x < view.x || p.x >= view.right())
    {
        return -1;
    }
    const float x = p.x - view.x + horizontal_->value.get();
    for (int c = 0; c < static_cast<int>(columns_.size()); ++c)
    {
        const float left = column_x(c);
        if (x >= left && x < left + column_draw_width(c))
        {
            return c;
        }
    }
    return -1;
}

int ItemView::Impl::header_divider_at(const Point& p) const
{
    const float hh = header_height();
    const Rect view = viewport();
    if (hh <= 0 || p.y < BORDER || p.y >= BORDER + hh || p.x < view.x)
    {
        return -1;
    }
    const float x = p.x - view.x + horizontal_->value.get();
    for (int c = 0; c < static_cast<int>(columns_.size()); ++c)
    {
        const float right = column_x(c) + columns_[c]->d_->width_;
        if (std::fabs(x - right) <= DIVIDER_GRAB)
        {
            return c;
        }
    }
    return -1;
}

void ItemView::arrange_children()
{
    const Rect area = local_rect;
    const float bar = ScrollBar::THICKNESS;
    const float hh = d_->header_height();
    const float rh = d_->row_header_width();
    const float content_h = d_->row_total() * d_->row_height();
    const float content_w = d_->content_width();
    // 収まらない向きにだけスクロールバーを出す(片方を出すともう片方が要ることがあるので 2 回確かめる)。
    float view_w = area.width - BORDER * 2 - rh, view_h = area.height - BORDER * 2 - hh;
    bool need_v = false, need_h = false;
    for (int i = 0; i < 2; ++i)
    {
        need_v = content_h > view_h;
        need_h = !d_->columns_.empty() && content_w > view_w;
        view_w = area.width - BORDER * 2 - rh - (need_v ? bar : 0);
        view_h = area.height - BORDER * 2 - hh - (need_h ? bar : 0);
    }
    view_w = std::max(0.0f, view_w);
    view_h = std::max(0.0f, view_h);
    d_->vertical_->visible = need_v;
    d_->horizontal_->visible = need_h;
    d_->vertical_->bounds = Rect(BORDER + rh + view_w, BORDER + hh, bar, view_h);
    d_->horizontal_->bounds = Rect(BORDER + rh, BORDER + hh + view_h, view_w, bar);
    d_->vertical_->page_step = view_h;
    d_->horizontal_->page_step = view_w;
    d_->vertical_->single_step = d_->row_height() * 3;
    d_->vertical_->maximum = need_v ? content_h - view_h : 0;
    d_->horizontal_->maximum = need_h ? content_w - view_w : 0;
}

Size ItemView::compute_preferred_size() const
{
    const float w = d_->columns_.empty() ? 200 : d_->content_width() + ScrollBar::THICKNESS;
    return Size(w + d_->row_header_width() + BORDER * 2, d_->header_height() + d_->row_height() * 8 + BORDER * 2);
}

Size ItemView::compute_minimum_size() const
{
    return Size(80, d_->header_height() + d_->row_height() * 2 + BORDER * 2);
}

void ItemView::Impl::reveal_row(int row)
{
    const Rect view = viewport();
    const float h = row_height();
    const float top = row * h;
    const float scroll = vertical_->value;
    if (top < scroll)
    {
        vertical_->value = top;
    }
    else if (top + h > scroll + view.height)
    {
        vertical_->value = top + h - view.height;
    }
}

void ItemView::scroll_to(const ModelIndex& index)
{
    if (!d_->model_ || !index.is_valid())
    {
        return;
    }
    if (d_->tree_)
    {
        // 祖先を開く。
        for (ModelIndex p = d_->model_->parent_of(index); p.is_valid(); p = d_->model_->parent_of(p))
        {
            if (p.id() && !d_->expanded_.count(p.id()))
            {
                d_->expanded_.insert(p.id());
                d_->structure_changed();
            }
        }
    }
    // スクロールバーの最大値を今の行の数に合わせてから動かす。
    arrange_children();
    const int r = d_->row_of(index);
    if (r >= 0)
    {
        d_->reveal_row(r);
    }
}

// ---- 木 ----

void ItemView::Impl::expand(const ModelIndex& index)
{
    if (!model_ || !index.is_valid() || !index.id() || expanded_.count(index.id()))
    {
        return;
    }
    expanded_.insert(index.id());
    structure_changed();
}

void ItemView::Impl::collapse(const ModelIndex& index)
{
    if (!index.is_valid() || !expanded_.erase(index.id()))
    {
        return;
    }
    // current が閉じた中に隠れたら、閉じた項目へ移す。
    for (ModelIndex p = model_ ? model_->parent_of(current_) : ModelIndex(); p.is_valid(); p = model_->parent_of(p))
    {
        if (p == index)
        {
            if (selection_mode_ == SelectionMode::SINGLE)
            {
                q->select(index);
            }
            else
            {
                anchor_ = index;
                change_current(index);
            }
            break;
        }
    }
    structure_changed();
}

bool ItemView::Impl::is_expanded(const ModelIndex& index) const { return index.id() && expanded_.count(index.id()) != 0; }

void ItemView::Impl::expand_all()
{
    if (!model_)
    {
        return;
    }
    std::vector<ModelIndex> stack;
    stack.push_back(ModelIndex());
    while (!stack.empty())
    {
        const ModelIndex p = stack.back();
        stack.pop_back();
        const int n = model_->row_count(p);
        if (p.is_valid() && n > 0 && p.id())
        {
            expanded_.insert(p.id());
        }
        for (int r = 0; r < n; ++r)
        {
            stack.push_back(model_->index(r, p));
        }
    }
    structure_changed();
}

void ItemView::Impl::collapse_all()
{
    expanded_.clear();
    // current が隠れたら、一番上の祖先へ移す。
    if (model_ && current_.is_valid())
    {
        ModelIndex top = current_;
        while (model_->parent_of(top).is_valid())
        {
            top = model_->parent_of(top);
        }
        if (top != current_)
        {
            if (selection_mode_ == SelectionMode::SINGLE)
            {
                q->select(top);
            }
            else
            {
                anchor_ = top;
                change_current(top);
            }
        }
    }
    structure_changed();
}

void ItemView::Impl::set_indentation(const float& v)
{
    indentation_ = std::max(8.0f, v);
    q->invalidate();
}

void ItemView::Impl::toggle_expanded(int row)
{
    const Row r = row_at(row);
    if (!r.has_children)
    {
        return;
    }
    if (is_expanded(r.index))
    {
        collapse(r.index);
    }
    else
    {
        expand(r.index);
    }
}

// ---- 描画 ----

const TextLayout& ItemView::Impl::text_layout(const std::string& text, const Font& font) const
{
    const float scale = q->display_scale;
    if (scale != layouts_scale_ || layouts_.size() > LAYOUT_CACHE_LIMIT)
    {
        layouts_.clear();
        layouts_scale_ = scale;
    }
    const LayoutKey key(text, std::make_pair(font.size, (font.bold ? 1 : 0) | (font.italic ? 2 : 0)));
    std::map<LayoutKey, TextLayout>::iterator it = layouts_.find(key);
    if (it == layouts_.end())
    {
        it = layouts_.insert(std::make_pair(key, TextLayout(text, font, scale))).first;
    }
    return it->second;
}

void ItemView::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool enabled_now = effectively_enabled;
    canvas.fill_round_rect(r, theme::field_radius(), enabled_now ? theme::field() : theme::field_disabled());

    const Rect view = d_->viewport();
    if (d_->model_)
    {
        const float h = d_->row_height();
        const float scroll = d_->vertical_->value;
        const Rect clip = canvas.clip_bounds().intersected(view);
        if (!clip.is_empty())
        {
            const int first = std::max(0, static_cast<int>(std::floor((clip.y - view.y + scroll) / h)));
            const int last = std::min(d_->row_total() - 1, static_cast<int>(std::floor((clip.bottom() - view.y + scroll) / h)));
            canvas.save();
            canvas.clip_rect(view);
            for (int row = first; row <= last; ++row)
            {
                d_->paint_row(canvas, row, view);
            }
            if (d_->table_)
            {
                d_->paint_grid(canvas, view);
            }
            d_->paint_drop_indicator(canvas, view);
            canvas.restore();
        }
    }
    if (d_->header_height() > 0)
    {
        d_->paint_header(canvas, view);
    }
    if (d_->row_header_width() > 0)
    {
        d_->paint_row_header(canvas, view);
    }
    canvas.stroke_round_rect(r, theme::field_radius(), 1, focused ? theme::accent() : theme::border());
    Widget::paint(canvas);
}

void ItemView::Impl::paint_header(Canvas& canvas, const Rect& view)
{
    const float hh = header_height();
    const Rect bar(BORDER, BORDER, q->local_rect.get().width - BORDER * 2, hh);
    canvas.save();
    canvas.clip_rect(bar);
    canvas.fill_rect(bar, theme::window());
    canvas.fill_rect(Rect(bar.x, bar.bottom() - 1, bar.width, 1), theme::border());
    canvas.clip_rect(Rect(view.x, bar.y, std::max(0.0f, bar.right() - view.x), hh));  // 行の見出しの上(左上の隅)には描かない
    const float x0 = view.x - horizontal_->value.get();
    const bool enabled_now = q->effectively_enabled;
    for (int c = 0; c < static_cast<int>(columns_.size()); ++c)
    {
        const Column* col = columns_[c];
        const float cx = x0 + column_x(c);
        const float cw = column_draw_width(c);
        if (cx > bar.right() || cx + cw < bar.x)
        {
            continue;
        }
        if (col->d_->layout_.is_null() || col->d_->layout_.scale() != q->display_scale.get())
        {
            col->d_->layout_ = TextLayout(col->d_->title_, Font(), q->display_scale);
        }
        const TextLayout& l = col->d_->layout_;
        const bool sorted = sorting_enabled_ && sort_column_ == c;
        const float arrow_space = sorted ? 14 : 0;
        float tx = cx + CELL_PADDING;
        if (col->d_->alignment_ == Align::END)
        {
            tx = cx + cw - CELL_PADDING - arrow_space - l.width();
        }
        else if (col->d_->alignment_ == Align::CENTER)
        {
            tx = cx + (cw - arrow_space - l.width()) / 2;
        }
        canvas.save();
        canvas.clip_rect(Rect(cx, bar.y, cw, hh));
        const Color fg = enabled_now ? theme::color(ColorRole::TEXT_SECONDARY) : theme::text_disabled();
        canvas.draw_text(l, Point(tx, bar.y + (hh - l.height()) / 2), fg);
        if (sorted)
        {
            // 並べ替えの向き(昇順は上向き)。
            const float ax = std::min(tx + l.width() + 8, cx + cw - 10), ay = bar.y + hh / 2;
            Path mark;
            const float d = sort_order_ == SortOrder::ASCENDING ? -1.0f : 1.0f;
            mark.move_to(ax - 3.5f, ay - d * 2);
            mark.line_to(ax, ay + d * 2);
            mark.line_to(ax + 3.5f, ay - d * 2);
            canvas.stroke_path(mark, 1.5f, fg, LineCap::ROUND);
        }
        canvas.restore();
        // 列の境目。
        if (c + 1 < static_cast<int>(columns_.size()))
        {
            canvas.fill_rect(Rect(cx + cw - 1, bar.y + 6, 1, hh - 12), theme::border());
        }
    }
    canvas.restore();
}

void ItemView::Impl::paint_row(Canvas& canvas, int row, const Rect& view)
{
    const Row info = row_at(row);
    const ModelIndex index = info.index;
    const float h = row_height();
    const float y = view.y + row * h - vertical_->value.get();
    const float x0 = view.x - horizontal_->value.get();
    const Rect line(view.x, y, view.width, h);
    const bool enabled_now = q->effectively_enabled && model_->enabled(index);
    const bool cells = table_ && behavior_ == SelectionBehavior::CELLS;
    const bool row_selected = !cells && selected_.count(index) != 0;
    const bool has_focus = q->focused || (editor_ && editor_->focused.get());

    if (row_selected)
    {
        canvas.fill_rect(line, has_focus ? theme::selection() : theme::selection_inactive());
    }
    else if (row == hover_row_ && enabled_now && !table_)
    {
        canvas.fill_rect(line, theme::selection_inactive().with_alpha(96));
    }

    for (int c = 0; c < column_count(); ++c)
    {
        const float cx = x0 + column_x(c);
        const float cw = column_draw_width(c);
        if (cx > view.right() || cx + cw < view.x)
        {
            continue;
        }
        // 表でセルを選ぶときは、セルごとに塗る
        const bool selected = cells ? cell_selected(row, c) : row_selected;
        if (cells && selected)
        {
            canvas.fill_rect(Rect(cx, y, cw, h), has_focus ? theme::selection() : theme::selection_inactive());
        }
        const Color fg = !enabled_now ? theme::text_disabled() : selected && has_focus ? theme::on_selection() : theme::text();
        canvas.save();
        canvas.clip_rect(Rect(cx, y, cw, h));
        float tx = cx + CELL_PADDING;
        if (c == 0)
        {
            if (tree_ && info.has_children)
            {
                const Rect a = arrow_rect(row);
                paint_arrow(canvas, Rect(cx + a.x, y + a.y, a.width, a.height), is_expanded(index),
                            selected && has_focus ? theme::on_selection() : enabled_now ? theme::text() : theme::text_disabled());
            }
            tx = cx + content_start(info.depth);
            if (model_->checkable(index))
            {
                const Rect box = check_rect(row);
                paint_check(canvas, Rect(cx + box.x, y + box.y, box.width, box.height), model_->checked(index), enabled_now);
                tx += ICON_SIZE + GAP;
            }
            const Image icon = model_->icon(index);
            if (!icon.is_null())
            {
                canvas.draw_image(icon, Rect(tx, y + std::floor((h - ICON_SIZE) / 2), ICON_SIZE, ICON_SIZE), fg,
                                  enabled_now || icon.is_icon() ? 1.0f : 0.4f);
                tx += ICON_SIZE + GAP;
            }
        }
        const TextLayout& l = text_layout(model_->text(index, c), model_->font(index, c));
        const Align align = columns_.empty() ? Align::START : columns_[c]->d_->alignment_;
        if (align == Align::END)
        {
            tx = std::max(tx, cx + cw - CELL_PADDING - l.width());
        }
        else if (align == Align::CENTER)
        {
            tx = std::max(tx, cx + (cw - l.width()) / 2);
        }
        // 項目の文字の色(無効な項目・フォーカスのあるビューで選んでいる行は、ビューの色)
        const ThemeColor own = model_->text_color(index, c);
        const Color color = enabled_now && !(selected && has_focus) && own != ThemeColor() ? own.resolve() : fg;
        canvas.draw_text(l, Point(tx, y + (h - l.height()) / 2), color);
        canvas.restore();
    }

    // 表: current のセルを枠で示す
    if (table_)
    {
        if (has_focus && index == current_ && current_column_ < column_count())
        {
            const Rect cell(x0 + column_x(current_column_), y, column_draw_width(current_column_), h);
            canvas.stroke_rect(cell.adjusted(1, 1, 1, 1), 2, theme::focus_ring());
        }
        return;
    }
    // フォーカスのある current の行を枠で示す(SINGLE で選んでいる行は色で分かるので描かない)。
    if (has_focus && index == current_ && (selection_mode_ == SelectionMode::MULTI || !row_selected))
    {
        canvas.stroke_round_rect(line.adjusted(1, 1, 1, 1), 2, 1, theme::focus_ring());
    }
}

// ---- 入力 ----

void ItemView::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    e.accepted = true;
    // 見出し: 境目なら幅を変え始め、それ以外なら押した列を覚える(離したときに並べ替える)。
    const int divider = d_->header_divider_at(e.pos);
    if (divider >= 0)
    {
        d_->resizing_column_ = divider;
        d_->resize_origin_ = e.pos.x;
        d_->resize_width_ = d_->columns_[divider]->d_->width_;
        return;
    }
    if (d_->header_column_at(e.pos) >= 0)
    {
        d_->pressed_column_ = d_->header_column_at(e.pos);
        return;
    }
    if (d_->table_)
    {
        d_->table_mouse_down(e);
        return;
    }
    const int row = d_->row_at_point(e.pos);
    if (row < 0)
    {
        // 行の無いところ: 修飾キーが無ければ選択を外す。
        if (d_->viewport().contains(e.pos) && !(e.modifiers & (MODIFIER_CONTROL | MODIFIER_SHIFT)))
        {
            clear_selection();
        }
        return;
    }
    const Impl::Row info = d_->row_at(row);
    const Rect view = d_->viewport();
    const Point in_row(e.pos.x - view.x + d_->horizontal_->value.get(), e.pos.y - view.y + d_->vertical_->value.get() - row * d_->row_height());
    if (d_->tree_ && info.has_children && d_->arrow_rect(row).contains(in_row))
    {
        d_->toggle_expanded(row);
        return;
    }
    if (!d_->model_->enabled(info.index))
    {
        return;
    }
    if (d_->model_->checkable(info.index) && d_->check_rect(row).adjusted(-2, -2, -2, -2).contains(in_row))
    {
        d_->model_->set_checked(info.index, !d_->model_->checked(info.index));
        d_->anchor_ = info.index;
        d_->change_current(info.index);
        return;
    }
    // 選んでいる current の項目を修飾キー無しでもう一度押したら、少し待ってから編集する(ダブルクリックなら編集しない)
    const int column = d_->column_at(in_row.x);
    if (e.clicks == 1 && !(e.modifiers & (MODIFIER_CONTROL | MODIFIER_SHIFT)) && info.index == d_->current_ &&
        is_selected(info.index) && column >= 0 && d_->model_->editable(info.index, column))
    {
        d_->pending_edit_ = info.index;
        d_->pending_column_ = column;
        d_->edit_timer_->enabled = false;  // 押し直したら待ち直す
        d_->edit_timer_->enabled = true;
    }
    else
    {
        d_->edit_timer_->enabled = false;
        d_->pending_edit_ = ModelIndex();
    }
    if (e.clicks == 2)
    {
        if (d_->tree_ && info.has_children)
        {
            d_->toggle_expanded(row);
        }
        d_->activate();
        return;
    }
    // ドラッグできるなら、押した行を覚える。選んでいる項目を修飾キー無しで押したら、複数の選択のままドラッグできるように、
    // 選択は離したときに変える(docs/adr/0047)
    d_->press_row_ = d_->drag_enabled_ ? row : -1;
    d_->press_pos_ = e.pos;
    d_->press_modifiers_ = e.modifiers;
    d_->press_deferred_ = d_->drag_enabled_ && is_selected(info.index) && !(e.modifiers & (MODIFIER_CONTROL | MODIFIER_SHIFT));
    if (!d_->press_deferred_)
    {
        d_->move_to(row, e.modifiers, true);
    }
}

void ItemView::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    if (d_->table_ && d_->selecting_ != 0)
    {
        d_->table_mouse_move(e);
        return;
    }
    // 押した行から少し動かしたら、選んでいる項目のドラッグを始める
    if (d_->press_row_ >= 0 && d_->model_ && std::hypot(e.pos.x - d_->press_pos_.x, e.pos.y - d_->press_pos_.y) > DRAG_THRESHOLD)
    {
        const int row = d_->press_row_;
        d_->press_row_ = -1;
        if (d_->press_deferred_)
        {
            d_->press_deferred_ = false;
        }
        d_->edit_timer_->enabled = false;
        d_->pending_edit_ = ModelIndex();
        const std::vector<ModelIndex> items = d_->get_selection();
        const ModelIndex pressed = row < d_->row_total() ? d_->row_at(row).index : ModelIndex();
        if (!items.empty() && is_selected(pressed))
        {
            DragData data;
            std::string label;
            for (std::size_t k = 0; k < items.size(); ++k)
            {
                const std::string t = d_->model_->text(items[k], 0);
                data.text += (k == 0 ? "" : "\n") + t;
                if (k == 0)
                {
                    label = t;
                }
            }
            if (items.size() > 1)
            {
                label += " (+" + std::to_string(items.size() - 1) + ")";
            }
            start_drag(data, label);
            return;
        }
    }
    if (d_->resizing_column_ >= 0)
    {
        d_->columns_[d_->resizing_column_]->width = d_->resize_width_ + (e.pos.x - d_->resize_origin_);
        return;
    }
    cursor = d_->header_divider_at(e.pos) >= 0 ? MouseCursor::SIZE_HORIZONTAL : MouseCursor::ARROW;
    const int row = d_->pressed_column_ < 0 ? d_->row_at_point(e.pos) : -1;
    if (row != d_->hover_row_)
    {
        d_->hover_row_ = row;
        invalidate();
    }
}

void ItemView::mouse_up(MouseEvent& e)
{
    Widget::mouse_up(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    d_->selecting_ = 0;
    // ドラッグしなかった: 押した選んでいる項目を、ふつうのクリックとして選ぶ
    if (d_->press_deferred_ && d_->press_row_ >= 0 && d_->press_row_ < d_->row_total())
    {
        d_->move_to(d_->press_row_, d_->press_modifiers_, true);
    }
    d_->press_row_ = -1;
    d_->press_deferred_ = false;
    if (d_->resizing_column_ >= 0)
    {
        e.accepted = true;
        d_->resizing_column_ = -1;
        return;
    }
    if (d_->pressed_column_ >= 0)
    {
        e.accepted = true;
        const int c = d_->pressed_column_;
        d_->pressed_column_ = -1;
        if (d_->sorting_enabled_ && d_->header_column_at(e.pos) == c)
        {
            // 同じ列なら向きを替え、別の列なら昇順で並べる。
            const SortOrder order = d_->sort_column_ == c && d_->sort_order_ == SortOrder::ASCENDING ? SortOrder::DESCENDING
                                                                                               : SortOrder::ASCENDING;
            const bool column_changed_now = d_->sort_column_ != c;
            const bool order_changed = d_->sort_order_ != order;
            d_->sort_column_ = c;
            d_->sort_order_ = order;
            d_->apply_sort();
            invalidate();
            if (column_changed_now)
            {
                notify_changed(sort_column);
            }
            if (order_changed)
            {
                notify_changed(sort_order);
            }
        }
    }
}

void ItemView::mouse_leave()
{
    if (d_->hover_row_ != -1)
    {
        d_->hover_row_ = -1;
        invalidate();
    }
    cursor = MouseCursor::ARROW;
    Widget::mouse_leave();
}

void ItemView::mouse_wheel(WheelEvent& e)
{
    Widget::mouse_wheel(e);
    if (e.accepted)
    {
        return;
    }
    const bool sideways = e.has_modifier(MODIFIER_SHIFT) || e.delta.x != 0;
    ScrollBar* bar = sideways ? d_->horizontal_ : d_->vertical_;
    if (bar->maximum.get() <= 0)
    {
        return;  // 動かせなければ親へ送る
    }
    e.accepted = true;
    const float ticks = sideways ? (e.delta.x != 0 ? e.delta.x : -e.delta.y) : -e.delta.y;
    const float step = sideways ? 48 : bar->single_step.get();
    bar->value = bar->value.get() + ticks * step;
}

namespace
{

/// 文字での検索の、続けて押したとみなす間隔(秒)。
const double SEARCH_TIMEOUT = 1.0;

double now_seconds()
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

/// ASCII の英字を小文字にした写し(それ以外の文字はそのまま比べる)。
std::string fold(const std::string& s)
{
    std::string out = s;
    for (std::size_t i = 0; i < out.size(); ++i)
    {
        if (out[i] >= 'A' && out[i] <= 'Z')
        {
            out[i] = static_cast<char>(out[i] - 'A' + 'a');
        }
    }
    return out;
}

}  // namespace

// ---- ドラッグ&ドロップ(docs/adr/0047) ----

void ItemView::Impl::set_drop_enabled(const bool& v)
{
    drop_enabled_ = v;
    q->accepts_drops = v;
}

ItemView::Impl::DropTarget ItemView::Impl::drop_target_at(const Point& p) const
{
    DropTarget t;
    t.place = DropPlace::NONE;
    t.row = -1;
    t.depth = 0;
    t.position = -1;
    const Rect view = viewport();
    if (!model_ || p.y < view.y || p.y >= view.bottom())
    {
        return t;
    }
    const float h = row_height();
    const float y = p.y - view.y + vertical_->value.get();
    const int row = static_cast<int>(std::floor(y / h));
    if (row >= row_total())
    {
        // 最後の行より下: 一番上の終わり
        t.place = DropPlace::END;
        t.row = row_total();
        return t;
    }
    if (row < 0)
    {
        return t;
    }
    const Row info = row_at(row);
    const float within = y - row * h;
    // 木では行の中ほどなら項目の子に、上下の端なら間に。一覧では上半分なら前、下半分なら後ろ
    const float edge = tree_ ? h / 4 : h / 2;
    if (tree_ && within >= edge && within < h - edge)
    {
        t.place = DropPlace::ONTO;
        t.row = row;
        t.depth = info.depth + 1;
        t.parent = info.index;
        return t;
    }
    t.place = DropPlace::BEFORE;
    if (within < edge)
    {
        // 項目の前
        t.row = row;
        t.depth = info.depth;
        t.parent = model_->parent_of(info.index);
        t.position = info.index.row();
    }
    else if (tree_ && row + 1 < row_total() && row_at(row + 1).depth > info.depth)
    {
        // 開いている項目の後ろ: 最初の子
        t.row = row + 1;
        t.depth = info.depth + 1;
        t.parent = info.index;
        t.position = 0;
    }
    else
    {
        // 項目の後ろ(同じ親の次)
        t.row = row + 1;
        t.depth = info.depth;
        t.parent = model_->parent_of(info.index);
        t.position = info.index.row() + 1;
    }
    return t;
}

bool ItemView::Impl::can_accept(const DragEvent& e, const DropTarget& target, std::vector<ModelIndex>& indexes) const
{
    indexes.clear();
    if (!model_ || target.place == DropPlace::NONE)
    {
        return false;
    }
    // 同じモデルを表示しているビューからの項目は、モデルの中で移す
    const ItemView* source = dynamic_cast<const ItemView*>(e.data.source());
    if (source && source->d_->model_ == model_)
    {
        indexes = source->d_->get_selection();
        return model_->can_move_rows(indexes, target.parent, target.position);
    }
    return model_->can_drop(e.data, target.parent, target.position);
}

void ItemView::drag_move(DragEvent& e)
{
    const Impl::DropTarget target = d_->drop_target_at(e.pos);
    std::vector<ModelIndex> indexes;
    const bool ok = d_->can_accept(e, target, indexes);
    if (ok && !indexes.empty())
    {
        e.action = DropAction::MOVE;  // 同じモデルの中は移すだけ
    }
    if (ok)
    {
        e.accepted = true;
    }
    else
    {
        // モデルが受け取らないものは、ハンドラ(on_drag_move・on_drop)に任せる(位置は index_at で引ける)
        Widget::drag_move(e);
    }
    const Impl::DropTarget shown = ok ? target : Impl::DropTarget();
    if (shown.place != d_->drop_target_.place || shown.row != d_->drop_target_.row || shown.depth != d_->drop_target_.depth)
    {
        d_->drop_target_ = shown;
        if (!ok)
        {
            d_->drop_target_.place = Impl::DropPlace::NONE;
        }
        invalidate();
    }
    // 端に近づけたらスクロールする
    const Rect view = d_->viewport();
    const float h = d_->row_height();
    if (e.pos.y < view.y + h / 2)
    {
        d_->vertical_->value = d_->vertical_->value.get() - h / 2;
    }
    else if (e.pos.y > view.bottom() - h / 2)
    {
        d_->vertical_->value = d_->vertical_->value.get() + h / 2;
    }
}

void ItemView::drag_leave()
{
    if (d_->drop_target_.place != Impl::DropPlace::NONE)
    {
        d_->drop_target_.place = Impl::DropPlace::NONE;
        invalidate();
    }
}

void ItemView::drop(DragEvent& e)
{
    const Impl::DropTarget target = d_->drop_target_at(e.pos);
    d_->drop_target_.place = Impl::DropPlace::NONE;
    invalidate();
    std::vector<ModelIndex> indexes;
    if (!d_->can_accept(e, target, indexes))
    {
        Widget::drop(e);
        return;
    }
    const ModelIndex parent = target.parent;
    if (!indexes.empty())
    {
        e.action = DropAction::MOVE;
        e.accepted = d_->model_->move_rows(indexes, target.parent, target.position);
    }
    else
    {
        e.accepted = d_->model_->drop_data(e.data, e.action, target.parent, target.position);
    }
    // 子にしたら開いて見せる
    if (e.accepted && d_->tree_ && parent.is_valid())
    {
        const ModelIndex now = d_->model_->locate(parent);
        d_->expand(now.is_valid() ? now : parent);
    }
}

void ItemView::Impl::paint_drop_indicator(Canvas& canvas, const Rect& view)
{
    if (drop_target_.place == DropPlace::NONE)
    {
        return;
    }
    const float h = row_height();
    const float y = view.y + drop_target_.row * h - vertical_->value.get();
    const Color c = theme::accent();
    if (drop_target_.place == DropPlace::ONTO)
    {
        canvas.stroke_round_rect(Rect(view.x + 1, y + 1, view.width - 2, h - 2), 3, 2, c);
        return;
    }
    // 間: 入れる深さの字下げから線を引く
    const float x = tree_ ? view.x + content_start(drop_target_.depth) - horizontal_->value.get() : view.x;
    canvas.fill_rect(Rect(x, y - 1, view.right() - x, 2), c);
}

// ---- セルの編集 ----

namespace
{

/// セルの編集の欄。Enter で確定・Esc でやめ、ほかへフォーカスが移ったら確定する。キーはビューへ送らない
/// (↑↓・文字のキーで current が動かないように)。
class CellEditor : public LineEdit
{
public:
    CellEditor(ItemView* view, bool table) : LineEdit(view), view_(view), table_(table) {}

protected:
    void key_down(KeyEvent& e) override
    {
        // 表: Enter・Tab で確定し、ビューへ送って次のセルへ動く
        if (table_ && (e.key == Key::RETURN || e.key == Key::TAB))
        {
            e.accepted = true;
            view_->finish_editing(true);
            KeyEvent forwarded(e.key, e.modifiers, e.repeat, e.native_key);
            detail::WidgetAccess::key_down(view_, forwarded);
            return;
        }
        if (e.key == Key::ESCAPE || e.key == Key::RETURN)
        {
            e.accepted = true;
            view_->finish_editing(e.key == Key::RETURN);
            return;
        }
        LineEdit::key_down(e);
        if (e.key != Key::TAB)
        {
            e.accepted = true;
        }
    }

    void focus_out() override
    {
        LineEdit::focus_out();
        view_->finish_editing(true);
    }

private:
    ItemView* view_;
    bool table_;
};

}  // namespace

int ItemView::Impl::column_at(float x) const
{
    if (columns_.empty())
    {
        return 0;
    }
    for (int c = 0; c < column_count(); ++c)
    {
        const float left = column_x(c);
        if (x >= left && x < left + column_draw_width(c))
        {
            return c;
        }
    }
    return -1;
}

int ItemView::Impl::first_editable_column(const ModelIndex& index) const
{
    if (!model_)
    {
        return -1;
    }
    for (int c = 0; c < column_count(); ++c)
    {
        if (model_->editable(index, c))
        {
            return c;
        }
    }
    return -1;
}

Rect ItemView::Impl::cell_text_rect(int row, int column) const
{
    const Rect view = viewport();
    const Row info = row_at(row);
    const float h = row_height();
    const float y = view.y + row * h - vertical_->value.get();
    const float cx = view.x - horizontal_->value.get() + column_x(column);
    const float cw = column_draw_width(column);
    float tx = cx + CELL_PADDING;
    if (column == 0)
    {
        // 木の印・チェックボックス・画像の後ろ(paint_row と同じ)
        tx = cx + content_start(info.depth);
        if (model_->checkable(info.index))
        {
            tx += ICON_SIZE + GAP;
        }
        if (!model_->icon(info.index).is_null())
        {
            tx += ICON_SIZE + GAP;
        }
    }
    // 編集の欄の内側の余白と枠の分だけ左に広げ、文字の位置を描いた文字にそろえる。見えている範囲に収める
    const float left = std::max(view.x, tx - theme::field_padding() - 1);
    const float right = std::min(view.right(), cx + cw);
    return Rect(left, y, std::max(right - left, MIN_EDITOR_WIDTH), h);
}

void ItemView::edit(const ModelIndex& index, int column)
{
    finish_editing(true);
    if (!d_->model_ || !index.is_valid() || column < 0 || column >= d_->column_count() || !d_->model_->editable(index, column) ||
        !d_->model_->enabled(index))
    {
        return;
    }
    scroll_to(index);
    const int row = d_->row_of(index);
    if (row < 0)
    {
        return;
    }
    // 列が見えるように横にスクロールする
    if (!d_->columns_.empty())
    {
        const float left = d_->column_x(column);
        const float scroll = d_->horizontal_->value;
        const float view_w = d_->viewport().width;
        if (left < scroll || left + std::min(d_->column_draw_width(column), view_w) > scroll + view_w)
        {
            d_->horizontal_->value = left;
        }
    }
    if (!d_->editor_)
    {
        d_->editor_ = new CellEditor(this, d_->table_);
    }
    d_->edit_index_ = index;
    d_->edit_column_ = column;
    d_->editor_->font = d_->model_->font(index, column);
    d_->editor_->text = d_->model_->text(index, column);
    d_->editor_->bounds = d_->cell_text_rect(row, column);
    d_->editor_->visible = true;
    d_->editor_->focused = true;
    d_->editor_->select_all();
}

bool ItemView::is_editing() const { return d_->edit_index_.is_valid(); }

void ItemView::finish_editing(bool commit)
{
    if (!d_->edit_index_.is_valid() || !d_->editor_)
    {
        return;
    }
    // 確定・やめるの途中で、フォーカスの移動から呼ばれても 1 回だけにする
    const ModelIndex index = d_->edit_index_;
    const int column = d_->edit_column_;
    d_->edit_index_ = ModelIndex();
    const bool had_focus = d_->editor_->focused;
    const std::string text = d_->editor_->text;
    d_->editor_->visible = false;
    if (had_focus)
    {
        focused = true;
    }
    if (commit && d_->model_ && text != d_->model_->text(index, column))
    {
        d_->model_->set_text(index, column, text);
    }
    invalidate();
}

bool ItemView::Impl::search_key(const KeyEvent& e)
{
    if (e.modifiers & (MODIFIER_CONTROL | MODIFIER_ALT | MODIFIER_META))
    {
        return false;
    }
    // 英字・数字・記号のキー(SDL のキーコードは、その文字の ASCII の値)
    const std::uint32_t code = e.native_key;
    const double now = now_seconds();
    const bool continuing = !search_text_.empty() && now - search_time_ < SEARCH_TIMEOUT;
    // 空白は、検索を続けているときだけ文字にする(それ以外は選ぶ・チェックする)
    if (code < 0x20 || code >= 0x7F || (code == 0x20 && !continuing))
    {
        return false;
    }
    search_text_ = (continuing ? search_text_ : std::string()) + fold(std::string(1, static_cast<char>(code)));
    search_time_ = now;
    const int n = row_total();
    if (n == 0)
    {
        return true;
    }
    // 同じ文字を続けて押したら、その文字で始まる次の項目へ。そうでなければ、今の項目から前方一致を探す
    const bool repeated = search_text_.find_first_not_of(search_text_[0]) == std::string::npos;
    const std::string needle = repeated ? search_text_.substr(0, 1) : search_text_;
    const int cur = row_of(current_);
    const int start = cur < 0 ? 0 : repeated ? cur + 1 : cur;
    for (int k = 0; k < n; ++k)
    {
        const int r = (start + k) % n;
        if (!row_enabled(r))
        {
            continue;
        }
        const std::string text = fold(model_->text(row_at(r).index, 0));
        if (text.compare(0, needle.size(), needle) == 0)
        {
            move_to(r, MODIFIER_NONE, false);
            break;
        }
    }
    return true;
}

void ItemView::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    if (!d_->model_)
    {
        return;
    }
    if (d_->table_)
    {
        d_->table_key_down(e);
        return;
    }
    // F2 で、current の項目の編集できる最初の列を編集する(docs/adr/0046)
    if (e.key == Key::F2 && !e.modifiers && d_->current_.is_valid())
    {
        const int column = d_->first_editable_column(d_->current_);
        if (column >= 0)
        {
            e.accepted = true;
            edit(d_->current_, column);
            return;
        }
    }
    // 押した文字で始まる項目へ動く(docs/adr/0046)
    if (d_->search_key(e))
    {
        e.accepted = true;
        return;
    }
    const int n = d_->row_total();
    const int cur = d_->row_of(d_->current_);
    const int page = std::max(1, static_cast<int>(d_->viewport().height / d_->row_height()) - 1);
    const bool ctrl = e.has_modifier(MODIFIER_CONTROL);
    int target = -2;  // -2 なら動かない
    switch (e.key)
    {
    case Key::UP:
        target = cur < 0 ? d_->next_enabled_row(0, 1) : d_->next_enabled_row(cur - 1, -1);
        break;
    case Key::DOWN:
        target = cur < 0 ? d_->next_enabled_row(0, 1) : d_->next_enabled_row(cur + 1, 1);
        break;
    case Key::PAGE_UP:
        target = d_->next_enabled_row(std::max(0, cur - page), 1);
        break;
    case Key::PAGE_DOWN:
        target = d_->next_enabled_row(std::min(n - 1, cur + page), -1);
        break;
    case Key::HOME:
        target = d_->next_enabled_row(0, 1);
        break;
    case Key::END:
        target = d_->next_enabled_row(n - 1, -1);
        break;
    case Key::LEFT:
    case Key::RIGHT:
        if (!d_->tree_ || cur < 0)
        {
            return;
        }
        e.accepted = true;
        {
            const Impl::Row info = d_->row_at(cur);
            const bool open = d_->is_expanded(info.index);
            if (e.key == Key::RIGHT)
            {
                if (info.has_children && !open)
                {
                    d_->expand(info.index);
                }
                else if (info.has_children)
                {
                    target = d_->next_enabled_row(cur + 1, 1);  // 最初の子へ
                }
            }
            else if (open)
            {
                d_->collapse(info.index);
            }
            else
            {
                target = d_->row_of(d_->model_->parent_of(info.index));  // 親へ
                if (target < 0)
                {
                    target = -2;
                }
            }
        }
        break;
    case Key::SPACE:
        if (cur < 0)
        {
            return;
        }
        e.accepted = true;
        if (ctrl && d_->selection_mode_ == SelectionMode::MULTI)
        {
            d_->move_to(cur, MODIFIER_CONTROL, true);
        }
        else if (d_->model_->checkable(d_->current_) && d_->model_->enabled(d_->current_))
        {
            d_->model_->set_checked(d_->current_, !d_->model_->checked(d_->current_));
        }
        else if (d_->selection_mode_ != SelectionMode::NONE)
        {
            select(d_->current_);
        }
        return;
    case Key::RETURN:
        e.accepted = true;
        d_->activate();
        return;
    case Key::A:
        if (ctrl && d_->selection_mode_ == SelectionMode::MULTI)
        {
            e.accepted = true;
            select_all();
        }
        return;
    default:
        return;
    }
    e.accepted = true;
    if (target >= 0)
    {
        d_->move_to(target, e.modifiers, false);
    }
}

void ItemView::text_input(TextInputEvent& e)
{
    // 表: 文字を打つと、current のセルを編集し始める(その文字から)
    if (d_->table_ && !is_editing() && d_->model_ && d_->current_.is_valid())
    {
        d_->start_typing_edit();
        if (is_editing())
        {
            detail::WidgetAccess::text_input(d_->editor_, e);
            return;
        }
    }
    Widget::text_input(e);
}

void ItemView::text_editing(TextEditingEvent& e)
{
    // 表: IME で変換を始めたら、current のセルを編集し始め、変換中の文字を編集の欄に渡す
    if (d_->table_ && !is_editing() && d_->model_ && d_->current_.is_valid() && !e.text.empty())
    {
        d_->start_typing_edit();
        if (is_editing())
        {
            detail::WidgetAccess::text_editing(d_->editor_, e);
            return;
        }
    }
    Widget::text_editing(e);
}

void ItemView::focus_in()
{
    invalidate();
    Widget::focus_in();
}

void ItemView::focus_out()
{
    invalidate();
    Widget::focus_out();
}

void ItemView::scale_changed()
{
    d_->layouts_.clear();
    for (std::size_t i = 0; i < d_->columns_.size(); ++i)
    {
        d_->columns_[i]->d_->layout_ = TextLayout();
    }
}

void ItemView::theme_changed()
{
    Widget::theme_changed();
    invalidate();
}

}  // namespace tokotachi
