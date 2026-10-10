#include <tokotachi/widgets/button.hpp>

#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

Button::Button(Widget* parent_widget, const std::string& initial_text)
    : ButtonBase(parent_widget, initial_text), TOKOTACHI_PROPERTY_MEMBER(Button, ButtonStyle, style), style_(ButtonStyle::PRIMARY)
{
}

void Button::set_style(const ButtonStyle& v)
{
    style_ = v;
    invalidate();
}

Size Button::compute_preferred_size() const
{
    const TextLayout& l = text_layout();
    return Size(std::ceil(l.width()) + 32, std::ceil(l.height()) + 16);
}

void Button::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool enabled_now = effectively_enabled;
    Color fg = theme::on_accent();
    if (style_ == ButtonStyle::PRIMARY)
    {
        Color fill = theme::accent();
        if (!enabled_now)
        {
            fill = theme::accent_disabled();
        }
        else if (is_pressed())
        {
            fill = theme::accent_pressed();
        }
        else if (is_hovered())
        {
            fill = theme::accent_hover();
        }
        canvas.fill_round_rect(r, theme::radius(), fill);
    }
    else
    {
        Color fill = enabled_now ? theme::field() : theme::field_disabled();
        if (enabled_now && is_pressed())
        {
            fill = theme::selection_inactive();
        }
        else if (enabled_now && is_hovered())
        {
            fill = theme::selection_inactive().with_alpha(128);
        }
        canvas.fill_round_rect(r, theme::radius(), fill);
        canvas.stroke_round_rect(r, theme::radius(), 1, enabled_now && is_hovered() ? theme::accent() : theme::border());
        fg = enabled_now ? theme::text() : theme::text_disabled();
    }
    if (focused)
    {
        canvas.stroke_round_rect(r.adjusted(2, 2, 2, 2), theme::radius() - 2, 1.5f, theme::focus_ring());
    }

    const TextLayout& l = text_layout();
    canvas.draw_text(l, Point((r.width - l.width()) / 2, (r.height - l.height()) / 2), fg);
    Widget::paint(canvas);
}

}  // namespace tokotachi
