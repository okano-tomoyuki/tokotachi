#include <gui/button.hpp>

#include <cmath>

#include <gui/canvas.hpp>

#include "theme.hpp"

namespace gui
{

Button::Button(Widget* parent_widget, const std::string& initial_text) : ButtonBase(parent_widget, initial_text) {}

Size Button::compute_preferred_size() const
{
    const TextLayout& l = text_layout();
    return Size(std::ceil(l.width()) + 32, std::ceil(l.height()) + 16);
}

void Button::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    Color fill = theme::accent();
    if (!effectively_enabled)
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
    canvas.fill_round_rect(r, theme::RADIUS, fill);
    if (focused)
    {
        canvas.stroke_round_rect(r.adjusted(2, 2, 2, 2), theme::RADIUS - 2, 1.5f, theme::focus_ring());
    }

    const TextLayout& l = text_layout();
    canvas.draw_text(l, Point((r.width - l.width()) / 2, (r.height - l.height()) / 2), theme::on_accent());
    Widget::paint(canvas);
}

}  // namespace gui
