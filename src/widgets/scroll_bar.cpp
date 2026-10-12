#include <tokotachi/widgets/scroll_bar.hpp>

#include <algorithm>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

const float ScrollBar::THICKNESS = 10;

namespace
{
const float MIN_THUMB = 20;  // つまみの最小の長さ(dp)
}

class ScrollBar::Impl
{
    TOKOTACHI_IMPL(ScrollBar);

    explicit Impl(ScrollBar* object) : q(object) {}

    Orientation get_orientation() const { return orientation_; }
    void set_orientation(const Orientation& v);

    float value() const { return static_cast<float>(q->value.get()); }
    float maximum() const { return static_cast<float>(q->maximum.get()); }
    float page_step() const { return static_cast<float>(q->page_step.get()); }

    /// 溝の長さ・つまみの位置と長さ(並ぶ向き、dp)。
    float track_length() const;
    void thumb(float& pos, float& len) const;
    /// 並ぶ向きの座標。
    float along(const Point& p) const;

    Orientation orientation_;
    bool hovered_;
    bool dragging_;
    float drag_offset_;  ///< つまみの始めから、押した位置まで
};

ScrollBar::ScrollBar(Widget* parent_widget, Orientation initial_orientation)
    : RangeBase(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, Orientation, orientation),
      d_(new Impl(this))
{
    d_->orientation_ = initial_orientation;
    maximum = 0;
    page_step = 0;
    single_step = 48;
    d_->hovered_ = false;
    d_->dragging_ = false;
    d_->drag_offset_ = 0;
}

ScrollBar::~ScrollBar() { delete d_; }

void ScrollBar::Impl::set_orientation(const Orientation& v)
{
    orientation_ = v;
    q->invalidate();
    q->request_layout();
}

float ScrollBar::Impl::along(const Point& p) const { return orientation_ == Orientation::VERTICAL ? p.y : p.x; }

float ScrollBar::Impl::track_length() const
{
    const Rect r = q->local_rect;
    return orientation_ == Orientation::VERTICAL ? r.height : r.width;
}

void ScrollBar::Impl::thumb(float& pos, float& len) const
{
    const float track = track_length();
    const float total = maximum() + page_step();
    len = total > 0 ? std::max(MIN_THUMB, track * page_step() / total) : track;
    len = std::min(len, track);
    pos = maximum() > 0 ? (track - len) * value() / maximum() : 0;
}

Size ScrollBar::compute_preferred_size() const
{
    return d_->orientation_ == Orientation::VERTICAL ? Size(THICKNESS, MIN_THUMB * 2) : Size(MIN_THUMB * 2, THICKNESS);
}

Size ScrollBar::compute_minimum_size() const { return compute_preferred_size(); }

void ScrollBar::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    canvas.fill_rect(r, theme::scroll_track());
    if (d_->maximum() > 0)
    {
        float pos = 0, len = 0;
        d_->thumb(pos, len);
        const float inset = 2;
        const Rect t = d_->orientation_ == Orientation::VERTICAL ? Rect(inset, pos + inset, r.width - inset * 2, len - inset * 2)
                                                             : Rect(pos + inset, inset, len - inset * 2, r.height - inset * 2);
        canvas.fill_round_rect(t, (THICKNESS - inset * 2) / 2,
                               d_->dragging_ || d_->hovered_ ? theme::scroll_thumb_active() : theme::scroll_thumb());
    }
    Widget::paint(canvas);
}

void ScrollBar::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    e.accepted = true;
    if (d_->maximum() <= 0)
    {
        return;
    }
    float pos = 0, len = 0;
    d_->thumb(pos, len);
    const float at = d_->along(e.pos);
    if (at >= pos && at < pos + len)
    {
        // つまみをつかむ。
        d_->dragging_ = true;
        d_->drag_offset_ = at - pos;
        invalidate();
    }
    else
    {
        // 溝のクリックは 1 ページ分動かす。
        value = d_->value() + (at < pos ? -d_->page_step() : d_->page_step());
    }
}

void ScrollBar::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    if (!d_->dragging_)
    {
        return;
    }
    float pos = 0, len = 0;
    d_->thumb(pos, len);
    const float room = d_->track_length() - len;
    if (room > 0)
    {
        value = (d_->along(e.pos) - d_->drag_offset_) / room * d_->maximum();
    }
}

void ScrollBar::mouse_up(MouseEvent& e)
{
    Widget::mouse_up(e);
    if (e.button == MouseButton::LEFT)
    {
        e.accepted = true;
        d_->dragging_ = false;
        invalidate();
    }
}

void ScrollBar::mouse_enter()
{
    d_->hovered_ = true;
    invalidate();
    Widget::mouse_enter();
}

void ScrollBar::mouse_leave()
{
    d_->hovered_ = false;
    invalidate();
    Widget::mouse_leave();
}

void ScrollBar::mouse_wheel(WheelEvent& e)
{
    Widget::mouse_wheel(e);
    if (d_->maximum() <= 0)
    {
        return;  // 動かせなければ親へ送る
    }
    e.accepted = true;
    // 縦のスクロールバーは縦のホイール(奥が上)で、横は横のホイール(無ければ縦)で動かす。
    const float ticks = d_->orientation_ == Orientation::VERTICAL ? -e.delta.y : (e.delta.x != 0 ? e.delta.x : -e.delta.y);
    value = d_->value() + ticks * static_cast<float>(single_step.get());
}

}  // namespace tokotachi
