#include <tokotachi/widgets/button_base.hpp>

#include "widget_access.hpp"
#include "../actions/mnemonic.hpp"
#include "../core/impl.hpp"

namespace tokotachi
{

class ButtonBase::Impl : public detail::WidgetHooks
{
    TOKOTACHI_IMPL(ButtonBase);

    explicit Impl(ButtonBase* object) : q(object) {}

    // 内部のフック(docs/adr/0058)
    Key mnemonic_key() const override;
    void mnemonic_activated() override;

    void set_text(const std::string& v);
    void set_font(const Font& v);

    std::string text_;
    std::string display_;          ///< text_ から & を除いた、表示する文字
    std::size_t mnemonic_offset_;  ///< display_ の中のニーモニックの文字の位置(無ければ npos)
    Key mnemonic_;
    Font font_;
    NotifyHandler on_click_;
    bool hovered_;
    bool pressed_;     ///< 左ボタンを押している(外に出ても続く)
    bool pressed_in_;  ///< 押していて、ポインタが上にある
    mutable TextLayout layout_;
};

ButtonBase::ButtonBase(Widget* parent_widget, const std::string& initial_text)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, text),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, Font, font),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_click),
      d_(new Impl(this))
{
    d_->install(this);
    d_->text_ = initial_text;
    d_->mnemonic_offset_ = std::string::npos;
    d_->mnemonic_ = Key::UNKNOWN;
    d_->hovered_ = false;
    d_->pressed_ = false;
    d_->pressed_in_ = false;
    focus_policy = FocusPolicy::TAB_AND_CLICK;
    cursor = MouseCursor::POINTING_HAND;
    const detail::Mnemonic m = detail::parse_mnemonic(d_->text_);
    d_->display_ = m.text;
    d_->mnemonic_offset_ = m.offset;
    d_->mnemonic_ = m.key;
}

ButtonBase::~ButtonBase() { delete d_; }

bool ButtonBase::is_hovered() const { return d_->hovered_; }

bool ButtonBase::is_pressed() const { return d_->pressed_in_; }

ControlState ButtonBase::state() const
{
    ControlState s;
    s.enabled = effectively_enabled;
    s.hovered = d_->hovered_;
    s.pressed = d_->pressed_in_;
    s.focused = focused;
    return s;
}

void ButtonBase::Impl::set_text(const std::string& v)
{
    if (v == text_)
    {
        return;
    }
    text_ = v;
    const detail::Mnemonic m = detail::parse_mnemonic(text_);
    display_ = m.text;
    mnemonic_offset_ = m.offset;
    mnemonic_ = m.key;
    layout_ = TextLayout();
    q->invalidate();
    q->request_layout();
}

void ButtonBase::Impl::set_font(const Font& v)
{
    if (v == font_)
    {
        return;
    }
    font_ = v;
    layout_ = TextLayout();
    q->invalidate();
    q->request_layout();
}

const TextLayout& ButtonBase::text_layout() const
{
    const float scale = display_scale;
    if (d_->layout_.is_null() || d_->layout_.scale() != scale)
    {
        d_->layout_ = TextLayout(d_->display_, d_->font_, scale);
    }
    return d_->layout_;
}

void ButtonBase::paint_mnemonic(Canvas& canvas, const Point& at, const Color& color) const
{
    if (detail::WidgetAccess::mnemonic_cues(this))
    {
        detail::paint_mnemonic_underline(canvas, text_layout(), at, d_->mnemonic_offset_, color);
    }
}

Key ButtonBase::Impl::mnemonic_key() const { return mnemonic_; }

void ButtonBase::Impl::mnemonic_activated()
{
    q->focused = true;
    q->click();
}

void ButtonBase::click()
{
    Object* self = this;
    detail::fire(d_->on_click_, self);
}

void ButtonBase::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    e.accepted = true;
    d_->pressed_ = d_->pressed_in_ = true;
    invalidate();
}

void ButtonBase::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    if (!d_->pressed_)
    {
        return;
    }
    const bool in = local_rect.get().contains(e.pos);
    if (in != d_->pressed_in_)
    {
        d_->pressed_in_ = in;
        invalidate();
    }
}

void ButtonBase::mouse_up(MouseEvent& e)
{
    Widget::mouse_up(e);
    if (e.button != MouseButton::LEFT || !d_->pressed_)
    {
        return;
    }
    e.accepted = true;
    const bool inside = local_rect.get().contains(e.pos);
    d_->pressed_ = d_->pressed_in_ = false;
    invalidate();
    // ハンドラの中でこのウィジェットが破棄されてもよいよう、最後に呼ぶ。
    if (inside)
    {
        click();
    }
}

void ButtonBase::mouse_enter()
{
    d_->hovered_ = true;
    invalidate();
    Widget::mouse_enter();
}

void ButtonBase::mouse_leave()
{
    d_->hovered_ = false;
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

void ButtonBase::scale_changed() { d_->layout_ = TextLayout(); }

Size ButtonBase::compute_minimum_size() const { return compute_preferred_size(); }

}  // namespace tokotachi
