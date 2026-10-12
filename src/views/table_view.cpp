// 表(TableView)と、ItemView の表のための部分(docs/adr/0050)。
#include <tokotachi/views/table_view.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/core/clipboard.hpp>
#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/widgets/line_edit.hpp>
#include <tokotachi/widgets/scroll_bar.hpp>

#include "column_impl.hpp"
#include "item_view_impl.hpp"
#include "../core/impl.hpp"
#include "../widgets/theme.hpp"

namespace tokotachi
{

namespace
{
const float BORDER = 1;             // 枠の太さ(item_view.cpp と同じ)
const float HEADER_PADDING = 8;     // 行の見出しの番号の左右の余白
const float MIN_ROW_HEADER = 32;    // 行の見出しの最小の幅

// セルの文字を、写す文字の中で区切りと紛れないようにする(タブ・改行は空白に)。
std::string flatten(const std::string& s)
{
    std::string out = s;
    for (std::size_t i = 0; i < out.size(); ++i)
    {
        if (out[i] == '\t' || out[i] == '\n' || out[i] == '\r')
        {
            out[i] = ' ';
        }
    }
    return out;
}

// 写した文字(行は改行、列はタブ)を表に分ける。最後の空の行は除く。
std::vector<std::vector<std::string> > split_table(const std::string& text)
{
    std::vector<std::vector<std::string> > rows;
    std::string line;
    std::vector<std::string> lines;
    for (std::size_t i = 0; i <= text.size(); ++i)
    {
        if (i == text.size() || text[i] == '\n')
        {
            if (!line.empty() && line[line.size() - 1] == '\r')
            {
                line.erase(line.size() - 1);
            }
            lines.push_back(line);
            line.clear();
        }
        else
        {
            line += text[i];
        }
    }
    while (!lines.empty() && lines.back().empty())
    {
        lines.pop_back();
    }
    for (std::size_t k = 0; k < lines.size(); ++k)
    {
        std::vector<std::string> cells;
        std::string cell;
        for (std::size_t i = 0; i <= lines[k].size(); ++i)
        {
            if (i == lines[k].size() || lines[k][i] == '\t')
            {
                cells.push_back(cell);
                cell.clear();
            }
            else
            {
                cell += lines[k][i];
            }
        }
        rows.push_back(cells);
    }
    return rows;
}

}  // namespace

CellRange::CellRange(int row_a, int column_a, int row_b, int column_b)
    : first_row(std::min(row_a, row_b)),
      first_column(std::min(column_a, column_b)),
      last_row(std::max(row_a, row_b)),
      last_column(std::max(column_a, column_b))
{
}

// ---- TableView ----

class TableView::Impl
{
    TOKOTACHI_IMPL(TableView);

    explicit Impl(TableView* object) : q(object) {}

    SelectionBehavior get_selection_behavior() const { return q->ItemView::get_selection_behavior(); }
    void set_selection_behavior(const SelectionBehavior& v) { q->ItemView::set_selection_behavior(v); }
    int get_current_column() const { return q->ItemView::get_current_column(); }
    void set_current_column(const int& v) { q->ItemView::set_current_column(v); }
    std::vector<CellRange> get_selected_ranges() const { return q->ItemView::get_selected_ranges(); }
    bool get_grid_visible() const { return q->ItemView::get_grid_visible(); }
    void set_grid_visible(const bool& v) { q->ItemView::set_grid_visible(v); }
    bool get_row_header_visible() const { return q->ItemView::get_row_header_visible(); }
    void set_row_header_visible(const bool& v) { q->ItemView::set_row_header_visible(v); }
};

TableView::TableView(Widget* parent_widget)
    : ItemView(parent_widget, false, true),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, SelectionBehavior, selection_behavior),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, int, current_column),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::vector<CellRange>, selected_ranges),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, grid_visible),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, row_header_visible),
      d_(new Impl(this))
{
}

TableView::~TableView() { delete d_; }

void TableView::cells_changed(bool current_column_changed, bool ranges_changed)
{
    if (current_column_changed)
    {
        notify_changed(current_column);
    }
    if (ranges_changed)
    {
        notify_changed(selected_ranges);
    }
}

// ---- ItemView の表の部分 ----

void ItemView::cells_changed(bool, bool) {}

void ItemView::set_selection_behavior(const SelectionBehavior& v)
{
    if (v == d_->behavior_)
    {
        return;
    }
    d_->behavior_ = v;
    // 選び方が変わったら、current のセル(行)だけを選び直す
    const bool had_ranges = !d_->ranges_.empty();
    d_->ranges_.clear();
    if (had_ranges)
    {
        cells_changed(false, true);
    }
    d_->change_selection(std::set<ModelIndex>());
    const int r = d_->row_of(d_->current_);
    if (r >= 0)
    {
        d_->move_to_cell(r, d_->current_column_, false, false);
    }
    invalidate();
}

void ItemView::set_current_column(const int& v)
{
    const int r = d_->row_of(d_->current_);
    if (r >= 0)
    {
        d_->move_to_cell(r, v, false, false);
    }
    else
    {
        d_->change_current_column(v);
    }
}

void ItemView::Impl::change_current_column(int column)
{
    const int c = std::max(0, std::min(column, column_count() - 1));
    if (c == current_column_)
    {
        return;
    }
    current_column_ = c;
    q->invalidate();
    q->cells_changed(true, false);
}

bool ItemView::cell_selected(int row, int column) const
{
    if (d_->behavior_ == SelectionBehavior::ROWS)
    {
        return row >= 0 && row < d_->row_total() && d_->selected_.count(d_->row_at(row).index) != 0;
    }
    for (std::size_t k = 0; k < d_->ranges_.size(); ++k)
    {
        if (d_->ranges_[k].contains(row, column))
        {
            return true;
        }
    }
    return false;
}

void ItemView::Impl::set_ranges(const std::vector<CellRange>& next)
{
    if (next == ranges_)
    {
        return;
    }
    ranges_ = next;
    q->invalidate();
    q->notify_changed(q->selection);
    q->cells_changed(false, true);
    Object* self = q;
    detail::fire(on_selection_change_, self);
}

void ItemView::Impl::reset_ranges()
{
    // 行の数・並びが変わった: 行の番号の範囲は意味を失うので、current のセルだけにする(知らせは呼び出し元が出す)
    ranges_.clear();
    const int r = row_of(current_);
    if (r >= 0 && behavior_ == SelectionBehavior::CELLS && selection_mode_ != SelectionMode::NONE)
    {
        ranges_.push_back(CellRange(r, current_column_, r, current_column_));
    }
}

void ItemView::select_cells(const CellRange& range, bool add)
{
    const int n = d_->row_total(), m = d_->column_count();
    if (!d_->table_ || range.is_empty() || n == 0 || d_->selection_mode_ == SelectionMode::NONE)
    {
        return;
    }
    const CellRange r(std::max(0, range.first_row), std::max(0, range.first_column), std::min(n - 1, range.last_row),
                      std::min(m - 1, range.last_column));
    if (r.is_empty())
    {
        return;
    }
    if (d_->behavior_ == SelectionBehavior::ROWS || d_->selection_mode_ == SelectionMode::SINGLE)
    {
        d_->move_to_cell(r.first_row, r.first_column, false, false);
        if (d_->behavior_ == SelectionBehavior::ROWS && d_->selection_mode_ == SelectionMode::MULTI && r.last_row > r.first_row)
        {
            d_->move_to_cell(r.last_row, r.first_column, true, false);
        }
        return;
    }
    std::vector<CellRange> next = add ? d_->ranges_ : std::vector<CellRange>();
    next.push_back(r);
    if (!add || !d_->current_.is_valid())
    {
        d_->anchor_ = d_->row_at(r.first_row).index;
        d_->anchor_column_ = r.first_column;
        d_->change_current(d_->anchor_);
        d_->change_current_column(r.first_column);
    }
    d_->set_ranges(next);
}

void ItemView::Impl::reveal_column(int column)
{
    if (columns_.empty() || column < 0 || column >= column_count())
    {
        return;
    }
    const float left = column_x(column);
    const float right = left + columns_[column]->d_->width_;
    const float scroll = horizontal_->value;
    const float view_w = viewport().width;
    if (left < scroll)
    {
        horizontal_->value = left;
    }
    else if (right > scroll + view_w)
    {
        horizontal_->value = std::min(left, right - view_w);
    }
}

void ItemView::Impl::move_to_cell(int row, int column, bool shift, bool add)
{
    if (!model_ || row < 0 || row >= row_total())
    {
        return;
    }
    const int c = std::max(0, std::min(column, column_count() - 1));
    const ModelIndex index = row_at(row).index;
    reveal_row(row);
    reveal_column(c);
    if (behavior_ == SelectionBehavior::ROWS)
    {
        // 行を選ぶ: 一覧と同じ選び方で、列だけ覚える
        Modifiers m = MODIFIER_NONE;
        if (shift)
        {
            m = static_cast<Modifiers>(m | MODIFIER_SHIFT);
        }
        if (add)
        {
            m = static_cast<Modifiers>(m | MODIFIER_CONTROL);
        }
        move_to(row, m, add);
        change_current_column(c);
        return;
    }
    const bool multi = selection_mode_ == SelectionMode::MULTI;
    if (selection_mode_ == SelectionMode::NONE)
    {
        anchor_ = index;
        anchor_column_ = c;
        change_current(index);
        change_current_column(c);
        return;
    }
    std::vector<CellRange> next;
    if (multi && shift)
    {
        // 起点のセルからの矩形に広げる(最後の範囲を置き換える)
        int from = row_of(anchor_);
        if (from < 0)
        {
            from = row;
            anchor_ = index;
            anchor_column_ = c;
        }
        next = ranges_;
        const CellRange extended(from, anchor_column_, row, c);
        if (next.empty())
        {
            next.push_back(extended);
        }
        else
        {
            next.back() = extended;
        }
    }
    else
    {
        anchor_ = index;
        anchor_column_ = c;
        if (multi && add)
        {
            next = ranges_;
        }
        next.push_back(CellRange(row, c, row, c));
    }
    change_current(index);
    change_current_column(c);
    set_ranges(next);
}

// ---- 寸法と描画 ----

float ItemView::Impl::row_header_width() const
{
    if (!table_ || !row_header_visible_)
    {
        return 0;
    }
    // 最も大きい番号の桁数の「0」の幅
    int digits = 1;
    for (int n = std::max(1, row_total()); n >= 10; n /= 10)
    {
        ++digits;
    }
    const float w = TextLayout(std::string(static_cast<std::size_t>(digits), '0'), Font(), q->display_scale).width();
    return std::max(MIN_ROW_HEADER, std::ceil(w) + HEADER_PADDING * 2);
}

void ItemView::set_grid_visible(const bool& v)
{
    d_->grid_visible_ = v;
    invalidate();
}

void ItemView::set_row_header_visible(const bool& v)
{
    d_->row_header_visible_ = v;
    request_layout();
    d_->column_changed();
}

void ItemView::Impl::paint_grid(Canvas& canvas, const Rect& view)
{
    if (!grid_visible_ || !model_)
    {
        return;
    }
    const float h = row_height();
    const float scroll = vertical_->value;
    const float x0 = view.x - horizontal_->value.get();
    const float bottom = std::min(view.bottom(), view.y + row_total() * h - scroll);
    float right = x0;
    for (int c = 0; c < column_count(); ++c)
    {
        right = x0 + column_x(c) + column_draw_width(c);
        if (c + 1 < column_count() && right >= view.x && right <= view.right())
        {
            canvas.fill_rect(Rect(right - 1, view.y, 1, std::max(0.0f, bottom - view.y)), theme::border());
        }
    }
    right = std::min(right, view.right());
    const int first = std::max(0, static_cast<int>(std::floor(scroll / h)));
    const int last = std::min(row_total() - 1, static_cast<int>(std::floor((scroll + view.height) / h)));
    for (int r = first; r <= last; ++r)
    {
        const float y = view.y + (r + 1) * h - scroll - 1;
        canvas.fill_rect(Rect(view.x, y, std::max(0.0f, right - view.x), 1), theme::border());
    }
}

void ItemView::Impl::paint_row_header(Canvas& canvas, const Rect& view)
{
    const float rh = row_header_width();
    const Rect area(BORDER, view.y, rh, view.height);
    canvas.save();
    canvas.clip_rect(area);
    canvas.fill_rect(area, theme::window());
    if (model_)
    {
        const float h = row_height();
        const float scroll = vertical_->value;
        const int first = std::max(0, static_cast<int>(std::floor(scroll / h)));
        const int last = std::min(row_total() - 1, static_cast<int>(std::floor((scroll + view.height) / h)));
        const bool enabled_now = q->effectively_enabled;
        const int current_row = row_of(current_);
        for (int r = first; r <= last; ++r)
        {
            const float y = view.y + r * h - scroll;
            // 選んでいるセルのある行は、見出しも色を付ける
            bool any = false;
            for (int c = 0; c < column_count() && !any; ++c)
            {
                any = q->cell_selected(r, c);
            }
            if (any)
            {
                canvas.fill_rect(Rect(area.x, y, rh, h), theme::selection_inactive());
            }
            const TextLayout& l = text_layout(std::to_string(r + 1));
            const Color fg = !enabled_now ? theme::text_disabled()
                             : r == current_row ? theme::text()
                                                : theme::color(ColorRole::TEXT_SECONDARY);
            canvas.draw_text(l, Point(area.x + rh - HEADER_PADDING - l.width(), y + (h - l.height()) / 2), fg);
            if (grid_visible_)
            {
                canvas.fill_rect(Rect(area.x, y + h - 1, rh, 1), theme::border());
            }
        }
    }
    canvas.restore();
    // 見出しと中身の境目(左上の隅から下まで)
    canvas.fill_rect(Rect(BORDER + rh - 1, BORDER, 1, view.bottom() - BORDER), theme::border());
}

// ---- 入力 ----

bool ItemView::Impl::cell_at_point(const Point& p, int& row, int& column) const
{
    const Rect view = viewport();
    if (!model_ || p.y < view.y || p.y >= view.bottom() || p.x < BORDER || p.x >= view.right())
    {
        return false;
    }
    row = static_cast<int>(std::floor((p.y - view.y + vertical_->value.get()) / row_height()));
    if (row < 0 || row >= row_total())
    {
        return false;
    }
    if (p.x < view.x)
    {
        column = -1;  // 行の見出し
        return true;
    }
    column = column_at(p.x - view.x + horizontal_->value.get());
    return column >= 0;
}

void ItemView::Impl::table_mouse_down(MouseEvent& e)
{
    const Rect view = viewport();
    // 左上の隅: すべてを選ぶ
    if (header_height() > 0 && e.pos.x < view.x && e.pos.y < view.y)
    {
        q->select_all();
        return;
    }
    int row = -1, column = -1;
    if (!cell_at_point(e.pos, row, column))
    {
        if (view.contains(e.pos) && !(e.modifiers & (MODIFIER_CONTROL | MODIFIER_SHIFT)))
        {
            q->clear_selection();
        }
        return;
    }
    const ModelIndex index = row_at(row).index;
    if (!model_->enabled(index))
    {
        return;
    }
    const bool shift = (e.modifiers & MODIFIER_SHIFT) != 0;
    const bool add = (e.modifiers & MODIFIER_CONTROL) != 0;
    if (column < 0)
    {
        // 行の見出し: 行のすべてのセル(ドラッグで行を広げる)
        if (behavior_ == SelectionBehavior::ROWS)
        {
            move_to_cell(row, current_column_, shift, add);
        }
        else if (selection_mode_ == SelectionMode::MULTI)
        {
            if (!shift)
            {
                anchor_ = index;
                anchor_column_ = 0;
            }
            int from = row_of(anchor_);
            from = from < 0 ? row : from;
            std::vector<CellRange> next = shift || add ? ranges_ : std::vector<CellRange>();
            const CellRange whole(from, 0, row, column_count() - 1);
            if (shift && !next.empty())
            {
                next.back() = whole;
            }
            else
            {
                next.push_back(whole);
            }
            change_current(index);
            set_ranges(next);
        }
        else
        {
            move_to_cell(row, current_column_, false, false);
        }
        selecting_ = 2;
        return;
    }
    // 1 列目のチェックボックス
    if (column == 0 && model_->checkable(index))
    {
        const float h = row_height();
        const Point in_row(e.pos.x - view.x + horizontal_->value.get(), e.pos.y - view.y + vertical_->value.get() - row * h);
        if (check_rect(row).adjusted(-2, -2, -2, -2).contains(in_row))
        {
            model_->set_checked(index, !model_->checked(index));
            move_to_cell(row, 0, false, false);
            return;
        }
    }
    if (e.clicks == 2)
    {
        // ダブルクリック: 編集できるセルなら今の文字の続きから編集し、できなければ on_activate
        if (model_->editable(index, column))
        {
            q->edit(index, column);
            if (q->is_editing())
            {
                editor_->cursor_position = editor_->text.get().size();
            }
        }
        else
        {
            activate();
        }
        return;
    }
    move_to_cell(row, column, shift, add);
    selecting_ = 1;
}

void ItemView::Impl::table_mouse_move(MouseEvent& e)
{
    // 範囲を選んでいる: ポインタの下のセルまで広げる(ビューの外なら端のセル。少しずつスクロールする)
    const Rect view = viewport();
    if (!model_ || row_total() == 0 || view.is_empty())
    {
        return;
    }
    const float h = row_height();
    if (e.pos.y < view.y)
    {
        vertical_->value = vertical_->value.get() - h / 2;
    }
    else if (e.pos.y >= view.bottom())
    {
        vertical_->value = vertical_->value.get() + h / 2;
    }
    const Point p(std::max(view.x, std::min(e.pos.x, view.right() - 1)), std::max(view.y, std::min(e.pos.y, view.bottom() - 1)));
    int row = static_cast<int>(std::floor((p.y - view.y + vertical_->value.get()) / h));
    row = std::max(0, std::min(row, row_total() - 1));
    int column = column_at(p.x - view.x + horizontal_->value.get());
    column = column < 0 ? column_count() - 1 : column;
    if (selection_mode_ != SelectionMode::MULTI)
    {
        move_to_cell(row, column, false, false);
        return;
    }
    if (selecting_ == 2 && behavior_ == SelectionBehavior::CELLS)
    {
        int from = row_of(anchor_);
        from = from < 0 ? row : from;
        std::vector<CellRange> next = ranges_;
        const CellRange whole(from, 0, row, column_count() - 1);
        if (next.empty())
        {
            next.push_back(whole);
        }
        else
        {
            next.back() = whole;
        }
        change_current(row_at(row).index);
        reveal_row(row);
        set_ranges(next);
        return;
    }
    move_to_cell(row, selecting_ == 2 ? current_column_ : column, true, false);
}

void ItemView::Impl::start_typing_edit()
{
    if (!model_ || !current_.is_valid() || !model_->editable(current_, current_column_) || !model_->enabled(current_))
    {
        return;
    }
    q->edit(current_, current_column_);
    if (q->is_editing())
    {
        editor_->text = "";  // 上書き: 打った文字から始める
    }
}

std::string ItemView::selected_cells_text() const
{
    if (!d_->model_)
    {
        return std::string();
    }
    std::string out;
    if (d_->behavior_ == SelectionBehavior::ROWS)
    {
        // 選んでいる行のすべての列
        const std::vector<ModelIndex> rows = d_->get_selection();
        for (std::size_t k = 0; k < rows.size(); ++k)
        {
            for (int c = 0; c < d_->column_count(); ++c)
            {
                out += (c == 0 ? "" : "\t") + flatten(d_->model_->text(rows[k], c));
            }
            out += k + 1 < rows.size() ? "\n" : "";
        }
        return out;
    }
    if (d_->ranges_.empty())
    {
        return std::string();
    }
    // 範囲すべてを囲む矩形(選んでいないセルは空)
    CellRange box = d_->ranges_[0];
    for (std::size_t k = 1; k < d_->ranges_.size(); ++k)
    {
        box = CellRange(std::min(box.first_row, d_->ranges_[k].first_row), std::min(box.first_column, d_->ranges_[k].first_column),
                        std::max(box.last_row, d_->ranges_[k].last_row), std::max(box.last_column, d_->ranges_[k].last_column));
    }
    const int n = d_->row_total();
    for (int r = box.first_row; r <= box.last_row && r < n; ++r)
    {
        const ModelIndex index = d_->row_at(r).index;
        for (int c = box.first_column; c <= box.last_column; ++c)
        {
            out += (c == box.first_column ? "" : "\t") + (cell_selected(r, c) ? flatten(d_->model_->text(index, c)) : std::string());
        }
        out += r < box.last_row ? "\n" : "";
    }
    return out;
}

void ItemView::copy_cells()
{
    const std::string text = selected_cells_text();
    if (!text.empty() || !d_->ranges_.empty())
    {
        clipboard::set_text(text);
    }
}

void ItemView::clear_cells()
{
    if (!d_->model_)
    {
        return;
    }
    const int n = d_->row_total();
    for (int r = 0; r < n; ++r)
    {
        const ModelIndex index = d_->row_at(r).index;
        for (int c = 0; c < d_->column_count(); ++c)
        {
            if (cell_selected(r, c) && d_->model_->enabled(index) && d_->model_->editable(index, c) && !d_->model_->text(index, c).empty())
            {
                d_->model_->set_text(index, c, std::string());
            }
        }
    }
}

void ItemView::paste_cells()
{
    const int start_row = d_->row_of(d_->current_);
    if (!d_->model_ || start_row < 0 || !clipboard::has_text())
    {
        return;
    }
    const std::vector<std::vector<std::string> > table = split_table(clipboard::text());
    if (table.empty())
    {
        return;
    }
    const int n = d_->row_total(), m = d_->column_count();
    const auto put = [&](int r, int c, const std::string& value)
    {
        const ModelIndex index = d_->row_at(r).index;
        if (d_->model_->enabled(index) && d_->model_->editable(index, c) && d_->model_->text(index, c) != value)
        {
            d_->model_->set_text(index, c, value);
        }
    };
    // 1 つの値: 選んでいるすべてのセルへ(何も選んでいなければ current のセル)
    if (table.size() == 1 && table[0].size() == 1)
    {
        bool any = false;
        for (int r = 0; r < n; ++r)
        {
            for (int c = 0; c < m; ++c)
            {
                if (cell_selected(r, c))
                {
                    put(r, c, table[0][0]);
                    any = true;
                }
            }
        }
        if (!any)
        {
            put(start_row, d_->current_column_, table[0][0]);
        }
        return;
    }
    // 表: current のセル(範囲を選んでいれば、その左上)から貼り、貼った範囲を選ぶ
    int top = start_row, left = d_->current_column_;
    if (d_->behavior_ == SelectionBehavior::CELLS && !d_->ranges_.empty())
    {
        top = d_->ranges_.back().first_row;
        left = d_->ranges_.back().first_column;
    }
    int bottom = top, right = left;
    for (std::size_t i = 0; i < table.size() && top + static_cast<int>(i) < n; ++i)
    {
        const int r = top + static_cast<int>(i);
        for (std::size_t j = 0; j < table[i].size() && left + static_cast<int>(j) < m; ++j)
        {
            const int c = left + static_cast<int>(j);
            put(r, c, table[i][j]);
            bottom = std::max(bottom, r);
            right = std::max(right, c);
        }
    }
    if (d_->behavior_ == SelectionBehavior::CELLS && d_->selection_mode_ == SelectionMode::MULTI)
    {
        select_cells(CellRange(top, left, bottom, right), false);
    }
}

void ItemView::Impl::table_key_down(KeyEvent& e)
{
    const int n = row_total(), m = column_count();
    const int cur = row_of(current_);
    const int cc = std::min(current_column_, m - 1);
    const bool shift = e.has_modifier(MODIFIER_SHIFT);
    const bool ctrl = e.has_modifier(MODIFIER_CONTROL);
    const bool other = e.has_modifier(MODIFIER_ALT) || e.has_modifier(MODIFIER_META);
    if (other || n == 0)
    {
        return;
    }
    // クリップボード・すべてを選ぶ
    if (ctrl && !shift)
    {
        switch (e.key)
        {
        case Key::C:
            e.accepted = true;
            q->copy_cells();
            return;
        case Key::X:
            e.accepted = true;
            q->copy_cells();
            q->clear_cells();
            return;
        case Key::V:
            e.accepted = true;
            q->paste_cells();
            return;
        case Key::A:
            e.accepted = true;
            q->select_all();
            return;
        default:
            break;
        }
    }
    if (cur >= 0 && !ctrl && !shift)
    {
        switch (e.key)
        {
        case Key::F2:
            // 今の文字の続きから編集する
            e.accepted = true;
            q->edit(current_, cc);
            if (q->is_editing())
            {
                editor_->cursor_position = editor_->text.get().size();
            }
            return;
        case Key::DEL:
            e.accepted = true;
            q->clear_cells();
            return;
        case Key::BACKSPACE:
            // 空にして編集を始める
            e.accepted = true;
            start_typing_edit();
            return;
        case Key::SPACE:
            if (cc == 0 && model_->checkable(current_) && model_->enabled(current_))
            {
                e.accepted = true;
                model_->set_checked(current_, !model_->checked(current_));
            }
            return;
        default:
            break;
        }
    }
    if (cur < 0)
    {
        // まだ current が無い: 最初のセルへ
        if (e.key == Key::UP || e.key == Key::DOWN || e.key == Key::LEFT || e.key == Key::RIGHT || e.key == Key::HOME ||
            e.key == Key::END || e.key == Key::PAGE_UP || e.key == Key::PAGE_DOWN)
        {
            e.accepted = true;
            const int first = next_enabled_row(0, 1);
            if (first >= 0)
            {
                move_to_cell(first, 0, false, false);
            }
        }
        return;
    }
    const int page = std::max(1, static_cast<int>(viewport().height / row_height()) - 1);
    int row = cur, column = cc;
    bool extend = shift;
    switch (e.key)
    {
    case Key::UP:
        row = ctrl ? next_enabled_row(0, 1) : next_enabled_row(cur - 1, -1);
        break;
    case Key::DOWN:
        row = ctrl ? next_enabled_row(n - 1, -1) : next_enabled_row(cur + 1, 1);
        break;
    case Key::LEFT:
        column = ctrl ? 0 : std::max(0, cc - 1);
        break;
    case Key::RIGHT:
        column = ctrl ? m - 1 : std::min(m - 1, cc + 1);
        break;
    case Key::HOME:
        column = 0;
        row = ctrl ? next_enabled_row(0, 1) : cur;
        break;
    case Key::END:
        column = m - 1;
        row = ctrl ? next_enabled_row(n - 1, -1) : cur;
        break;
    case Key::PAGE_UP:
        row = next_enabled_row(std::max(0, cur - page), 1);
        break;
    case Key::PAGE_DOWN:
        row = next_enabled_row(std::min(n - 1, cur + page), -1);
        break;
    case Key::TAB:
        // 右・左へ(行をまたぐ)。表の端ならフォーカスをほかへ移す(受け取らない)
        if (ctrl)
        {
            return;
        }
        extend = false;
        if (!shift)
        {
            if (cc + 1 < m)
            {
                column = cc + 1;
            }
            else
            {
                row = next_enabled_row(cur + 1, 1);
                column = 0;
            }
        }
        else if (cc > 0)
        {
            column = cc - 1;
        }
        else
        {
            row = next_enabled_row(cur - 1, -1);
            column = m - 1;
        }
        if (row < 0)
        {
            return;
        }
        break;
    case Key::RETURN:
        // 下・上へ(端なら動かない)
        if (ctrl)
        {
            return;
        }
        extend = false;
        row = shift ? next_enabled_row(cur - 1, -1) : next_enabled_row(cur + 1, 1);
        if (row < 0)
        {
            row = cur;
        }
        break;
    default:
        return;
    }
    e.accepted = true;
    if (row >= 0)
    {
        move_to_cell(row, column, extend, false);
    }
}

}  // namespace tokotachi
