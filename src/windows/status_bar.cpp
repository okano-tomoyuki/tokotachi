#include <tokotachi/windows/status_bar.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../widgets/widget_access.hpp"
#include "../core/impl.hpp"
#include "../widgets/theme.hpp"

namespace tokotachi
{

namespace
{
const float PADDING_X = 10;  // 左右の余白(dp)
const float PADDING_Y = 4;   // 上下の余白
const float GAP = 12;        // 部品の間(区切りの線を挟む)
}  // namespace

class StatusBar::Impl : public detail::WidgetHooks
{
    TOKOTACHI_IMPL(StatusBar);

    explicit Impl(StatusBar* object) : q(object) {}

    // 内部のフック(docs/adr/0058)
    void child_removed(Widget* child) override;

    void set_text(const std::string& v);
    std::string get_message() const { return message_; }
    const TextLayout& shown_layout() const;

    std::string text_;
    std::string message_;
    std::vector<Widget*> widgets_;
    Timer timer_;
    mutable TextLayout layout_;
};

StatusBar::StatusBar(Widget* parent_widget)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, text),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::string, message),
      d_(new Impl(this))
{
    d_->install(this);
    d_->timer_.repeat = false;
    d_->timer_.on_timeout = [this](Object*) { clear_message(); };
}

StatusBar::~StatusBar() { delete d_; }

void StatusBar::Impl::set_text(const std::string& v)
{
    text_ = v;
    layout_ = TextLayout();
    q->invalidate();
}

void StatusBar::show_message(const std::string& msg, int timeout_ms)
{
    d_->timer_.enabled = false;
    const bool changed = msg != d_->message_;
    d_->message_ = msg;
    d_->layout_ = TextLayout();
    invalidate();
    if (changed)
    {
        notify_changed(message);
    }
    if (timeout_ms > 0)
    {
        d_->timer_.interval = timeout_ms;
        d_->timer_.enabled = true;
    }
}

void StatusBar::clear_message() { show_message(std::string()); }

void StatusBar::add_widget(Widget* widget)
{
    if (!widget)
    {
        return;
    }
    widget->parent = this;
    if (widget->parent.get() != this)
    {
        return;
    }
    d_->widgets_.push_back(widget);
    request_layout();
}

void StatusBar::Impl::child_removed(Widget* child)
{
    widgets_.erase(std::remove(widgets_.begin(), widgets_.end(), child), widgets_.end());
    q->request_layout();
}

void StatusBar::scale_changed() { d_->layout_ = TextLayout(); }

const TextLayout& StatusBar::Impl::shown_layout() const
{
    if (layout_.is_null() || layout_.scale() != q->display_scale.get())
    {
        layout_ = TextLayout(message_.empty() ? text_ : message_, Font(), q->display_scale);
    }
    return layout_;
}

Size StatusBar::compute_preferred_size() const
{
    float w = PADDING_X * 2 + std::ceil(d_->shown_layout().width());
    float h = std::ceil(TextLayout("Ag", Font(), display_scale).height());
    for (std::size_t i = 0; i < d_->widgets_.size(); ++i)
    {
        const Size s = d_->widgets_[i]->preferred_size;
        w += GAP + s.width;
        h = std::max(h, s.height);
    }
    return Size(w, h + PADDING_Y * 2);
}

Size StatusBar::compute_minimum_size() const { return Size(0, compute_preferred_size().height); }

void StatusBar::arrange_children()
{
    // 部品を右から並べる(加えた順が左から)。
    const Rect r = local_rect;
    const float inner_h = std::max(0.0f, r.height - PADDING_Y * 2);
    float x = r.width - PADDING_X;
    for (std::size_t i = d_->widgets_.size(); i-- > 0;)
    {
        Widget* w = d_->widgets_[i];
        const Size s = w->preferred_size;
        x -= s.width;
        const float h = std::min(s.height, inner_h);
        w->bounds = Rect(x, PADDING_Y + (inner_h - h) / 2, s.width, h);
        x -= GAP;
    }
    invalidate();
}

void StatusBar::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    canvas.fill_rect(r, theme::window());
    canvas.fill_rect(Rect(0, 0, r.width, 1), theme::border());
    // 文字は、部品の左までに切る。
    float right = r.width - PADDING_X;
    for (std::size_t i = 0; i < d_->widgets_.size(); ++i)
    {
        if (d_->widgets_[i]->visible)
        {
            right = std::min(right, d_->widgets_[i]->x.get() - GAP);
            canvas.fill_rect(Rect(std::floor(d_->widgets_[i]->x.get() - GAP / 2), PADDING_Y, 1, r.height - PADDING_Y * 2),
                             theme::border());
        }
    }
    const TextLayout& l = d_->shown_layout();
    canvas.save();
    canvas.clip_rect(Rect(PADDING_X, 0, std::max(0.0f, right - PADDING_X), r.height));
    canvas.draw_text(l, Point(PADDING_X, (r.height - l.height()) / 2),
                     d_->message_.empty() ? theme::color(ColorRole::TEXT_SECONDARY) : theme::text());
    canvas.restore();
    Widget::paint(canvas);
}

}  // namespace tokotachi
