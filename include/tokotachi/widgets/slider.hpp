/// @file
/// スライダー: つまみを動かして、範囲の中の値を選ぶ(docs/adr/0024)。縦のときは下が minimum。
///
/// @code
/// tokotachi::Slider* volume = new tokotachi::Slider(panel);
/// volume->maximum = 100;
/// volume->on_change = [=](tokotachi::Object*) { ... volume->value ... };
/// @endcode
///
/// 溝を押すとその位置へ動き、そのままドラッグできる。矢印キー(single_step)・Page Up/Down(page_step)・Home/End で動かし、
/// フォーカスがあるときはホイールでも動かす(フォームをスクロールしているときに、うっかり値を変えないように)。
#ifndef TOKOTACHI_WIDGETS_SLIDER_HPP
#define TOKOTACHI_WIDGETS_SLIDER_HPP

#include "../layout/alignment.hpp"
#include "range_base.hpp"

namespace tokotachi
{

class Slider : public RangeBase
{
public:
    explicit Slider(Widget* parent = nullptr, Orientation orientation = Orientation::HORIZONTAL);

    Property<Orientation> orientation;

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
    Orientation get_orientation() const { return orientation_; }
    void set_orientation(const Orientation& v);

    /// 並ぶ向きの、つまみの中心が動ける長さ(dp)。
    float travel() const;
    /// つまみの中心(自分の座標系)。
    Point thumb_center() const;
    /// 並ぶ向きの位置 p に当たる値(single_step に合わせる)。
    double value_at(const Point& p) const;

    Orientation orientation_;
    bool hovered_;
    bool dragging_;
    Point drag_offset_;  ///< つまみの中心から、押した位置まで
};

}  // namespace tokotachi

#endif
