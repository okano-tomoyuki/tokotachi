/// @file
/// tokotachi::Slider の宣言。
#ifndef TOKOTACHI_WIDGETS_SLIDER_HPP
#define TOKOTACHI_WIDGETS_SLIDER_HPP

#include "../core/export.hpp"
#include "../layout/alignment.hpp"
#include "range_base.hpp"

namespace tokotachi
{

/// スライダー: つまみを動かして、範囲の中の値を選ぶ(docs/adr/0024)。縦のときは下が minimum。
///
/// @image html widget_slider.png
///
/// @code
/// tokotachi::Slider* slider_volume = new tokotachi::Slider(panel);
/// slider_volume->maximum = 100;
/// slider_volume->tick_interval = 10;  // 10 ごとに目盛りを出す
/// slider_volume->on_change = [=](tokotachi::Object*) { ... slider_volume->value ... };
/// @endcode
///
/// 溝を押すとその位置へ動き、そのままドラッグできる。矢印キー(single_step)・Page Up/Down(page_step)・Home/End で動かし、
/// フォーカスがあるときはホイールでも動かす(フォームをスクロールしているときに、うっかり値を変えないように)。
///
/// @ingroup widgets_input
class TOKOTACHI_API Slider : public RangeBase
{
public:
    /// parent の子として作る(parent が所有する)。
    explicit Slider(Widget* parent = nullptr, Orientation orientation = Orientation::HORIZONTAL);
    ~Slider() override;

    /// 向き。
    Property<Orientation> orientation;
    /// 目盛りの間隔(既定 0: 目盛りを出さない。docs/adr/0049)。minimum から、この間隔ごとと maximum に、
    /// 溝の下(縦なら右)へ目盛りを出す。目盛りが詰まりすぎるときは出さない。
    Property<double> tick_interval;

protected:
    void paint(Canvas& canvas) override;
    void mouse_down(MouseEvent& e) override;
    void mouse_move(MouseEvent& e) override;
    void mouse_up(MouseEvent& e) override;
    void mouse_enter() override;
    void mouse_leave() override;
    void mouse_wheel(WheelEvent& e) override;
    void key_down(KeyEvent& e) override;
    void focus_in() override;
    void focus_out() override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
