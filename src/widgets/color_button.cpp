#include <tokotachi/widgets/color_button.hpp>

#include <cmath>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/windows/color_dialog.hpp>

#include "../core/impl.hpp"
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

class ColorButton::Impl
{
    TOKOTACHI_IMPL(ColorButton);

    explicit Impl(ColorButton* object) : q(object) {}

    Color get_color() const { return color_; }
    void set_color(const Color& v);
    bool get_alpha_enabled() const { return alpha_enabled_; }
    void set_alpha_enabled(const bool& v);
    std::string get_dialog_title() const { return dialog_title_; }
    void set_dialog_title(const std::string& v) { dialog_title_ = v; }
    /// 出す文字(text が空なら 16 進数)。
    std::string label() const;

    Color color_;
    bool alpha_enabled_;
    std::string dialog_title_;
    NotifyHandler on_change_;
};

ColorButton::ColorButton(Widget* parent_widget, const Color& initial)
    : ButtonBase(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, Color, color),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, alpha_enabled),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, std::string, dialog_title),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_change),
      d_(new Impl(this))
{
    d_->color_ = initial;
    d_->alpha_enabled_ = false;
}

ColorButton::~ColorButton() { delete d_; }

void ColorButton::Impl::set_color(const Color& v)
{
    if (v == color_)
    {
        return;
    }
    color_ = v;
    q->invalidate();
    q->request_layout();
    q->notify_changed(q->color);
    Object* self = q;
    detail::fire(on_change_, self);
}

void ColorButton::Impl::set_alpha_enabled(const bool& v)
{
    alpha_enabled_ = v;
    q->invalidate();
    q->request_layout();
}

std::string ColorButton::Impl::label() const
{
    const std::string t = q->text_layout().text();
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
        w, d_->color_,
        [self](Object* sender)
        {
            ColorDialog* d = static_cast<ColorDialog*>(sender);
            if (d->result == DialogButton::OK)
            {
                self->color = d->color.get();
            }
        },
        d_->alpha_enabled_, d_->dialog_title_);
}

Size ColorButton::compute_preferred_size() const
{
    const TextLayout l(d_->label(), font, display_scale);
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
    if (d_->color_.a < 255)
    {
        detail::paint_checker(canvas, swatch, 4);
    }
    canvas.fill_rect(swatch, enabled_now ? d_->color_ : d_->color_.with_alpha(static_cast<std::uint8_t>(d_->color_.a * 0.4f)));
    canvas.stroke_rect(swatch.adjusted(-0.5f, -0.5f, -0.5f, -0.5f), 1, theme::border());
    const TextLayout l(d_->label(), font, display_scale);
    const Point at(swatch.right() + 8, (r.height - l.height()) / 2);
    const Color fg = enabled_now ? theme::text() : theme::text_disabled();
    canvas.draw_text(l, at, fg);
    paint_mnemonic(canvas, at, fg);
    Widget::paint(canvas);
}

}  // namespace tokotachi
