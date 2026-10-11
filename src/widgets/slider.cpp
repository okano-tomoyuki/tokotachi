#include <tokotachi/widgets/slider.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

namespace
{
const float THUMB_RADIUS = 8;   // つまみの半径(dp)
const float TRACK_WIDTH = 4;    // 溝の太さ(dp)
const float LENGTH = 160;       // 望む長さ(dp)
const float MIN_LENGTH = 48;    // 最小の長さ(dp)
const float TICK_ROOM = 6;      // 目盛りのために、溝の下(縦なら右)に足す幅(dp)
const float TICK_LENGTH = 4;    // 目盛りの線の長さ(dp)
const float TICK_MIN_GAP = 4;   // これより詰まる目盛りは出さない(dp)
}  // namespace

Slider::Slider(Widget* parent_widget, Orientation initial_orientation)
    : RangeBase(parent_widget),
      TOKOTACHI_PROPERTY_GETTER(Slider, Orientation, orientation),
      TOKOTACHI_PROPERTY_GETTER(Slider, double, tick_interval),
      orientation_(initial_orientation),
      tick_interval_(0),
      hovered_(false),
      dragging_(false)
{
    focus_policy = FocusPolicy::TAB_AND_CLICK;
}

void Slider::set_orientation(const Orientation& v)
{
    orientation_ = v;
    invalidate();
    request_layout();
}

void Slider::set_tick_interval(const double& v)
{
    const double interval = v > 0 && std::isfinite(v) ? v : 0;
    if (interval == tick_interval_)
    {
        return;
    }
    const bool room_changed = (interval > 0) != (tick_interval_ > 0);
    tick_interval_ = interval;
    invalidate();
    if (room_changed)
    {
        request_layout();
    }
}

float Slider::tick_room() const { return tick_interval_ > 0 ? TICK_ROOM : 0; }

float Slider::travel() const
{
    const Rect r = local_rect;
    const float length = orientation_ == Orientation::HORIZONTAL ? r.width : r.height;
    return std::max(0.0f, length - THUMB_RADIUS * 2);
}

Point Slider::thumb_center() const
{
    const Rect r = local_rect;
    const float t = static_cast<float>(ratio()) * travel();
    if (orientation_ == Orientation::HORIZONTAL)
    {
        return Point(THUMB_RADIUS + t, (r.height - tick_room()) / 2);
    }
    return Point((r.width - tick_room()) / 2, r.height - THUMB_RADIUS - t);  // 縦は下が minimum
}

double Slider::value_at(const Point& p) const
{
    const Rect r = local_rect;
    const float len = travel();
    if (len <= 0)
    {
        return minimum;
    }
    const float along = orientation_ == Orientation::HORIZONTAL ? p.x - THUMB_RADIUS : r.height - THUMB_RADIUS - p.y;
    const double t = std::max(0.0, std::min(1.0, static_cast<double>(along / len)));
    return snapped(minimum + t * (maximum - minimum));
}

Size Slider::compute_preferred_size() const
{
    const float thickness = THUMB_RADIUS * 2 + 8 + tick_room();  // フォーカスの輪・目盛りの分を含める
    return orientation_ == Orientation::HORIZONTAL ? Size(LENGTH, thickness) : Size(thickness, LENGTH);
}

Size Slider::compute_minimum_size() const
{
    const Size p = compute_preferred_size();
    return orientation_ == Orientation::HORIZONTAL ? Size(MIN_LENGTH, p.height) : Size(p.width, MIN_LENGTH);
}

void Slider::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool enabled_now = effectively_enabled;
    const Point c = thumb_center();
    // 溝: minimum からつまみまでを強調の色で塗る。
    Rect track;
    Rect filled;
    if (orientation_ == Orientation::HORIZONTAL)
    {
        track = Rect(THUMB_RADIUS, c.y - TRACK_WIDTH / 2, travel(), TRACK_WIDTH);
        filled = Rect(track.x, track.y, c.x - track.x, TRACK_WIDTH);
    }
    else
    {
        track = Rect(c.x - TRACK_WIDTH / 2, THUMB_RADIUS, TRACK_WIDTH, travel());
        filled = Rect(track.x, c.y, TRACK_WIDTH, track.bottom() - c.y);
    }
    canvas.fill_round_rect(track, TRACK_WIDTH / 2, enabled_now ? theme::scroll_track() : theme::field_disabled());
    canvas.stroke_round_rect(track, TRACK_WIDTH / 2, 1, theme::border());
    if (!filled.is_empty())
    {
        canvas.fill_round_rect(filled, TRACK_WIDTH / 2, enabled_now ? theme::accent() : theme::accent_disabled());
    }
    // 目盛り: minimum から tick_interval ごとと maximum。詰まりすぎるなら出さない
    const double span = maximum.get() - minimum.get();
    if (tick_interval_ > 0 && span > 0 && travel() * tick_interval_ / span >= TICK_MIN_GAP)
    {
        const Color tick = enabled_now ? theme::placeholder() : theme::border();
        const float ty = THUMB_RADIUS + 5;  // つまみの中心から目盛りの始まりまで
        const int n = static_cast<int>(std::floor(span / tick_interval_ + 1e-9));
        for (int k = 0; k <= n + 1; ++k)
        {
            const double v = k <= n ? k * tick_interval_ : span;
            if (k == n + 1 && std::fabs(n * tick_interval_ - span) < 1e-9 * std::max(1.0, span))
            {
                break;  // maximum は最後の目盛りと重なる
            }
            const float t = std::floor(static_cast<float>(v / span) * travel());
            const Rect line = orientation_ == Orientation::HORIZONTAL
                                  ? Rect(THUMB_RADIUS + t, c.y + ty, 1, TICK_LENGTH)
                                  : Rect(c.x + ty, r.height - THUMB_RADIUS - t - 1, TICK_LENGTH, 1);
            canvas.fill_rect(line, tick);
        }
    }
    // つまみ
    const Rect thumb(c.x - THUMB_RADIUS, c.y - THUMB_RADIUS, THUMB_RADIUS * 2, THUMB_RADIUS * 2);
    if (focused && enabled_now)
    {
        canvas.stroke_round_rect(thumb.adjusted(-3, -3, -3, -3), THUMB_RADIUS + 3, 2, theme::focus_ring());
    }
    canvas.fill_round_rect(thumb, THUMB_RADIUS, enabled_now ? theme::field() : theme::field_disabled());
    const Color ring = !enabled_now                  ? theme::accent_disabled()
                       : dragging_                   ? theme::accent_pressed()
                       : hovered_                    ? theme::accent_hover()
                                                     : theme::accent();
    canvas.stroke_round_rect(thumb.adjusted(1, 1, 1, 1), THUMB_RADIUS - 1, 2, ring);
    Widget::paint(canvas);
}

void Slider::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    e.accepted = true;
    const Point c = thumb_center();
    const float dx = e.pos.x - c.x;
    const float dy = e.pos.y - c.y;
    if (dx * dx + dy * dy <= THUMB_RADIUS * THUMB_RADIUS)
    {
        // つまみをつかむ(押した位置とつまみの中心のずれを保つ)。
        drag_offset_ = Point(dx, dy);
    }
    else
    {
        // 溝を押したら、その位置へ動かしてからつかむ。
        drag_offset_ = Point();
        value = value_at(e.pos);
    }
    dragging_ = true;
    invalidate();
}

void Slider::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    if (dragging_)
    {
        value = value_at(Point(e.pos.x - drag_offset_.x, e.pos.y - drag_offset_.y));
    }
}

void Slider::mouse_up(MouseEvent& e)
{
    Widget::mouse_up(e);
    if (e.button == MouseButton::LEFT && dragging_)
    {
        e.accepted = true;
        dragging_ = false;
        invalidate();
    }
}

void Slider::mouse_enter()
{
    hovered_ = true;
    invalidate();
    Widget::mouse_enter();
}

void Slider::mouse_leave()
{
    hovered_ = false;
    invalidate();
    Widget::mouse_leave();
}

void Slider::mouse_wheel(WheelEvent& e)
{
    Widget::mouse_wheel(e);
    if (!focused)
    {
        return;  // 親(スクロール領域など)へ送る
    }
    e.accepted = true;
    // 奥(上)へ回すと増やす。横のホイールは右で増やす。
    const float ticks = e.delta.y != 0 ? e.delta.y : e.delta.x;
    step_by(ticks * single_step.get());
}

void Slider::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    switch (e.key)
    {
    case Key::LEFT:
    case Key::DOWN:
        step_by(-single_step.get());
        break;
    case Key::RIGHT:
    case Key::UP:
        step_by(single_step.get());
        break;
    case Key::PAGE_UP:
        step_by(page_step.get());
        break;
    case Key::PAGE_DOWN:
        step_by(-page_step.get());
        break;
    case Key::HOME:
        value = minimum.get();
        break;
    case Key::END:
        value = maximum.get();
        break;
    default:
        return;
    }
    e.accepted = true;
}

void Slider::focus_in()
{
    invalidate();
    Widget::focus_in();
}

void Slider::focus_out()
{
    invalidate();
    Widget::focus_out();
}

}  // namespace tokotachi
