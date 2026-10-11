#include <tokotachi/widgets/color_button.hpp>

#include <cmath>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/windows/color_dialog.hpp>

#include "../core/property_impl.hpp"
#include "color_util.hpp"
#include "theme.hpp"

namespace tokotachi
{

namespace
{
const float SWATCH_W = 24;  // 色の見本の大きさ
const float SWATCH_H = 16;
const float PADDING = 8;
}  // namespace

ColorButton::ColorButton(Widget* parent_widget, const Color& initial)
    : ButtonBase(parent_widget),
      TOKOTACHI_PROPERTY_GETTER_SELF_NOTIFY(ColorButton, Color, color),
      TOKOTACHI_PROPERTY_GETTER(ColorButton, bool, alpha_enabled),
      TOKOTACHI_PROPERTY_GETTER(ColorButton, std::string, dialog_title),
      TOKOTACHI_HANDLER_PROPERTY(ColorButton, NotifyHandler, on_change),
      color_(initial),
      alpha_enabled_(false)
{
}

void ColorButton::set_color(const Color& v)
{
    if (v == color_)
    {
        return;
    }
    color_ = v;
    invalidate();
    request_layout();
    notify_changed(color);
    Object* self = this;
    detail::fire(on_change_, self);
}

void ColorButton::set_alpha_enabled(const bool& v)
{
    alpha_enabled_ = v;
    invalidate();
    request_layout();
}

std::string ColorButton::label() const
{
    const std::string t = text_layout().text();
    return t.empty() ? detail::color_to_hex(color_, alpha_enabled_ || color_.a != 255) : t;
}

void ColorButton::click()
{
    open_dialog();
    ButtonBase::click();
}

void ColorButton::open_dialog()
{
    Window* w = window;
    if (!w)
    {
        return;
    }
    ColorButton* self = this;
    ColorDialog::show(
        w, color_,
        [self](Object* sender)
        {
            ColorDialog* d = static_cast<ColorDialog*>(sender);
            if (d->result == DialogButton::OK)
            {
                self->color = d->color.get();
            }
        },
        alpha_enabled_, dialog_title_);
}

Size ColorButton::compute_preferred_size() const
{
    const TextLayout l(label(), font, display_scale);
    return Size(std::ceil(PADDING + SWATCH_W + 8 + l.width() + PADDING + 4), std::ceil(l.height()) + 16);
}

void ColorButton::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool enabled_now = effectively_enabled;
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
    if (focused)
    {
        canvas.stroke_round_rect(r.adjusted(2, 2, 2, 2), theme::radius() - 2, 1.5f, theme::focus_ring());
    }
    // 色の見本(透過は市松模様の上に)
    const Rect swatch(PADDING, std::floor((r.height - SWATCH_H) / 2), SWATCH_W, SWATCH_H);
    if (color_.a < 255)
    {
        detail::paint_checker(canvas, swatch, 4);
    }
    canvas.fill_rect(swatch, enabled_now ? color_ : color_.with_alpha(static_cast<std::uint8_t>(color_.a * 0.4f)));
    canvas.stroke_rect(swatch.adjusted(-0.5f, -0.5f, -0.5f, -0.5f), 1, theme::border());
    const TextLayout l(label(), font, display_scale);
    const Point at(swatch.right() + 8, (r.height - l.height()) / 2);
    const Color fg = enabled_now ? theme::text() : theme::text_disabled();
    canvas.draw_text(l, at, fg);
    paint_mnemonic(canvas, at, fg);
    Widget::paint(canvas);
}

}  // namespace tokotachi
