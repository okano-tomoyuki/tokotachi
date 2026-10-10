#include <tokotachi/widgets/line_edit.hpp>

#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

LineEdit::LineEdit(Widget* parent_widget, const std::string& initial_text)
    : TextInputBase(parent_widget, initial_text, false), TOKOTACHI_HANDLER_PROPERTY(LineEdit, NotifyHandler, on_submit)
{
}

Rect LineEdit::text_area() const
{
    const Rect r = local_rect;
    return r.adjusted(theme::field_padding(), 0, theme::field_padding(), 0);
}

bool LineEdit::submit()
{
    if (!on_submit_)
    {
        return false;
    }
    Object* self = this;
    detail::fire(on_submit_, self);
    return true;
}

Size LineEdit::compute_preferred_size() const { return Size(160, std::ceil(text_layout().height()) + 16); }

// 横は狭くしてよい(スクロールする)。縦は文字が切れないようにする。
Size LineEdit::compute_minimum_size() const { return Size(48, compute_preferred_size().height); }

void LineEdit::paint(Canvas& canvas)
{
    const bool has_focus = focused;
    const Rect r = local_rect;
    canvas.fill_round_rect(r, theme::field_radius(), effectively_enabled ? theme::field() : theme::field_disabled());
    canvas.stroke_round_rect(r, theme::field_radius(), has_focus ? 2.0f : 1.0f,
                             has_focus ? theme::accent() : theme::border());
    paint_text(canvas);
    Widget::paint(canvas);
}

}  // namespace tokotachi
