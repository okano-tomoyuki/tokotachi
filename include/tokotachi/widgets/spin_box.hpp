/// @file
/// 数値の入力欄: 数値を打ち込むか、上下のボタン・矢印キーで single_step ずつ増減する(docs/adr/0024)。
///
/// @code
/// tokotachi::SpinBox* spin_box_count = new tokotachi::SpinBox(panel);
/// spin_box_count->maximum = 10;
/// spin_box_count->decimals = 1;  // 小数 1 桁で表示・入力
/// spin_box_count->suffix = " kg";  // 数値の後ろに単位を出す
/// tokotachi::bind(spin_box_count->value, amount->value);  // DoubleVariable と結ぶ
/// @endcode
///
/// 打ち込んだ数値は、Enter・フォーカスが外れたときに value にする(数値として読めなければ元に戻す)。
/// ↑↓(single_step)・Page Up/Down(page_step)で増減し、フォーカスがあるときはホイールでも増減する。
/// ボタンを押し続けると繰り返す。フォーカスは中の入力欄が受ける。
#ifndef TOKOTACHI_WIDGETS_SPIN_BOX_HPP
#define TOKOTACHI_WIDGETS_SPIN_BOX_HPP

#include <string>

#include "range_base.hpp"
#include "../core/timer.hpp"

namespace tokotachi
{

namespace detail
{
class SpinEdit;
}

class SpinBox : public RangeBase
{
public:
    explicit SpinBox(Widget* parent = nullptr);

    /// 小数の桁数(既定 0。0〜10)。表示をこの桁に丸め、打ち込んだ数値もこの桁に丸める。
    Property<int> decimals;
    /// 数値の前・後ろに出す文字(既定は空。「¥」「 kg」「%」など。docs/adr/0049)。打ち込むときは、付けても付けなくてもよい。
    Property<std::string> prefix;
    Property<std::string> suffix;
    /// 入力欄に出している文字列(打ち込み中は、まだ value になっていないことがある)。
    ReadOnlyProperty<std::string> text;

    /// 打ち込み中の文字列を value にする(Enter・フォーカスが外れたときに自動で呼ぶ)。
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
    std::string get_prefix() const { return prefix_; }
    void set_prefix(const std::string& v);
    std::string get_suffix() const { return suffix_; }
    void set_suffix(const std::string& v);
    std::string get_text() const;

    std::string format(double v) const;
    /// 上下のボタンの領域(自分の座標系)。
    Rect up_button() const;
    Rect down_button() const;
    /// ボタンを押し始めた・押し続けている(direction は 1 か -1)。
    void press(int direction);
    void stop_repeat();

    detail::SpinEdit* edit_;
    int decimals_;
    std::string prefix_;
    std::string suffix_;
    int pressed_;  ///< 押しているボタン(1 は上、-1 は下、0 は無し)
    Timer repeat_;
};

}  // namespace tokotachi

#endif
