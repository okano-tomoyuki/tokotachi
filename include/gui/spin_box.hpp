// 数値の入力欄: 数値を打ち込むか、上下のボタン・矢印キーで single_step ずつ増減する(docs/adr/0024)。
//
//   gui::SpinBox* count = new gui::SpinBox(panel);
//   count->maximum = 10;
//   count->decimals = 1;  // 小数 1 桁で表示・入力
//   gui::bind(count->value, amount->value);  // DoubleVariable と結ぶ
//
// 打ち込んだ数値は、Enter・フォーカスが外れたときに value にする(数値として読めなければ元に戻す)。
// ↑↓(single_step)・Page Up/Down(page_step)で増減し、フォーカスがあるときはホイールでも増減する。
// ボタンを押し続けると繰り返す。フォーカスは中の入力欄が受ける。
#ifndef GUI_SPIN_BOX_HPP
#define GUI_SPIN_BOX_HPP

#include <string>

#include "range_base.hpp"
#include "timer.hpp"

namespace gui
{

namespace detail
{
class SpinEdit;
}

class SpinBox : public RangeBase
{
public:
    explicit SpinBox(Widget* parent = nullptr);

    // 小数の桁数(既定 0。0〜10)。表示をこの桁に丸め、打ち込んだ数値もこの桁に丸める。
    Property<int> decimals;
    // 入力欄に出している文字列(打ち込み中は、まだ value になっていないことがある)。
    ReadOnlyProperty<std::string> text;

    // 打ち込み中の文字列を value にする(Enter・フォーカスが外れたときに自動で呼ぶ)。
    void commit();

protected:
    void paint(Canvas& canvas) override;
    void mouse_down(MouseEvent& e) override;
    void mouse_up(MouseEvent& e) override;
    void mouse_wheel(WheelEvent& e) override;
    void key_down(KeyEvent& e) override;
    void arrange_children() override;
    void range_changed() override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    friend class detail::SpinEdit;

    int get_decimals() const { return decimals_; }
    void set_decimals(const int& v);
    std::string get_text() const;

    std::string format(double v) const;
    // 上下のボタンの領域(自分の座標系)。
    Rect up_button() const;
    Rect down_button() const;
    // ボタンを押し始めた・押し続けている(direction は 1 か -1)。
    void press(int direction);
    void stop_repeat();

    detail::SpinEdit* edit_;
    int decimals_;
    int pressed_;  // 押しているボタン(1 は上、-1 は下、0 は無し)
    Timer repeat_;
};

}  // namespace gui

#endif
