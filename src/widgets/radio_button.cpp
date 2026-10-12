#include <tokotachi/widgets/radio_button.hpp>

#include <tokotachi/widgets/draw.hpp>
#include <tokotachi/graphics/canvas.hpp>

#include "../core/impl.hpp"
#include "theme.hpp"
#include "toggle_painter.hpp"

namespace tokotachi
{

class RadioButton::Impl
{
    TOKOTACHI_IMPL(RadioButton);

    explicit Impl(RadioButton* object) : q(object) {}

    void set_value(const int& v);
    void set_group_value(const int& v);
    bool get_checked() const { return value_ == group_value_; }
    /// value_・group_value_ を変えた後に、変わったプロパティを知らせ、選択が変わっていれば on_change を呼ぶ。
    void finish(bool was_checked, const void* changed_property);

    int value_;
    int group_value_;
    NotifyHandler on_change_;
};

RadioButton::RadioButton(Widget* parent_widget, const std::string& initial_text, int initial_value)
    : ButtonBase(parent_widget, initial_text),
      TOKOTACHI_IMPL_PROPERTY_MEMBER_SELF_NOTIFY(Impl, int, value),
      TOKOTACHI_IMPL_PROPERTY_MEMBER_SELF_NOTIFY(Impl, int, group_value),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, bool, checked),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_change),
      d_(new Impl(this))
{
    d_->value_ = initial_value;
    d_->group_value_ = -1;
}

RadioButton::~RadioButton() { delete d_; }

void RadioButton::Impl::set_value(const int& v)
{
    if (v == value_)
    {
        return;
    }
    const bool was_checked = get_checked();
    value_ = v;
    finish(was_checked, &q->value);
}

void RadioButton::Impl::set_group_value(const int& v)
{
    if (v == group_value_)
    {
        return;
    }
    const bool was_checked = get_checked();
    group_value_ = v;
    finish(was_checked, &q->group_value);
}

void RadioButton::Impl::finish(bool was_checked, const void* changed_property)
{
    // 結びつけた相手へ伝えてから、ハンドラを呼ぶ。
    if (changed_property == &q->value)
    {
        q->notify_changed(q->value);
    }
    else
    {
        q->notify_changed(q->group_value);
    }
    if (was_checked == get_checked())
    {
        return;
    }
    q->invalidate();
    q->notify_changed(q->checked);
    Object* self = q;
    detail::fire(on_change_, self);
}

void RadioButton::click()
{
    d_->set_group_value(d_->value_);
    ButtonBase::click();
}

Size RadioButton::compute_preferred_size() const { return detail::toggle_preferred_size(text_layout()); }

void RadioButton::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const Rect box = detail::toggle_indicator_rect(r);
    const bool enabled_now = effectively_enabled;
    ControlState st = state();
    st.checked = d_->get_checked();
    draw::radio_mark(canvas, box, st);
    const Point at = detail::paint_toggle_text(canvas, text_layout(), r, focused, enabled_now);
    paint_mnemonic(canvas, at, enabled_now ? theme::text() : theme::text_disabled());
    Widget::paint(canvas);
}

}  // namespace tokotachi
