#include <tokotachi/widgets/splitter.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/impl.hpp"
#include "theme.hpp"
#include "widget_access.hpp"

namespace tokotachi
{

const float Splitter::HANDLE = 6;

namespace
{
const float GRAB_MARGIN = 2;  // 境目の外側でもつかめる幅(dp)
const float KEY_STEP = 10;    // 矢印キーで境目を動かす幅(dp)
}  // namespace

// ---- SplitterPane ----

class SplitterPane::Impl
{
    TOKOTACHI_IMPL(SplitterPane);

    explicit Impl(SplitterPane* object) : q(object) {}

    void set_stretch(const int& v);
    void set_collapsible(const bool& v);
    bool get_collapsed() const { return collapsed_; }
    void set_collapsed(const bool& v);
    Widget* get_widget() const { return widget_; }

    Splitter* splitter_;
    Widget* widget_;
    int stretch_;
    bool collapsible_;
    float size_;      ///< 並ぶ向きの大きさ(dp。たたんでいる間は使わない)。負ならまだ決めていない
    bool collapsed_;  ///< たたんでいる(大きさ 0 として扱い、部品を隠す)
};

class Splitter::Impl : public detail::WidgetHooks
{
    TOKOTACHI_IMPL(Splitter);

    explicit Impl(Splitter* object) : q(object) {}

    // 内部のフック(docs/adr/0058)
    void child_removed(Widget* child) override;

    Orientation get_orientation() const { return orientation_; }
    void set_orientation(const Orientation& v);
    std::vector<SplitterPane*> get_panes() const { return panes_; }
    std::vector<float> get_sizes() const;
    void set_sizes(const std::vector<float>& v);

    /// 並ぶ向きの長さ・座標。
    float along(const Size& s) const { return orientation_ == Orientation::HORIZONTAL ? s.width : s.height; }
    float along(const Point& p) const { return orientation_ == Orientation::HORIZONTAL ? p.x : p.y; }
    float minimum_of(const SplitterPane* pane) const;
    /// 部品を隠していない区画(並べる・境目を出すもの)。境目の番号はこの並びの番号。
    std::vector<SplitterPane*> shown_panes() const;
    /// 区画の大きさの合計を available に合わせる(初めてなら望む大きさから決める)。
    void fit(float available);
    /// 位置 p にある境目(i 番目の区画の後ろ。無ければ -1)。
    int handle_at(const Point& p) const;
    float handle_position(int i) const;
    /// i 番目の境目を position(並ぶ向きの位置)へ動かす。利用者の操作なので on_change を呼ぶ。
    void drag_to(int i, float position);
    /// i 番目の境目のダブルクリック(隣のたたんだ区画を戻す・どちらかをたたむ)。
    void toggle_handle(int i);
    /// 区画をたたむ・戻す(その分は隣の区画が受け持つ)。変えたら true。
    bool collapse_pane(SplitterPane* pane, bool collapse);

    Orientation orientation_;
    std::vector<SplitterPane*> panes_;
    NotifyHandler on_change_;
    int hover_handle_;
    int drag_handle_;
    float drag_offset_;  ///< 押した位置と境目の始まりの差
    int key_handle_;     ///< キーで動かす境目(境目を押してフォーカスを得たとき。無ければ -1)
    bool destroying_;
};

SplitterPane::SplitterPane(Splitter* splitter, Widget* pane_widget, int initial_stretch)
    : Object(splitter),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, int, stretch),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, collapsible),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, collapsed),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Widget*, widget),
      d_(new Impl(this))
{
    d_->splitter_ = splitter;
    d_->widget_ = pane_widget;
    d_->stretch_ = std::max(0, initial_stretch);
    d_->collapsible_ = true;
    d_->size_ = -1;
    d_->collapsed_ = false;
}

SplitterPane::~SplitterPane() {
    delete d_;
}

void SplitterPane::Impl::set_stretch(const int& v)
{
    stretch_ = std::max(0, v);
}

void SplitterPane::Impl::set_collapsible(const bool& v) { collapsible_ = v; }

void SplitterPane::Impl::set_collapsed(const bool& v) { splitter_->d_->collapse_pane(q, v); }

// ---- Splitter ----

Splitter::Splitter(Widget* parent_widget, Orientation initial_orientation)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, Orientation, orientation),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::vector<SplitterPane*>, panes),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, std::vector<float>, sizes),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_change),
      d_(new Impl(this))
{
    d_->install(this);
    d_->orientation_ = initial_orientation;
    d_->hover_handle_ = -1;
    d_->drag_handle_ = -1;
    d_->drag_offset_ = 0;
    d_->key_handle_ = -1;
    d_->destroying_ = false;
}

Splitter::~Splitter()
{
    // 区画の部品(子)は Widget のデストラクタが破棄する。区画はここで先に破棄する。
    d_->destroying_ = true;
    for (std::size_t i = 0; i < d_->panes_.size(); ++i)
    {
        delete d_->panes_[i];
    }
    d_->panes_.clear();
    delete d_;
}

void Splitter::Impl::set_orientation(const Orientation& v)
{
    if (v == orientation_)
    {
        return;
    }
    orientation_ = v;
    for (std::size_t i = 0; i < panes_.size(); ++i)
    {
        panes_[i]->d_->size_ = -1;  // 望む大きさから決め直す
    }
    q->request_layout();
    detail::WidgetAccess::mark_layout_dirty(q);
    q->invalidate();
}

SplitterPane* Splitter::add_widget(Widget* w, int stretch_value)
{
    return insert_widget(static_cast<int>(d_->panes_.size()), w, stretch_value);
}

SplitterPane* Splitter::insert_widget(int index, Widget* w, int stretch_value)
{
    if (!w || w == this)
    {
        return nullptr;
    }
    for (std::size_t i = 0; i < d_->panes_.size(); ++i)
    {
        if (d_->panes_[i]->d_->widget_ == w)
        {
            return d_->panes_[i];
        }
    }
    w->parent = this;
    if (w->parent.get() != this)
    {
        return nullptr;
    }
    const int n = static_cast<int>(d_->panes_.size());
    const int at = index >= 0 && index <= n ? index : n;
    SplitterPane* pane = new SplitterPane(this, w, stretch_value);
    d_->panes_.insert(d_->panes_.begin() + at, pane);
    notify_changed(panes);
    request_layout();
    detail::WidgetAccess::mark_layout_dirty(this);
    return pane;
}

void Splitter::Impl::child_removed(Widget* child)
{
    if (destroying_)
    {
        return;
    }
    for (std::size_t i = 0; i < panes_.size(); ++i)
    {
        if (panes_[i]->d_->widget_ == child)
        {
            delete panes_[i];
            panes_.erase(panes_.begin() + static_cast<std::ptrdiff_t>(i));
            hover_handle_ = drag_handle_ = -1;
            q->notify_changed(q->panes);
            q->request_layout();
            detail::WidgetAccess::mark_layout_dirty(q);
            return;
        }
    }
}

std::vector<float> Splitter::Impl::get_sizes() const
{
    std::vector<float> out;
    for (std::size_t i = 0; i < panes_.size(); ++i)
    {
        out.push_back(panes_[i]->d_->collapsed_ ? 0.0f : std::max(0.0f, panes_[i]->d_->size_));
    }
    return out;
}

void Splitter::Impl::set_sizes(const std::vector<float>& v)
{
    for (std::size_t i = 0; i < panes_.size() && i < v.size(); ++i)
    {
        SplitterPane* p = panes_[i];
        const bool collapse = v[i] <= 0 && p->d_->collapsible_;
        if (collapse != p->d_->collapsed_)
        {
            p->d_->collapsed_ = collapse;
            p->notify_changed(p->collapsed);
        }
        if (!collapse)
        {
            p->d_->size_ = std::max(0.0f, v[i]);
        }
    }
    detail::WidgetAccess::mark_layout_dirty(q);
    q->invalidate();
}

float Splitter::Impl::minimum_of(const SplitterPane* pane) const { return along(pane->d_->widget_->minimum_hint.get()); }

std::vector<SplitterPane*> Splitter::Impl::shown_panes() const
{
    std::vector<SplitterPane*> out;
    for (std::size_t i = 0; i < panes_.size(); ++i)
    {
        if (panes_[i]->d_->widget_->visible.get())
        {
            out.push_back(panes_[i]);
        }
    }
    return out;
}

/// ---- 大きさの配り方 ----

namespace
{
// 区画の、並ぶ向きの今の大きさ(たたんでいれば 0)。
float extent_of(float size, bool collapsed) { return collapsed ? 0.0f : std::max(0.0f, size); }
}  // namespace

bool Splitter::Impl::collapse_pane(SplitterPane* pane, bool collapse)
{
    if (collapse == pane->d_->collapsed_ || (collapse && !pane->d_->collapsible_))
    {
        return false;
    }
    // 隣(後ろ、無ければ前)のたたんでいない区画が、大きさを受け持つ
    const std::vector<SplitterPane*> shown = shown_panes();
    const std::vector<SplitterPane*>::const_iterator at = std::find(shown.begin(), shown.end(), pane);
    SplitterPane* neighbor = nullptr;
    if (at != shown.end())
    {
        for (std::vector<SplitterPane*>::const_iterator it = at + 1; it != shown.end() && !neighbor; ++it)
        {
            neighbor = (*it)->d_->collapsed_ ? nullptr : *it;
        }
        for (std::vector<SplitterPane*>::const_iterator it = at; it != shown.begin() && !neighbor;)
        {
            --it;
            neighbor = (*it)->d_->collapsed_ ? nullptr : *it;
        }
    }
    if (collapse)
    {
        // 大きさは覚えておく(戻すときに使う)
        if (neighbor)
        {
            neighbor->d_->size_ = extent_of(neighbor->d_->size_, false) + extent_of(pane->d_->size_, false);
        }
    }
    else
    {
        // 覚えている大きさ(最小・望む大きさ以上)で戻し、隣から最小を超える分だけもらう(足りない分は stretch で配り直す)
        const float want = std::max(pane->d_->size_, std::max(minimum_of(pane), along(pane->d_->widget_->preferred_size.get())));
        pane->d_->size_ = want;
        if (neighbor)
        {
            const float give = std::max(0.0f, std::min(want, neighbor->d_->size_ - minimum_of(neighbor)));
            neighbor->d_->size_ -= give;
        }
    }
    pane->d_->collapsed_ = collapse;
    pane->notify_changed(pane->collapsed);
    detail::WidgetAccess::mark_layout_dirty(q);
    q->invalidate();
    q->notify_changed(q->sizes);
    return true;
}

void Splitter::Impl::toggle_handle(int i)
{
    const std::vector<SplitterPane*> shown = shown_panes();
    if (i < 0 || i + 1 >= static_cast<int>(shown.size()))
    {
        return;
    }
    SplitterPane* a = shown[static_cast<std::size_t>(i)];
    SplitterPane* b = shown[static_cast<std::size_t>(i + 1)];
    bool changed = false;
    if (a->d_->collapsed_ || b->d_->collapsed_)
    {
        changed = collapse_pane(a->d_->collapsed_ ? a : b, false);
    }
    else
    {
        // stretch の小さい方(同じなら前)をたたむ。たためなければ、もう一方
        SplitterPane* first = a->d_->stretch_ <= b->d_->stretch_ ? a : b;
        SplitterPane* second = first == a ? b : a;
        changed = collapse_pane(first->d_->collapsible_ ? first : second, true);
    }
    if (changed)
    {
        Object* self = q;
        detail::fire(on_change_, self);
    }
}

void Splitter::Impl::fit(float available)
{
    const std::vector<SplitterPane*> shown = shown_panes();  // 部品を隠した区画には配らない
    const int n = static_cast<int>(shown.size());
    if (n == 0)
    {
        return;
    }
    const float space = std::max(0.0f, available - q->HANDLE * static_cast<float>(n - 1));
    // まだ大きさの無い区画は、望む大きさから始める。
    for (int i = 0; i < n; ++i)
    {
        if (shown[i]->d_->size_ < 0)
        {
            shown[i]->d_->size_ = along(shown[i]->d_->widget_->preferred_size.get());
        }
    }
    float delta = space;
    for (int i = 0; i < n; ++i)
    {
        delta -= extent_of(shown[i]->d_->size_, shown[i]->d_->collapsed_);
    }
    // 余り・不足を stretch の比率で配る(縮めるときは最小の大きさで止め、残りを他へ回す)。たたんだ区画には配らない。
    for (int round = 0; round < n + 1 && std::fabs(delta) > 0.01f; ++round)
    {
        std::vector<SplitterPane*> takers;
        int total = 0;
        for (int i = 0; i < n; ++i)
        {
            SplitterPane* p = shown[i];
            if (p->d_->collapsed_ || p->d_->stretch_ == 0 || (delta < 0 && p->d_->size_ <= minimum_of(p)))
            {
                continue;
            }
            takers.push_back(p);
            total += p->d_->stretch_;
        }
        if (takers.empty())
        {
            // 伸びる区画が無い: たたんでいない区画のうち、最後の(縮めるなら最小より大きい)ものが受け持つ。
            for (int i = n - 1; i >= 0; --i)
            {
                SplitterPane* p = shown[i];
                if (!p->d_->collapsed_ && (delta > 0 || p->d_->size_ > minimum_of(p)))
                {
                    takers.push_back(p);
                    total = std::max(1, p->d_->stretch_);
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
            const int weight = std::max(1, p->d_->stretch_);
            float next = p->d_->size_ + delta * static_cast<float>(weight) / static_cast<float>(std::max(1, total));
            if (delta < 0)
            {
                next = std::max(next, minimum_of(p));
            }
            applied += next - p->d_->size_;
            p->d_->size_ = next;
        }
        delta -= applied;
    }
    if (delta < -0.01f)
    {
        // どの区画も最小まで縮めた: 収まらない分は、大きさの比で最小より小さくする(はみ出しを切る)。
        float total = 0;
        for (int i = 0; i < n; ++i)
        {
            total += extent_of(shown[i]->d_->size_, shown[i]->d_->collapsed_);
        }
        for (int i = 0; i < n && total > 0; ++i)
        {
            if (!shown[i]->d_->collapsed_)
            {
                shown[i]->d_->size_ = std::max(0.0f, shown[i]->d_->size_ + delta * shown[i]->d_->size_ / total);
            }
        }
    }
    else if (delta > 0.01f)
    {
        // すべての区画をたたんでいる: 最後の区画を開いて受け持たせる。
        SplitterPane* last = shown[static_cast<std::size_t>(n - 1)];
        if (last->d_->collapsed_)
        {
            last->d_->collapsed_ = false;
            last->d_->size_ = 0;
            last->notify_changed(last->collapsed);
        }
        last->d_->size_ += delta;
    }
}

void Splitter::arrange_children()
{
    const std::vector<float> before = d_->get_sizes();
    const Rect r = local_rect;
    const bool horizontal = d_->orientation_ == Orientation::HORIZONTAL;
    d_->fit(horizontal ? r.width : r.height);
    const std::vector<SplitterPane*> shown = d_->shown_panes();
    if (d_->drag_handle_ + 1 >= static_cast<int>(shown.size()) || d_->hover_handle_ + 1 >= static_cast<int>(shown.size()))
    {
        d_->drag_handle_ = d_->hover_handle_ = -1;  // 区画を隠して、境目が無くなった
    }
    float pos = 0;
    for (std::size_t i = 0; i < shown.size(); ++i)
    {
        SplitterPane* p = shown[i];
        const float s = extent_of(p->d_->size_, p->d_->collapsed_);
        detail::WidgetAccess::set_hidden_by_parent(p->d_->widget_, p->d_->collapsed_);  // たたんだ区画は隠す(visible は変えない)
        if (!p->d_->collapsed_)
        {
            p->d_->widget_->bounds = horizontal ? Rect(pos, 0, s, r.height) : Rect(0, pos, r.width, s);
        }
        pos += s + HANDLE;
    }
    invalidate();
    if (d_->get_sizes() != before)
    {
        notify_changed(sizes);
    }
}

Size Splitter::compute_preferred_size() const
{
    const std::vector<SplitterPane*> shown = d_->shown_panes();
    float a = 0, cross = 0;
    for (std::size_t i = 0; i < shown.size(); ++i)
    {
        const Size s = shown[i]->d_->widget_->preferred_size;
        if (!shown[i]->d_->collapsed_)
        {
            a += d_->along(s);
        }
        cross = std::max(cross, d_->orientation_ == Orientation::HORIZONTAL ? s.height : s.width);
    }
    a += HANDLE * static_cast<float>(shown.empty() ? 0 : shown.size() - 1);
    return d_->orientation_ == Orientation::HORIZONTAL ? Size(a, cross) : Size(cross, a);
}

Size Splitter::compute_minimum_size() const
{
    const std::vector<SplitterPane*> shown = d_->shown_panes();
    float a = 0, cross = 0;
    for (std::size_t i = 0; i < shown.size(); ++i)
    {
        const Size s = shown[i]->d_->widget_->minimum_hint;
        if (!shown[i]->d_->collapsed_)
        {
            a += d_->along(s);
        }
        cross = std::max(cross, d_->orientation_ == Orientation::HORIZONTAL ? s.height : s.width);
    }
    a += HANDLE * static_cast<float>(shown.empty() ? 0 : shown.size() - 1);
    return d_->orientation_ == Orientation::HORIZONTAL ? Size(a, cross) : Size(cross, a);
}

// ---- 境目 ----

float Splitter::Impl::handle_position(int i) const
{
    const std::vector<SplitterPane*> shown = shown_panes();
    float pos = 0;
    for (int k = 0; k <= i && k < static_cast<int>(shown.size()); ++k)
    {
        const SplitterPane* p = shown[static_cast<std::size_t>(k)];
        pos += extent_of(p->d_->size_, p->d_->collapsed_);
    }
    return pos + q->HANDLE * static_cast<float>(i);
}

int Splitter::Impl::handle_at(const Point& p) const
{
    const float a = along(p);
    const int n = static_cast<int>(shown_panes().size());
    for (int i = 0; i + 1 < n; ++i)
    {
        const float h = handle_position(i);
        if (a >= h - GRAB_MARGIN && a < h + q->HANDLE + GRAB_MARGIN)
        {
            return i;
        }
    }
    return -1;
}

void Splitter::Impl::drag_to(int handle, float position)
{
    const std::vector<SplitterPane*> shown = shown_panes();
    if (handle < 0 || handle + 1 >= static_cast<int>(shown.size()))
    {
        return;
    }
    SplitterPane* a = shown[static_cast<std::size_t>(handle)];
    SplitterPane* b = shown[static_cast<std::size_t>(handle + 1)];
    const float ea = extent_of(a->d_->size_, a->d_->collapsed_), eb = extent_of(b->d_->size_, b->d_->collapsed_);
    const float start = handle_position(handle) - ea;
    const float combined = ea + eb;
    const float min_a = std::min(minimum_of(a), combined), min_b = std::min(minimum_of(b), combined);
    // 前の区画の大きさを決め、最小より小さければたたむ(たためなければ最小)。後ろの区画も同じ。
    float sa = std::max(0.0f, std::min(position - start, combined));
    bool collapse_a = false, collapse_b = false;
    if (sa < min_a)
    {
        collapse_a = a->d_->collapsible_ && sa < min_a / 2;
        sa = collapse_a ? 0 : min_a;
    }
    float sb = combined - sa;
    if (sb < min_b)
    {
        collapse_b = b->d_->collapsible_ && sb < min_b / 2;
        sb = collapse_b ? 0 : min_b;
        sa = std::max(0.0f, combined - sb);
        collapse_a = collapse_a && sa <= 0;
    }
    if (sa == ea && sb == eb && collapse_a == a->d_->collapsed_ && collapse_b == b->d_->collapsed_)
    {
        return;
    }
    const bool changed_a = collapse_a != a->d_->collapsed_, changed_b = collapse_b != b->d_->collapsed_;
    a->d_->collapsed_ = collapse_a;
    b->d_->collapsed_ = collapse_b;
    if (!collapse_a)
    {
        a->d_->size_ = sa;
    }
    if (!collapse_b)
    {
        b->d_->size_ = sb;
    }
    if (changed_a)
    {
        a->notify_changed(a->collapsed);
    }
    if (changed_b)
    {
        b->notify_changed(b->collapsed);
    }
    detail::WidgetAccess::mark_layout_dirty(q);
    q->invalidate();
    q->notify_changed(q->sizes);
    Object* self = q;
    detail::fire(on_change_, self);
}

void Splitter::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool horizontal = d_->orientation_ == Orientation::HORIZONTAL;
    const int n = static_cast<int>(d_->shown_panes().size());
    for (int i = 0; i + 1 < n; ++i)
    {
        const float h = d_->handle_position(i);
        const Rect band = horizontal ? Rect(h, 0, HANDLE, r.height) : Rect(0, h, r.width, HANDLE);
        if (i == d_->drag_handle_ || (d_->drag_handle_ < 0 && i == d_->hover_handle_))
        {
            canvas.fill_rect(band, theme::selection_inactive());
        }
        // キーで動かす境目は、フォーカスの色で太く示す
        const bool keyed = i == d_->key_handle_ && focused;
        const float thick = keyed ? 2 : 1;
        const Rect line = horizontal ? Rect(std::floor(h + (HANDLE - thick) / 2), 0, thick, r.height)
                                     : Rect(0, std::floor(h + (HANDLE - thick) / 2), r.width, thick);
        canvas.fill_rect(line, keyed ? theme::focus_ring() : theme::border());
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
    const int i = d_->handle_at(e.pos);
    if (i < 0)
    {
        return;
    }
    e.accepted = true;
    // 境目を押したときだけフォーカスを得る(キーで動かすため)。区画の中を押したときには得ないよう、普段は NONE にしておく
    d_->key_handle_ = i;
    focus_policy = FocusPolicy::CLICK;
    focused = true;
    invalidate();
    if (e.clicks == 2)
    {
        d_->toggle_handle(i);
        return;
    }
    d_->drag_handle_ = i;
    d_->drag_offset_ = d_->along(e.pos) - d_->handle_position(i);
    invalidate();
}

void Splitter::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    if (d_->drag_handle_ >= 0)
    {
        d_->drag_to(d_->drag_handle_, d_->along(e.pos) - d_->drag_offset_);
        return;
    }
    const int i = d_->handle_at(e.pos);
    if (i != d_->hover_handle_)
    {
        d_->hover_handle_ = i;
        invalidate();
    }
    cursor = i < 0                                         ? MouseCursor::ARROW
             : d_->orientation_ == Orientation::HORIZONTAL ? MouseCursor::SIZE_HORIZONTAL
                                                       : MouseCursor::SIZE_VERTICAL;
}

void Splitter::mouse_up(MouseEvent& e)
{
    Widget::mouse_up(e);
    if (e.button == MouseButton::LEFT && d_->drag_handle_ >= 0)
    {
        e.accepted = true;
        d_->drag_handle_ = -1;
        d_->hover_handle_ = d_->handle_at(e.pos);
        if (d_->hover_handle_ < 0)
        {
            cursor = MouseCursor::ARROW;  // 境目の外で離した
        }
        invalidate();
    }
}

void Splitter::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    if (d_->key_handle_ < 0 || d_->key_handle_ + 1 >= static_cast<int>(d_->shown_panes().size()) || e.modifiers != MODIFIER_NONE)
    {
        return;
    }
    const bool horizontal = d_->orientation_ == Orientation::HORIZONTAL;
    float step = 0;
    if (e.key == (horizontal ? Key::LEFT : Key::UP))
    {
        step = -KEY_STEP;
    }
    else if (e.key == (horizontal ? Key::RIGHT : Key::DOWN))
    {
        step = KEY_STEP;
    }
    else if (e.key == Key::RETURN && !e.repeat)
    {
        e.accepted = true;
        d_->toggle_handle(d_->key_handle_);
        return;
    }
    else
    {
        return;
    }
    e.accepted = true;
    d_->drag_to(d_->key_handle_, d_->handle_position(d_->key_handle_) + step);
}

void Splitter::focus_out()
{
    d_->key_handle_ = -1;
    focus_policy = FocusPolicy::NONE;
    invalidate();
    Widget::focus_out();
}

void Splitter::mouse_leave()
{
    if (d_->hover_handle_ >= 0 && d_->drag_handle_ < 0)
    {
        d_->hover_handle_ = -1;
        invalidate();
    }
    if (d_->drag_handle_ < 0)
    {
        cursor = MouseCursor::ARROW;
    }
    Widget::mouse_leave();
}

}  // namespace tokotachi
