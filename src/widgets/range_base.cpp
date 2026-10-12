#include <tokotachi/widgets/range_base.hpp>

#include <algorithm>
#include <cmath>

#include "../core/impl.hpp"

namespace tokotachi
{

class RangeBase::Impl
{
    TOKOTACHI_IMPL(RangeBase);

    explicit Impl(RangeBase* object) : q(object) {}

    double get_minimum() const { return minimum_; }
    void set_minimum(const double& v);
    double get_maximum() const { return maximum_; }
    void set_maximum(const double& v);
    double get_value() const { return value_; }
    void set_value(const double& v);
    double get_single_step() const { return single_step_; }
    void set_single_step(const double& v);
    double get_page_step() const { return page_step_; }
    void set_page_step(const double& v);

    /// 範囲に収めた値を value_ にする。変わったら range_changed を呼び、知らせ、on_change を呼ぶ。
    void apply_value(double v);

    double minimum_;
    double maximum_;
    double value_;
    double single_step_;
    double page_step_;
    NotifyHandler on_change_;
};

RangeBase::RangeBase(Widget* parent_widget)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, double, minimum),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, double, maximum),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, double, value),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, double, single_step),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, double, page_step),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_change),
      d_(new Impl(this))
{
    d_->minimum_ = 0;
    d_->maximum_ = 100;
    d_->value_ = 0;
    d_->single_step_ = 1;
    d_->page_step_ = 10;
}

RangeBase::~RangeBase() { delete d_; }

void RangeBase::Impl::set_minimum(const double& v)
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
    q->range_changed();
    q->notify_changed(q->minimum);
    if (maximum_moved)
    {
        q->notify_changed(q->maximum);
    }
    apply_value(value_);
}

void RangeBase::Impl::set_maximum(const double& v)
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
    q->range_changed();
    q->notify_changed(q->maximum);
    if (minimum_moved)
    {
        q->notify_changed(q->minimum);
    }
    apply_value(value_);
}

void RangeBase::Impl::set_value(const double& v)
{
    if (!std::isnan(v))
    {
        apply_value(v);
    }
}

void RangeBase::Impl::set_single_step(const double& v) { single_step_ = std::max(0.0, v); }

void RangeBase::Impl::set_page_step(const double& v) { page_step_ = std::max(0.0, v); }

void RangeBase::Impl::apply_value(double v)
{
    const double clamped = std::max(minimum_, std::min(v, maximum_));
    if (clamped == value_)
    {
        return;
    }
    value_ = clamped;
    q->range_changed();
    // 結びつけた相手へ伝えてから、ハンドラを呼ぶ。
    q->notify_changed(q->value);
    Object* self = q;
    detail::fire(on_change_, self);
}

double RangeBase::snapped(double v) const
{
    if (d_->single_step_ > 0)
    {
        v = d_->minimum_ + std::floor((v - d_->minimum_) / d_->single_step_ + 0.5) * d_->single_step_;
    }
    return std::max(d_->minimum_, std::min(v, d_->maximum_));
}

void RangeBase::step_by(double amount) { d_->apply_value(snapped(d_->value_ + amount)); }

double RangeBase::ratio() const { return d_->maximum_ > d_->minimum_ ? (d_->value_ - d_->minimum_) / (d_->maximum_ - d_->minimum_) : 0.0; }

void RangeBase::range_changed() { invalidate(); }

}  // namespace tokotachi
