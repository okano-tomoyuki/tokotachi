#include <tokotachi/widgets/button.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

namespace
{
const float ICON_TEXT_GAP = 6;  // 画像と文字の間
}  // namespace

class Button::Impl
{
    TOKOTACHI_IMPL(Button);

    explicit Impl(Button* button) : q(button), style_(ButtonStyle::PRIMARY), icon_size_(16) {}

    void set_style(const ButtonStyle& v)
    {
        style_ = v;
        q->invalidate();
    }

    void set_icon(const Image& v)
    {
        if (v == icon_)
        {
            return;
        }
        const bool shown = !icon_.is_null();
        icon_ = v;
        q->invalidate();
        if (shown != !icon_.is_null())
        {
            q->request_layout();
        }
    }

    void set_icon_size(const float& v)
    {
        if (v == icon_size_)
        {
            return;
        }
        icon_size_ = std::max(0.0f, v);
        q->invalidate();
        q->request_layout();
    }

    /// 画像と、文字との間の幅(画像が無ければ 0)。
    float icon_extent() const
    {
        if (icon_.is_null())
        {
            return 0;
        }
        return q->text_layout().text().empty() ? icon_size_ : icon_size_ + ICON_TEXT_GAP;
    }

    ButtonStyle style_;
    Image icon_;
    float icon_size_;
};

Button::Button(Widget* parent_widget, const std::string& initial_text)
    : ButtonBase(parent_widget, initial_text),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, ButtonStyle, style),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, Image, icon),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, float, icon_size),
      d_(new Impl(this))
{
}

Button::~Button() { delete d_; }

Size Button::compute_preferred_size() const
{
    const TextLayout& l = text_layout();
    const float h = std::ceil(std::max(l.height(), d_->icon_.is_null() ? 0.0f : d_->icon_size_)) + 16;
    if (!d_->icon_.is_null() && l.text().empty())
    {
        return Size(std::max(h, d_->icon_size_ + 16), h);  // 画像だけ: 正方形に近く
    }
    return Size(std::ceil(d_->icon_extent() + l.width()) + 32, h);
}

void Button::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool enabled_now = effectively_enabled;
    Color fg = theme::on_accent();
    if (d_->style_ == ButtonStyle::PRIMARY)
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
    const float extent = d_->icon_extent();
    const float x = (r.width - extent - (l.text().empty() ? 0 : l.width())) / 2;
    if (!d_->icon_.is_null())
    {
        canvas.draw_image(d_->icon_, Rect(x, (r.height - d_->icon_size_) / 2, d_->icon_size_, d_->icon_size_), fg,
                          enabled_now || d_->icon_.is_icon() ? 1.0f : 0.4f);
    }
    const Point at(x + extent, (r.height - l.height()) / 2);
    canvas.draw_text(l, at, fg);
    paint_mnemonic(canvas, at, fg);
    Widget::paint(canvas);
}

}  // namespace tokotachi
