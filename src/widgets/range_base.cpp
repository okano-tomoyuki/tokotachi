#include <gui/range_base.hpp>

#include <algorithm>
#include <cmath>

#include "../core/property_impl.hpp"

namespace gui
{

RangeBase::RangeBase(Widget* parent_widget)
    : Widget(parent_widget),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(RangeBase, double, minimum),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(RangeBase, double, maximum),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(RangeBase, double, value),
      GUI_PROPERTY_GETTER(RangeBase, double, single_step),
      GUI_PROPERTY_GETTER(RangeBase, double, page_step),
      GUI_HANDLER_PROPERTY(RangeBase, NotifyHandler, on_change),
      minimum_(0),
      maximum_(100),
      value_(0),
      single_step_(1),
      page_step_(10)
{
}

void RangeBase::set_minimum(const double& v)
{
    if (v == minimum_ || std::isnan(v))
    {
        return;
    }
    minimum_ = v;
    const bool maximum_moved = maximum_ < v;
    if (maximum_moved)
    {
        maximum_ = v;
    }
    range_changed();
    notify_changed(minimum);
    if (maximum_moved)
    {
        notify_changed(maximum);
    }
    apply_value(value_);
}

void RangeBase::set_maximum(const double& v)
{
    if (v == maximum_ || std::isnan(v))
    {
        return;
    }
    maximum_ = v;
    const bool minimum_moved = minimum_ > v;
    if (minimum_moved)
    {
        minimum_ = v;
    }
    range_changed();
    notify_changed(maximum);
    if (minimum_moved)
    {
        notify_changed(minimum);
    }
    apply_value(value_);
}

void RangeBase::set_value(const double& v)
{
    if (!std::isnan(v))
    {
        apply_value(v);
    }
}

void RangeBase::set_single_step(const double& v) { single_step_ = std::max(0.0, v); }

void RangeBase::set_page_step(const double& v) { page_step_ = std::max(0.0, v); }

void RangeBase::apply_value(double v)
{
    const double clamped = std::max(minimum_, std::min(v, maximum_));
    if (clamped == value_)
    {
        return;
    }
    value_ = clamped;
    range_changed();
    // 結びつけた相手へ伝えてから、ハンドラを呼ぶ。
    notify_changed(value);
    Object* self = this;
    detail::fire(on_change_, self);
}

double RangeBase::snapped(double v) const
{
    if (single_step_ > 0)
    {
        v = minimum_ + std::floor((v - minimum_) / single_step_ + 0.5) * single_step_;
    }
    return std::max(minimum_, std::min(v, maximum_));
}

void RangeBase::step_by(double amount) { apply_value(snapped(value_ + amount)); }

double RangeBase::ratio() const { return maximum_ > minimum_ ? (value_ - minimum_) / (maximum_ - minimum_) : 0.0; }

void RangeBase::range_changed() { invalidate(); }

}  // namespace gui
