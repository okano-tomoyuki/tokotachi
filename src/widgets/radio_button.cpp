#include <gui/radio_button.hpp>

#include <gui/canvas.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"
#include "toggle_painter.hpp"

namespace gui
{

RadioButton::RadioButton(Widget* parent_widget, const std::string& initial_text, int initial_value)
    : ButtonBase(parent_widget, initial_text),
      GUI_PROPERTY_MEMBER_SELF_NOTIFY(RadioButton, int, value),
      GUI_PROPERTY_MEMBER_SELF_NOTIFY(RadioButton, int, group_value),
      GUI_READONLY_PROPERTY(RadioButton, bool, checked),
      GUI_HANDLER_PROPERTY(RadioButton, NotifyHandler, on_change),
      value_(initial_value),
      group_value_(-1)
{
}

void RadioButton::set_value(const int& v)
{
    if (v == value_)
    {
        return;
    }
    const bool was_checked = get_checked();
    value_ = v;
    finish(was_checked, &value);
}

void RadioButton::set_group_value(const int& v)
{
    if (v == group_value_)
    {
        return;
    }
    const bool was_checked = get_checked();
    group_value_ = v;
    finish(was_checked, &group_value);
}

void RadioButton::finish(bool was_checked, const void* changed_property)
{
    // 結びつけた相手へ伝えてから、ハンドラを呼ぶ。
    if (changed_property == &value)
    {
        notify_changed(value);
    }
    else
    {
        notify_changed(group_value);
    }
    if (was_checked == get_checked())
    {
        return;
    }
    invalidate();
    notify_changed(checked);
    Object* self = this;
    detail::fire(on_change_, self);
}

void RadioButton::click()
{
    set_group_value(value_);
    ButtonBase::click();
}

Size RadioButton::compute_preferred_size() const { return detail::toggle_preferred_size(text_layout()); }

void RadioButton::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const Rect box = detail::toggle_indicator_rect(r);
    const bool enabled_now = effectively_enabled;
    const Point c = box.center();
    const float radius = box.width / 2;
    Path outer;
    outer.add_ellipse(c, radius, radius);
    if (get_checked())
    {
        canvas.fill_path(outer, enabled_now ? theme::accent() : theme::accent_disabled());
        Path dot;
        dot.add_ellipse(c, radius * 0.4f, radius * 0.4f);
        canvas.fill_path(dot, theme::on_accent());
    }
    else
    {
        canvas.fill_path(outer, enabled_now ? theme::field() : theme::field_disabled());
        Path ring;
        ring.add_ellipse(c, radius - 0.5f, radius - 0.5f);
        canvas.stroke_path(ring, 1, enabled_now && is_hovered() ? theme::accent() : theme::border());
    }
    detail::paint_toggle_text(canvas, text_layout(), r, focused, enabled_now);
    Widget::paint(canvas);
}

}  // namespace gui
