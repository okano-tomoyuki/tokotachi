#include <tokotachi/widgets/draw.hpp>

#include "theme.hpp"

namespace tokotachi
{
namespace draw
{

void button_face(Canvas& canvas, const Rect& rect, const ControlState& state, ButtonStyle style)
{
    if (style == ButtonStyle::PRIMARY)
    {
        Color fill = theme::accent();
        if (!state.enabled)
        {
            fill = theme::accent_disabled();
        }
        else if (state.pressed)
        {
            fill = theme::accent_pressed();
        }
        else if (state.hovered)
        {
            fill = theme::accent_hover();
        }
        canvas.fill_round_rect(rect, theme::radius(), fill);
        return;
    }
    Color fill = state.enabled ? theme::field() : theme::field_disabled();
    if (state.enabled && state.pressed)
    {
        fill = theme::selection_inactive();
    }
    else if (state.enabled && state.hovered)
    {
        fill = theme::selection_inactive().with_alpha(128);
    }
    canvas.fill_round_rect(rect, theme::radius(), fill);
    canvas.stroke_round_rect(rect, theme::radius(), 1, state.enabled && state.hovered ? theme::accent() : theme::border());
}

Color button_text_color(const ControlState& state, ButtonStyle style)
{
    if (style == ButtonStyle::PRIMARY)
    {
        return theme::on_accent();
    }
    return state.enabled ? theme::text() : theme::text_disabled();
}

void check_mark(Canvas& canvas, const Rect& rect, const ControlState& state)
{
    if (state.checked)
    {
        canvas.fill_round_rect(rect, 3, state.enabled ? theme::accent() : theme::accent_disabled());
        Path mark;
        mark.move_to(rect.x + rect.width * 0.22f, rect.y + rect.height * 0.52f);
        mark.line_to(rect.x + rect.width * 0.42f, rect.y + rect.height * 0.72f);
        mark.line_to(rect.x + rect.width * 0.78f, rect.y + rect.height * 0.30f);
        canvas.stroke_path(mark, 2, theme::on_accent(), LineCap::ROUND);
        return;
    }
    canvas.fill_round_rect(rect, 3, state.enabled ? theme::field() : theme::field_disabled());
    canvas.stroke_round_rect(rect, 3, 1, state.enabled && state.hovered ? theme::accent() : theme::border());
}

void radio_mark(Canvas& canvas, const Rect& rect, const ControlState& state)
{
    const Point c = rect.center();
    const float radius = rect.width / 2;
    Path outer;
    outer.add_ellipse(c, radius, radius);
    if (state.checked)
    {
        canvas.fill_path(outer, state.enabled ? theme::accent() : theme::accent_disabled());
        Path dot;
        dot.add_ellipse(c, radius * 0.4f, radius * 0.4f);
        canvas.fill_path(dot, theme::on_accent());
        return;
    }
    canvas.fill_path(outer, state.enabled ? theme::field() : theme::field_disabled());
    Path ring;
    ring.add_ellipse(c, radius - 0.5f, radius - 0.5f);
    canvas.stroke_path(ring, 1, state.enabled && state.hovered ? theme::accent() : theme::border());
}

void focus_ring(Canvas& canvas, const Rect& rect)
{
    canvas.stroke_round_rect(rect.adjusted(2, 2, 2, 2), theme::radius() - 2, 1.5f, theme::focus_ring());
}

void field_frame(Canvas& canvas, const Rect& rect, const ControlState& state)
{
    canvas.fill_round_rect(rect, theme::field_radius(), state.enabled ? theme::field() : theme::field_disabled());
    canvas.stroke_round_rect(rect, theme::field_radius(), state.focused ? 2.0f : 1.0f,
                             state.focused ? theme::accent() : theme::border());
}

}  // namespace draw
}  // namespace tokotachi
