#include <tokotachi/widgets/scroll_bar.hpp>

#include <algorithm>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

const float ScrollBar::THICKNESS = 10;

namespace
{
const float MIN_THUMB = 20;  // つまみの最小の長さ(dp)
}

ScrollBar::ScrollBar(Widget* parent_widget, Orientation initial_orientation)
    : Widget(parent_widget),
      TOKOTACHI_PROPERTY_GETTER(ScrollBar, Orientation, orientation),
      TOKOTACHI_PROPERTY_GETTER_SELF_NOTIFY(ScrollBar, float, value),
      TOKOTACHI_PROPERTY_GETTER_SELF_NOTIFY(ScrollBar, float, maximum),
      TOKOTACHI_PROPERTY_GETTER(ScrollBar, float, page_step),
      TOKOTACHI_PROPERTY_GETTER(ScrollBar, float, single_step),
      TOKOTACHI_HANDLER_PROPERTY(ScrollBar, NotifyHandler, on_change),
      orientation_(initial_orientation),
      value_(0),
      maximum_(0),
      page_step_(0),
      single_step_(48),
      hovered_(false),
      dragging_(false),
      drag_offset_(0)
{
}

void ScrollBar::set_orientation(const Orientation& v)
{
    orientation_ = v;
    invalidate();
    request_layout();
}

void ScrollBar::set_value(const float& v)
{
    const float clamped = std::max(0.0f, std::min(v, maximum_));
    if (clamped == value_)
    {
        return;
    }
    value_ = clamped;
    invalidate();
    // 結びつけた相手へ伝えてから、ハンドラを呼ぶ。
    notify_changed(value);
    Object* self = this;
    detail::fire(on_change_, self);
}

void ScrollBar::set_maximum(const float& v)
{
    const float m = std::max(0.0f, v);
    if (m == maximum_)
    {
        return;
    }
    maximum_ = m;
    invalidate();
    notify_changed(maximum);
    if (value_ > maximum_)
    {
        set_value(maximum_);
    }
}

void ScrollBar::set_page_step(const float& v)
{
    page_step_ = std::max(0.0f, v);
    invalidate();
}

void ScrollBar::set_single_step(const float& v) { single_step_ = std::max(0.0f, v); }

float ScrollBar::along(const Point& p) const { return orientation_ == Orientation::VERTICAL ? p.y : p.x; }

float ScrollBar::track_length() const
{
    const Rect r = local_rect;
    return orientation_ == Orientation::VERTICAL ? r.height : r.width;
}

void ScrollBar::thumb(float& pos, float& len) const
{
    const float track = track_length();
    const float total = maximum_ + page_step_;
    len = total > 0 ? std::max(MIN_THUMB, track * page_step_ / total) : track;
    len = std::min(len, track);
    pos = maximum_ > 0 ? (track - len) * value_ / maximum_ : 0;
}

Size ScrollBar::compute_preferred_size() const
{
    return orientation_ == Orientation::VERTICAL ? Size(THICKNESS, MIN_THUMB * 2) : Size(MIN_THUMB * 2, THICKNESS);
}

Size ScrollBar::compute_minimum_size() const { return compute_preferred_size(); }

void ScrollBar::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    canvas.fill_rect(r, theme::scroll_track());
    if (maximum_ > 0)
    {
        float pos = 0, len = 0;
        thumb(pos, len);
        const float inset = 2;
        const Rect t = orientation_ == Orientation::VERTICAL ? Rect(inset, pos + inset, r.width - inset * 2, len - inset * 2)
                                                             : Rect(pos + inset, inset, len - inset * 2, r.height - inset * 2);
        canvas.fill_round_rect(t, (THICKNESS - inset * 2) / 2,
                               dragging_ || hovered_ ? theme::scroll_thumb_active() : theme::scroll_thumb());
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
    if (maximum_ <= 0)
    {
        return;
    }
    float pos = 0, len = 0;
    thumb(pos, len);
    const float at = along(e.pos);
    if (at >= pos && at < pos + len)
    {
        // つまみをつかむ。
        dragging_ = true;
        drag_offset_ = at - pos;
        invalidate();
    }
    else
    {
        // 溝のクリックは 1 ページ分動かす。
        value = value_ + (at < pos ? -page_step_ : page_step_);
    }
}

void ScrollBar::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    if (!dragging_)
    {
        return;
    }
    float pos = 0, len = 0;
    thumb(pos, len);
    const float room = track_length() - len;
    if (room > 0)
    {
        value = (along(e.pos) - drag_offset_) / room * maximum_;
    }
}

void ScrollBar::mouse_up(MouseEvent& e)
{
    Widget::mouse_up(e);
    if (e.button == MouseButton::LEFT)
    {
        e.accepted = true;
        dragging_ = false;
        invalidate();
    }
}

void ScrollBar::mouse_enter()
{
    hovered_ = true;
    invalidate();
    Widget::mouse_enter();
}

void ScrollBar::mouse_leave()
{
    hovered_ = false;
    invalidate();
    Widget::mouse_leave();
}

void ScrollBar::mouse_wheel(WheelEvent& e)
{
    Widget::mouse_wheel(e);
    if (maximum_ <= 0)
    {
        return;  // 動かせなければ親へ送る
    }
    e.accepted = true;
    // 縦のスクロールバーは縦のホイール(奥が上)で、横は横のホイール(無ければ縦)で動かす。
    const float ticks = orientation_ == Orientation::VERTICAL ? -e.delta.y : (e.delta.x != 0 ? e.delta.x : -e.delta.y);
    value = value_ + ticks * single_step_;
}

}  // namespace tokotachi
