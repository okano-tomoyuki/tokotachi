#include <tokotachi/widgets/button.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

Button::Button(Widget* parent_widget, const std::string& initial_text)
    : ButtonBase(parent_widget, initial_text),
      TOKOTACHI_PROPERTY_MEMBER(Button, ButtonStyle, style),
      TOKOTACHI_PROPERTY_MEMBER(Button, Image, icon),
      TOKOTACHI_PROPERTY_MEMBER(Button, float, icon_size),
      style_(ButtonStyle::PRIMARY),
      icon_size_(16)
{
}

namespace
{
const float ICON_TEXT_GAP = 6;  // 画像と文字の間
}  // namespace

void Button::set_icon(const Image& v)
{
    if (v == icon_)
    {
        return;
    }
    const bool shown = !icon_.is_null();
    icon_ = v;
    invalidate();
    if (shown != !icon_.is_null())
    {
        request_layout();
    }
}

void Button::set_icon_size(const float& v)
{
    if (v == icon_size_)
    {
        return;
    }
    icon_size_ = std::max(0.0f, v);
    invalidate();
    request_layout();
}

float Button::icon_extent() const
{
    if (icon_.is_null())
    {
        return 0;
    }
    return text.get().empty() ? icon_size_ : icon_size_ + ICON_TEXT_GAP;
}

void Button::set_style(const ButtonStyle& v)
{
    style_ = v;
    invalidate();
}

Size Button::compute_preferred_size() const
{
    const TextLayout& l = text_layout();
    const float h = std::ceil(std::max(l.height(), icon_.is_null() ? 0.0f : icon_size_)) + 16;
    if (!icon_.is_null() && text.get().empty())
    {
        return Size(std::max(h, icon_size_ + 16), h);  // 画像だけ: 正方形に近く
    }
    return Size(std::ceil(icon_extent() + l.width()) + 32, h);
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

    // 画像と文字をひとまとまりとして中ほどに置く
    const TextLayout& l = text_layout();
    const float extent = icon_extent();
    const float x = (r.width - extent - (text.get().empty() ? 0 : l.width())) / 2;
    if (!icon_.is_null())
    {
        canvas.draw_image(icon_, Rect(x, (r.height - icon_size_) / 2, icon_size_, icon_size_), enabled_now ? 1.0f : 0.4f);
    }
    canvas.draw_text(l, Point(x + extent, (r.height - l.height()) / 2), fg);
    Widget::paint(canvas);
}

}  // namespace tokotachi
