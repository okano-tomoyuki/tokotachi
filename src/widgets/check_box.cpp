#include <tokotachi/widgets/check_box.hpp>

#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/impl.hpp"
#include "theme.hpp"
#include "toggle_painter.hpp"

namespace tokotachi
{

class CheckBox::Impl
{
    TOKOTACHI_IMPL(CheckBox);

    explicit Impl(CheckBox* object) : q(object) {}

    void set_checked(const bool& v);

    bool checked_;
    NotifyHandler on_change_;
};

CheckBox::CheckBox(Widget* parent_widget, const std::string& initial_text)
    : ButtonBase(parent_widget, initial_text),
      TOKOTACHI_IMPL_PROPERTY_MEMBER_SELF_NOTIFY(Impl, bool, checked),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_change),
      d_(new Impl(this))
{
    d_->checked_ = false;
}

CheckBox::~CheckBox() { delete d_; }

void CheckBox::Impl::set_checked(const bool& v)
{
    if (v == checked_)
    {
        return;
    }
    checked_ = v;
    q->invalidate();
    // 結びつけた相手へ伝えてから、ハンドラを呼ぶ。
    q->notify_changed(q->checked);
    Object* self = q;
    detail::fire(on_change_, self);
}

void CheckBox::click()
{
    d_->set_checked(!d_->checked_);
    ButtonBase::click();
}

Size CheckBox::compute_preferred_size() const { return detail::toggle_preferred_size(text_layout()); }

void CheckBox::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const Rect box = detail::toggle_indicator_rect(r);
    const bool enabled_now = effectively_enabled;
    if (d_->checked_)
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
    const Point at = detail::paint_toggle_text(canvas, text_layout(), r, focused, enabled_now);
    paint_mnemonic(canvas, at, enabled_now ? theme::text() : theme::text_disabled());
    Widget::paint(canvas);
}

}  // namespace tokotachi
