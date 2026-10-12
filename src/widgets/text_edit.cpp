#include <tokotachi/widgets/text_edit.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "text_input_impl.hpp"
#include "../core/impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

namespace
{
const float PADDING_Y = 6;
}

class TextEdit::Impl : public detail::TextInputHooks
{
    TOKOTACHI_IMPL(TextEdit);

    explicit Impl(TextEdit* object) : q(object) {}

    Rect text_area() const override;
    float wrap_width() const override;
    void scroll_changed() override;

    bool get_word_wrap() const { return word_wrap_; }
    void set_word_wrap(const bool& v);

    bool word_wrap_;
    ScrollBar* bar_;
};

TextEdit::TextEdit(Widget* parent_widget, const std::string& initial_text)
    : TextInputBase(parent_widget, detail::internal_tag(), initial_text, true),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, word_wrap),
      d_(new Impl(this))
{
    detail::ImplAccess<TextInputBase>::of(this)->input_ = d_;
    d_->word_wrap_ = true;
    d_->bar_ = new ScrollBar(this, Orientation::VERTICAL);
    d_->bar_->visible = false;
    d_->bar_->cursor = MouseCursor::ARROW;
    d_->bar_->on_change = [this](Object*) { detail::ImplAccess<TextInputBase>::of(this)->set_scroll_offset(Point(detail::ImplAccess<TextInputBase>::of(this)->scroll_offset().x, d_->bar_->value.get())); };
}

TextEdit::~TextEdit() { delete d_; }

void TextEdit::Impl::set_word_wrap(const bool& v)
{
    word_wrap_ = v;
    detail::ImplAccess<TextInputBase>::of(q)->set_scroll_offset(Point());
    q->invalidate();
}

Rect TextEdit::Impl::text_area() const
{
    // 右にスクロールバーの幅を常に空けておく(出し入れで折り返しが変わらないように)。
    const Rect r = q->local_rect;
    return r.adjusted(theme::field_padding(), PADDING_Y, theme::field_padding() + ScrollBar::THICKNESS, PADDING_Y);
}

float TextEdit::Impl::wrap_width() const { return word_wrap_ ? std::max(1.0f, text_area().width) : 0; }

void TextEdit::Impl::scroll_changed()
{
    // スクロールバーを、文字の高さ・見えている高さ・スクロールの量に合わせる。
    const Rect area = text_area();
    const float content = detail::ImplAccess<TextInputBase>::of(q)->text_layout().height();
    const float excess = content - area.height;
    bar_->visible = excess > 0;
    bar_->page_step = area.height;
    bar_->maximum = std::max(0.0f, excess);
    bar_->value = detail::ImplAccess<TextInputBase>::of(q)->scroll_offset().y;
}

void TextEdit::arrange_children()
{
    const Rect r = local_rect;
    d_->bar_->bounds = Rect(r.width - ScrollBar::THICKNESS - 2, 2, ScrollBar::THICKNESS, r.height - 4);
    d_->scroll_changed();
}

void TextEdit::paint(Canvas& canvas)
{
    const bool has_focus = focused;
    const Rect r = local_rect;
    canvas.fill_round_rect(r, theme::field_radius(), effectively_enabled ? theme::field() : theme::field_disabled());
    canvas.stroke_round_rect(r, theme::field_radius(), has_focus ? 2.0f : 1.0f,
                             has_focus ? theme::accent() : theme::border());
    detail::ImplAccess<TextInputBase>::of(this)->paint_text(canvas);
    Widget::paint(canvas);
}

void TextEdit::mouse_wheel(WheelEvent& e)
{
    Widget::mouse_wheel(e);
    if (e.accepted)
    {
        return;  // 利用者のハンドラが受け取った
    }
    if (d_->bar_->maximum.get() <= 0)
    {
        return;  // スクロールできなければ親(ScrollArea など)へ送る
    }
    e.accepted = true;
    detail::ImplAccess<TextInputBase>::of(this)->set_scroll_offset(Point(detail::ImplAccess<TextInputBase>::of(this)->scroll_offset().x, detail::ImplAccess<TextInputBase>::of(this)->scroll_offset().y - e.delta.y * d_->bar_->single_step.get()));
}

Size TextEdit::compute_preferred_size() const
{
    const float line = detail::ImplAccess<TextInputBase>::of(this)->text_layout().line_height();
    return Size(240, std::ceil(line * 6) + PADDING_Y * 2);
}

Size TextEdit::compute_minimum_size() const
{
    const float line = detail::ImplAccess<TextInputBase>::of(this)->text_layout().line_height();
    return Size(80, std::ceil(line * 2) + PADDING_Y * 2);
}

}  // namespace tokotachi
