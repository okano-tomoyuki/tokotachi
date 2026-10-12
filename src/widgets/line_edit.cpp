#include <tokotachi/widgets/line_edit.hpp>

#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "text_input_impl.hpp"
#include "../core/impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

class LineEdit::Impl : public detail::TextInputHooks
{
    TOKOTACHI_IMPL(LineEdit);

    explicit Impl(LineEdit* object) : q(object) {}

    Rect text_area() const override;
    bool submit() override;

    NotifyHandler on_submit_;
};

LineEdit::LineEdit(Widget* parent_widget, const std::string& initial_text)
    : TextInputBase(parent_widget, detail::internal_tag(), initial_text, false), TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_submit),
      d_(new Impl(this))
{
    detail::ImplAccess<TextInputBase>::of(this)->input_ = d_;
}

LineEdit::~LineEdit() { delete d_; }

Rect LineEdit::Impl::text_area() const
{
    const Rect r = q->local_rect;
    return r.adjusted(theme::field_padding(), 0, theme::field_padding(), 0);
}

bool LineEdit::Impl::submit()
{
    if (!on_submit_)
    {
        return false;
    }
    Object* self = q;
    detail::fire(on_submit_, self);
    return true;
}

Size LineEdit::compute_preferred_size() const { return Size(160, std::ceil(detail::ImplAccess<TextInputBase>::of(this)->text_layout().height()) + 16); }

// 横は狭くしてよい(スクロールする)。縦は文字が切れないようにする。
Size LineEdit::compute_minimum_size() const { return Size(48, compute_preferred_size().height); }

void LineEdit::paint(Canvas& canvas)
{
    const bool has_focus = focused;
    const Rect r = local_rect;
    canvas.fill_round_rect(r, theme::field_radius(), effectively_enabled ? theme::field() : theme::field_disabled());
    canvas.stroke_round_rect(r, theme::field_radius(), has_focus ? 2.0f : 1.0f,
                             has_focus ? theme::accent() : theme::border());
    detail::ImplAccess<TextInputBase>::of(this)->paint_text(canvas);
    Widget::paint(canvas);
}

}  // namespace tokotachi
