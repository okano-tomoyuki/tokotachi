#include <gui/line_edit.hpp>

#include <cmath>

#include <gui/canvas.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace gui
{

LineEdit::LineEdit(Widget* parent_widget, const std::string& initial_text)
    : TextInputBase(parent_widget, initial_text, false), GUI_HANDLER_PROPERTY(LineEdit, NotifyHandler, on_submit)
{
}

Rect LineEdit::text_area() const
{
    const Rect r = local_rect;
    return r.adjusted(theme::FIELD_PADDING_X, 0, theme::FIELD_PADDING_X, 0);
}

void LineEdit::submit()
{
    Object* self = this;
    detail::fire(on_submit_, self);
}

Size LineEdit::compute_preferred_size() const { return Size(160, std::ceil(text_layout().height()) + 16); }

// 横は狭くしてよい(スクロールする)。縦は文字が切れないようにする。
Size LineEdit::compute_minimum_size() const { return Size(48, compute_preferred_size().height); }

void LineEdit::paint(Canvas& canvas)
{
    const bool has_focus = focused;
    const Rect r = local_rect;
    canvas.fill_round_rect(r, theme::FIELD_RADIUS, effectively_enabled ? theme::field() : theme::field_disabled());
    canvas.stroke_round_rect(r, theme::FIELD_RADIUS, has_focus ? 2.0f : 1.0f,
                             has_focus ? theme::accent() : theme::border());
    paint_text(canvas);
    Widget::paint(canvas);
}

}  // namespace gui
