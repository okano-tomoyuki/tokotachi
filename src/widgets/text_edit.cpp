#include <tokotachi/widgets/text_edit.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

namespace
{
const float PADDING_Y = 6;
}

class TextEdit::Impl
{
    TOKOTACHI_IMPL(TextEdit);

    explicit Impl(TextEdit* object) : q(object) {}

    bool get_word_wrap() const { return word_wrap_; }
    void set_word_wrap(const bool& v);

    bool word_wrap_;
    ScrollBar* bar_;
};

TextEdit::TextEdit(Widget* parent_widget, const std::string& initial_text)
    : TextInputBase(parent_widget, initial_text, true),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, word_wrap),
      d_(new Impl(this))
{
    d_->word_wrap_ = true;
    d_->bar_ = new ScrollBar(this, Orientation::VERTICAL);
    d_->bar_->visible = false;
    d_->bar_->cursor = MouseCursor::ARROW;
    d_->bar_->on_change = [this](Object*) { set_scroll_offset(Point(scroll_offset().x, d_->bar_->value.get())); };
}

TextEdit::~TextEdit() { delete d_; }

void TextEdit::Impl::set_word_wrap(const bool& v)
{
    word_wrap_ = v;
    q->set_scroll_offset(Point());
    q->invalidate();
}

Rect TextEdit::text_area() const
{
    // 右にスクロールバーの幅を常に空けておく(出し入れで折り返しが変わらないように)。
    const Rect r = local_rect;
    return r.adjusted(theme::field_padding(), PADDING_Y, theme::field_padding() + ScrollBar::THICKNESS, PADDING_Y);
}

float TextEdit::wrap_width() const { return d_->word_wrap_ ? std::max(1.0f, text_area().width) : 0; }

void TextEdit::scroll_changed()
{
    // スクロールバーを、文字の高さ・見えている高さ・スクロールの量に合わせる。
    const Rect area = text_area();
    const float content = text_layout().height();
    const float excess = content - area.height;
    d_->bar_->visible = excess > 0;
    d_->bar_->page_step = area.height;
    d_->bar_->maximum = std::max(0.0f, excess);
    d_->bar_->value = scroll_offset().y;
}

void TextEdit::arrange_children()
{
    const Rect r = local_rect;
    d_->bar_->bounds = Rect(r.width - ScrollBar::THICKNESS - 2, 2, ScrollBar::THICKNESS, r.height - 4);
    scroll_changed();
}

void TextEdit::paint(Canvas& canvas)
{
    const bool has_focus = focused;
    const Rect r = local_rect;
    canvas.fill_round_rect(r, theme::field_radius(), effectively_enabled ? theme::field() : theme::field_disabled());
    canvas.stroke_round_rect(r, theme::field_radius(), has_focus ? 2.0f : 1.0f,
                             has_focus ? theme::accent() : theme::border());
    paint_text(canvas);
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
    set_scroll_offset(Point(scroll_offset().x, scroll_offset().y - e.delta.y * d_->bar_->single_step.get()));
}

Size TextEdit::compute_preferred_size() const
{
    const float line = text_layout().line_height();
    return Size(240, std::ceil(line * 6) + PADDING_Y * 2);
}

Size TextEdit::compute_minimum_size() const
{
    const float line = text_layout().line_height();
    return Size(80, std::ceil(line * 2) + PADDING_Y * 2);
}

}  // namespace tokotachi
