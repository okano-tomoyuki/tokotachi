#include <tokotachi/widgets/check_box.hpp>

#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"
#include "toggle_painter.hpp"

namespace tokotachi
{

CheckBox::CheckBox(Widget* parent_widget, const std::string& initial_text)
    : ButtonBase(parent_widget, initial_text),
      TOKOTACHI_PROPERTY_MEMBER_SELF_NOTIFY(CheckBox, bool, checked),
      TOKOTACHI_HANDLER_PROPERTY(CheckBox, NotifyHandler, on_change),
      checked_(false)
{
}

void CheckBox::set_checked(const bool& v)
{
    if (v == checked_)
    {
        return;
    }
    checked_ = v;
    invalidate();
    // 結びつけた相手へ伝えてから、ハンドラを呼ぶ。
    notify_changed(checked);
    Object* self = this;
    detail::fire(on_change_, self);
}

void CheckBox::click()
{
    set_checked(!checked_);
    ButtonBase::click();
}

Size CheckBox::compute_preferred_size() const { return detail::toggle_preferred_size(text_layout()); }

void CheckBox::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const Rect box = detail::toggle_indicator_rect(r);
    const bool enabled_now = effectively_enabled;
    if (checked_)
    {
        canvas.fill_round_rect(box, 3, enabled_now ? theme::accent() : theme::accent_disabled());
        // チェックの印
        Path mark;
        mark.move_to(box.x + box.width * 0.22f, box.y + box.height * 0.52f);
        mark.line_to(box.x + box.width * 0.42f, box.y + box.height * 0.72f);
        mark.line_to(box.x + box.width * 0.78f, box.y + box.height * 0.30f);
        canvas.stroke_path(mark, 2, theme::on_accent(), LineCap::ROUND);
    }
    else
    {
        canvas.fill_round_rect(box, 3, enabled_now ? theme::field() : theme::field_disabled());
        canvas.stroke_round_rect(box, 3, 1, enabled_now && is_hovered() ? theme::accent() : theme::border());
    }
    detail::paint_toggle_text(canvas, text_layout(), r, focused, enabled_now);
    Widget::paint(canvas);
}

}  // namespace tokotachi
