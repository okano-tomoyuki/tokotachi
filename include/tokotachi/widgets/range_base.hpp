/// @file
/// tokotachi::RangeBase の宣言。
#ifndef TOKOTACHI_WIDGETS_RANGE_BASE_HPP
#define TOKOTACHI_WIDGETS_RANGE_BASE_HPP

#include "widget.hpp"

namespace tokotachi
{

/// 範囲の中の値を持ち、利用者が動かせるウィジェットの共通の基底(Slider・SpinBox。docs/adr/0024)。
///
/// @code
/// slider_volume->minimum = 0;
/// slider_volume->maximum = 100;
/// slider_volume->value = 25;
/// tokotachi::bind(slider_volume->value, volume->value);  // DoubleVariable と結ぶ
/// @endcode
///
/// @ingroup widgets_base
class RangeBase : public Widget
{
public:
    /// 範囲(minimum ≦ value ≦ maximum。既定 0〜100)。minimum を maximum より大きくすると maximum も同じ値にする(逆も同様)。
    Property<double> minimum;
    Property<double> maximum;
    /// 値(既定 0)。範囲の外は丸める。代入した値は single_step に合わせない(合わせるのは利用者の操作だけ)。
    Property<double> value;
    /// 矢印キー・ホイールで動く量(既定 1)。0 より大きければ、利用者の操作による値を minimum + n × single_step に合わせる。
    Property<double> single_step;
    /// Page Up・Page Down で動く量(既定 10)。
    Property<double> page_step;

    /// value が変わった(利用者の操作・代入のどちらでも)。
    Property<NotifyHandler> on_change;

protected:
    explicit RangeBase(Widget* parent);

    /// 値を amount だけ動かし、single_step に合わせる(利用者の操作)。
    void step_by(double amount);
    /// v を single_step に合わせ、範囲に収めた値。
    double snapped(double v) const;
    /// value の範囲の中の位置(0〜1。minimum == maximum なら 0)。
    double ratio() const;
    /// 範囲・値が変わった(既定の実装は描き直す)。変更を知らせる前に呼ぶ。
    virtual void range_changed();

private:
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

}  // namespace tokotachi

#endif
