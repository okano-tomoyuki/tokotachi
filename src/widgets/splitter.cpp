#include <tokotachi/widgets/splitter.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

const float Splitter::HANDLE = 6;

namespace
{
const float GRAB_MARGIN = 2;  // 境目の外側でもつかめる幅(dp)
}  // namespace

// ---- SplitterPane ----

SplitterPane::SplitterPane(Splitter* splitter, Widget* pane_widget, int initial_stretch)
    : Object(splitter),
      TOKOTACHI_PROPERTY_MEMBER(SplitterPane, int, stretch),
      TOKOTACHI_PROPERTY_MEMBER(SplitterPane, bool, collapsible),
      TOKOTACHI_READONLY_PROPERTY(SplitterPane, bool, collapsed),
      TOKOTACHI_READONLY_PROPERTY(SplitterPane, Widget*, widget),
      splitter_(splitter),
      widget_(pane_widget),
      stretch_(std::max(0, initial_stretch)),
      collapsible_(true),
      size_(-1),
      collapsed_(false)
{
}

SplitterPane::~SplitterPane() {}

void SplitterPane::set_stretch(const int& v)
{
    stretch_ = std::max(0, v);
}

void SplitterPane::set_collapsible(const bool& v) { collapsible_ = v; }

// ---- Splitter ----

Splitter::Splitter(Widget* parent_widget, Orientation initial_orientation)
    : Widget(parent_widget),
      TOKOTACHI_PROPERTY_GETTER(Splitter, Orientation, orientation),
      TOKOTACHI_READONLY_PROPERTY(Splitter, std::vector<SplitterPane*>, panes),
      TOKOTACHI_PROPERTY_GETTER(Splitter, std::vector<float>, sizes),
      TOKOTACHI_HANDLER_PROPERTY(Splitter, NotifyHandler, on_change),
      orientation_(initial_orientation),
      hover_handle_(-1),
      drag_handle_(-1),
      drag_offset_(0),
      destroying_(false)
{
}

Splitter::~Splitter()
{
    // 区画の部品(子)は Widget のデストラクタが破棄する。区画はここで先に破棄する。
    destroying_ = true;
    for (std::size_t i = 0; i < panes_.size(); ++i)
    {
        delete panes_[i];
    }
    panes_.clear();
}

void Splitter::set_orientation(const Orientation& v)
{
    if (v == orientation_)
    {
        return;
    }
    orientation_ = v;
    for (std::size_t i = 0; i < panes_.size(); ++i)
    {
        panes_[i]->size_ = -1;  // 望む大きさから決め直す
    }
    request_layout();
    mark_layout_dirty();
    invalidate();
}

SplitterPane* Splitter::add_widget(Widget* w, int stretch_value)
{
    return insert_widget(static_cast<int>(panes_.size()), w, stretch_value);
}

SplitterPane* Splitter::insert_widget(int index, Widget* w, int stretch_value)
{
    if (!w || w == this)
    {
        return nullptr;
    }
    for (std::size_t i = 0; i < panes_.size(); ++i)
    {
        if (panes_[i]->widget_ == w)
        {
            return panes_[i];
        }
    }
    w->parent = this;
    if (w->parent.get() != this)
    {
        return nullptr;
    }
    const int n = static_cast<int>(panes_.size());
    const int at = index >= 0 && index <= n ? index : n;
    SplitterPane* pane = new SplitterPane(this, w, stretch_value);
    panes_.insert(panes_.begin() + at, pane);
    notify_changed(panes);
    request_layout();
    mark_layout_dirty();
    return pane;
}

void Splitter::child_removed(Widget* child)
{
    if (destroying_)
    {
        return;
    }
    for (std::size_t i = 0; i < panes_.size(); ++i)
    {
        if (panes_[i]->widget_ == child)
        {
            delete panes_[i];
            panes_.erase(panes_.begin() + static_cast<std::ptrdiff_t>(i));
            hover_handle_ = drag_handle_ = -1;
            notify_changed(panes);
            request_layout();
            mark_layout_dirty();
            return;
        }
    }
}

std::vector<float> Splitter::get_sizes() const
{
    std::vector<float> out;
    for (std::size_t i = 0; i < panes_.size(); ++i)
    {
        out.push_back(panes_[i]->collapsed_ ? 0.0f : std::max(0.0f, panes_[i]->size_));
    }
    return out;
}

void Splitter::set_sizes(const std::vector<float>& v)
{
    for (std::size_t i = 0; i < panes_.size() && i < v.size(); ++i)
    {
        SplitterPane* p = panes_[i];
        const bool collapse = v[i] <= 0 && p->collapsible_;
        if (collapse != p->collapsed_)
        {
            p->collapsed_ = collapse;
            p->notify_changed(p->collapsed);
        }
        if (!collapse)
        {
            p->size_ = std::max(0.0f, v[i]);
        }
    }
    mark_layout_dirty();
    invalidate();
}

float Splitter::minimum_of(const SplitterPane* pane) const { return along(pane->widget_->minimum_hint.get()); }

/// ---- 大きさの配り方 ----

namespace
{
// 区画の、並ぶ向きの今の大きさ(たたんでいれば 0)。
float extent_of(float size, bool collapsed) { return collapsed ? 0.0f : std::max(0.0f, size); }
}  // namespace

void Splitter::fit(float available)
{
    const int n = static_cast<int>(panes_.size());
    if (n == 0)
    {
        return;
    }
    const float space = std::max(0.0f, available - HANDLE * static_cast<float>(n - 1));
    // まだ大きさの無い区画は、望む大きさから始める。
    for (int i = 0; i < n; ++i)
    {
        if (panes_[i]->size_ < 0)
        {
            panes_[i]->size_ = along(panes_[i]->widget_->preferred_size.get());
        }
    }
    float delta = space;
    for (int i = 0; i < n; ++i)
    {
        delta -= extent_of(panes_[i]->size_, panes_[i]->collapsed_);
    }
    // 余り・不足を stretch の比率で配る(縮めるときは最小の大きさで止め、残りを他へ回す)。たたんだ区画には配らない。
    for (int round = 0; round < n + 1 && std::fabs(delta) > 0.01f; ++round)
    {
        std::vector<SplitterPane*> takers;
        int total = 0;
        for (int i = 0; i < n; ++i)
        {
            SplitterPane* p = panes_[i];
            if (p->collapsed_ || p->stretch_ == 0 || (delta < 0 && p->size_ <= minimum_of(p)))
            {
                continue;
            }
            takers.push_back(p);
            total += p->stretch_;
        }
        if (takers.empty())
        {
            // 伸びる区画が無い: たたんでいない区画のうち、最後の(縮めるなら最小より大きい)ものが受け持つ。
            for (int i = n - 1; i >= 0; --i)
            {
                SplitterPane* p = panes_[i];
                if (!p->collapsed_ && (delta > 0 || p->size_ > minimum_of(p)))
                {
                    takers.push_back(p);
                    total = std::max(1, p->stretch_);
                    break;
                }
            }
        }
        if (takers.empty())
        {
            break;
        }
        float applied = 0;
        for (std::size_t k = 0; k < takers.size(); ++k)
        {
            SplitterPane* p = takers[k];
            const int weight = std::max(1, p->stretch_);
            float next = p->size_ + delta * static_cast<float>(weight) / static_cast<float>(std::max(1, total));
            if (delta < 0)
            {
                next = std::max(next, minimum_of(p));
            }
            applied += next - p->size_;
            p->size_ = next;
        }
        delta -= applied;
    }
    if (delta < -0.01f)
    {
        // どの区画も最小まで縮めた: 収まらない分は、大きさの比で最小より小さくする(はみ出しを切る)。
        float total = 0;
        for (int i = 0; i < n; ++i)
        {
            total += extent_of(panes_[i]->size_, panes_[i]->collapsed_);
        }
        for (int i = 0; i < n && total > 0; ++i)
        {
            if (!panes_[i]->collapsed_)
            {
                panes_[i]->size_ = std::max(0.0f, panes_[i]->size_ + delta * panes_[i]->size_ / total);
            }
        }
    }
    else if (delta > 0.01f)
    {
        // すべての区画をたたんでいる: 最後の区画を開いて受け持たせる。
        SplitterPane* last = panes_[static_cast<std::size_t>(n - 1)];
        if (last->collapsed_)
        {
            last->collapsed_ = false;
            last->size_ = 0;
            last->notify_changed(last->collapsed);
        }
        last->size_ += delta;
    }
}

void Splitter::arrange_children()
{
    const std::vector<float> before = get_sizes();
    const Rect r = local_rect;
    const bool horizontal = orientation_ == Orientation::HORIZONTAL;
    fit(horizontal ? r.width : r.height);
    float pos = 0;
    for (std::size_t i = 0; i < panes_.size(); ++i)
    {
        SplitterPane* p = panes_[i];
        const float s = extent_of(p->size_, p->collapsed_);
        p->widget_->visible = !p->collapsed_;  // たたんだ区画は隠す
        if (!p->collapsed_)
        {
            p->widget_->bounds = horizontal ? Rect(pos, 0, s, r.height) : Rect(0, pos, r.width, s);
        }
        pos += s + HANDLE;
    }
    invalidate();
    if (get_sizes() != before)
    {
        notify_changed(sizes);
    }
}

Size Splitter::compute_preferred_size() const
{
    float a = 0, cross = 0;
    for (std::size_t i = 0; i < panes_.size(); ++i)
    {
        const Size s = panes_[i]->widget_->preferred_size;
        if (!panes_[i]->collapsed_)
        {
            a += along(s);
        }
        cross = std::max(cross, orientation_ == Orientation::HORIZONTAL ? s.height : s.width);
    }
    a += HANDLE * static_cast<float>(panes_.empty() ? 0 : panes_.size() - 1);
    return orientation_ == Orientation::HORIZONTAL ? Size(a, cross) : Size(cross, a);
}

Size Splitter::compute_minimum_size() const
{
    float a = 0, cross = 0;
    for (std::size_t i = 0; i < panes_.size(); ++i)
    {
        const Size s = panes_[i]->widget_->minimum_hint;
        if (!panes_[i]->collapsed_)
        {
            a += along(s);
        }
        cross = std::max(cross, orientation_ == Orientation::HORIZONTAL ? s.height : s.width);
    }
    a += HANDLE * static_cast<float>(panes_.empty() ? 0 : panes_.size() - 1);
    return orientation_ == Orientation::HORIZONTAL ? Size(a, cross) : Size(cross, a);
}

// ---- 境目 ----

float Splitter::handle_position(int i) const
{
    float pos = 0;
    for (int k = 0; k <= i; ++k)
    {
        const SplitterPane* p = panes_[static_cast<std::size_t>(k)];
        pos += extent_of(p->size_, p->collapsed_);
    }
    return pos + HANDLE * static_cast<float>(i);
}

int Splitter::handle_at(const Point& p) const
{
    const float a = along(p);
    for (int i = 0; i + 1 < static_cast<int>(panes_.size()); ++i)
    {
        const float h = handle_position(i);
        if (a >= h - GRAB_MARGIN && a < h + HANDLE + GRAB_MARGIN)
        {
            return i;
        }
    }
    return -1;
}

void Splitter::drag_to(float position)
{
    SplitterPane* a = panes_[static_cast<std::size_t>(drag_handle_)];
    SplitterPane* b = panes_[static_cast<std::size_t>(drag_handle_ + 1)];
    const float ea = extent_of(a->size_, a->collapsed_), eb = extent_of(b->size_, b->collapsed_);
    const float start = handle_position(drag_handle_) - ea;
    const float combined = ea + eb;
    const float min_a = std::min(minimum_of(a), combined), min_b = std::min(minimum_of(b), combined);
    // 前の区画の大きさを決め、最小より小さければたたむ(たためなければ最小)。後ろの区画も同じ。
    float sa = std::max(0.0f, std::min(position - start, combined));
    bool collapse_a = false, collapse_b = false;
    if (sa < min_a)
    {
        collapse_a = a->collapsible_ && sa < min_a / 2;
        sa = collapse_a ? 0 : min_a;
    }
    float sb = combined - sa;
    if (sb < min_b)
    {
        collapse_b = b->collapsible_ && sb < min_b / 2;
        sb = collapse_b ? 0 : min_b;
        sa = std::max(0.0f, combined - sb);
        collapse_a = collapse_a && sa <= 0;
    }
    if (sa == ea && sb == eb && collapse_a == a->collapsed_ && collapse_b == b->collapsed_)
    {
        return;
    }
    const bool changed_a = collapse_a != a->collapsed_, changed_b = collapse_b != b->collapsed_;
    a->collapsed_ = collapse_a;
    b->collapsed_ = collapse_b;
    if (!collapse_a)
    {
        a->size_ = sa;
    }
    if (!collapse_b)
    {
        b->size_ = sb;
    }
    if (changed_a)
    {
        a->notify_changed(a->collapsed);
    }
    if (changed_b)
    {
        b->notify_changed(b->collapsed);
    }
    mark_layout_dirty();
    invalidate();
    notify_changed(sizes);
    Object* self = this;
    detail::fire(on_change_, self);
}

void Splitter::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool horizontal = orientation_ == Orientation::HORIZONTAL;
    for (int i = 0; i + 1 < static_cast<int>(panes_.size()); ++i)
    {
        const float h = handle_position(i);
        const Rect band = horizontal ? Rect(h, 0, HANDLE, r.height) : Rect(0, h, r.width, HANDLE);
        if (i == drag_handle_ || (drag_handle_ < 0 && i == hover_handle_))
        {
            canvas.fill_rect(band, theme::selection_inactive());
        }
        const Rect line = horizontal ? Rect(std::floor(h + HANDLE / 2), 0, 1, r.height)
                                     : Rect(0, std::floor(h + HANDLE / 2), r.width, 1);
        canvas.fill_rect(line, theme::border());
    }
    Widget::paint(canvas);
}

void Splitter::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    const int i = handle_at(e.pos);
    if (i < 0)
    {
        return;
    }
    e.accepted = true;
    drag_handle_ = i;
    drag_offset_ = along(e.pos) - handle_position(i);
    invalidate();
}

void Splitter::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    if (drag_handle_ >= 0)
    {
        drag_to(along(e.pos) - drag_offset_);
        return;
    }
    const int i = handle_at(e.pos);
    if (i != hover_handle_)
    {
        hover_handle_ = i;
        invalidate();
    }
    cursor = i < 0                                         ? MouseCursor::ARROW
             : orientation_ == Orientation::HORIZONTAL ? MouseCursor::SIZE_HORIZONTAL
                                                       : MouseCursor::SIZE_VERTICAL;
}

void Splitter::mouse_up(MouseEvent& e)
{
    Widget::mouse_up(e);
    if (e.button == MouseButton::LEFT && drag_handle_ >= 0)
    {
        e.accepted = true;
        drag_handle_ = -1;
        hover_handle_ = handle_at(e.pos);
        if (hover_handle_ < 0)
        {
            cursor = MouseCursor::ARROW;  // 境目の外で離した
        }
        invalidate();
    }
}

void Splitter::mouse_leave()
{
    if (hover_handle_ >= 0 && drag_handle_ < 0)
    {
        hover_handle_ = -1;
        invalidate();
    }
    if (drag_handle_ < 0)
    {
        cursor = MouseCursor::ARROW;
    }
    Widget::mouse_leave();
}

}  // namespace tokotachi
