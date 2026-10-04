#include <gui/widgets/button_base.hpp>

#include "../core/property_impl.hpp"

namespace gui
{

ButtonBase::ButtonBase(Widget* parent_widget, const std::string& initial_text)
    : Widget(parent_widget),
      GUI_PROPERTY_MEMBER(ButtonBase, std::string, text),
      GUI_PROPERTY_MEMBER(ButtonBase, Font, font),
      GUI_HANDLER_PROPERTY(ButtonBase, NotifyHandler, on_click),
      text_(initial_text),
      hovered_(false),
      pressed_(false),
      pressed_in_(false)
{
    focus_policy = FocusPolicy::TAB_AND_CLICK;
    cursor = MouseCursor::POINTING_HAND;
}

void ButtonBase::set_text(const std::string& v)
{
    if (v == text_)
    {
        return;
    }
    text_ = v;
    layout_ = TextLayout();
    invalidate();
    request_layout();
}

void ButtonBase::set_font(const Font& v)
{
    if (v == font_)
    {
        return;
    }
    font_ = v;
    layout_ = TextLayout();
    invalidate();
    request_layout();
}

const TextLayout& ButtonBase::text_layout() const
{
    const float scale = display_scale;
    if (layout_.is_null() || layout_.scale() != scale)
    {
        layout_ = TextLayout(text_, font_, scale);
    }
    return layout_;
}

void ButtonBase::click()
{
    Object* self = this;
    detail::fire(on_click_, self);
}

void ButtonBase::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    e.accepted = true;
    pressed_ = pressed_in_ = true;
    invalidate();
}

void ButtonBase::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    if (!pressed_)
    {
        return;
    }
    const bool in = local_rect.get().contains(e.pos);
    if (in != pressed_in_)
    {
        pressed_in_ = in;
        invalidate();
    }
}

void ButtonBase::mouse_up(MouseEvent& e)
{
    Widget::mouse_up(e);
    if (e.button != MouseButton::LEFT || !pressed_)
    {
        return;
    }
    e.accepted = true;
    const bool inside = local_rect.get().contains(e.pos);
    pressed_ = pressed_in_ = false;
    invalidate();
    // ハンドラの中でこのウィジェットが破棄されてもよいよう、最後に呼ぶ。
    if (inside)
    {
        click();
    }
}

void ButtonBase::mouse_enter()
{
    hovered_ = true;
    invalidate();
    Widget::mouse_enter();
}

void ButtonBase::mouse_leave()
{
    hovered_ = false;
    invalidate();
    Widget::mouse_leave();
}

void ButtonBase::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    if ((e.key == Key::SPACE || e.key == Key::RETURN) && !e.repeat)
    {
        e.accepted = true;
        click();
    }
}

void ButtonBase::focus_in()
{
    invalidate();
    Widget::focus_in();
}

void ButtonBase::focus_out()
{
    invalidate();
    Widget::focus_out();
}

void ButtonBase::scale_changed() { layout_ = TextLayout(); }

Size ButtonBase::compute_minimum_size() const { return compute_preferred_size(); }

}  // namespace gui
