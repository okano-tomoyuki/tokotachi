/// @file
/// ラジオボタン。group_value が自分の value と等しいときに選ばれている(docs/adr/0016)。
///
/// グループは、各ボタンの group_value を同じ IntVariable に結んで作る(型の変換は要らない):
/// @code
/// tokotachi::IntVariable* size = new tokotachi::IntVariable(&window, 2);
/// tokotachi::RadioButton* small = new tokotachi::RadioButton(&window, "小", 1);
/// tokotachi::RadioButton* large = new tokotachi::RadioButton(&window, "大", 2);
/// tokotachi::bind(small->group_value, size->value);
/// tokotachi::bind(large->group_value, size->value);   // large が選ばれた状態になる
/// @endcode
/// 押すと group_value に自分の value を入れ、結んだ変数を通じて他のボタンの選択が外れる。
#ifndef TOKOTACHI_WIDGETS_RADIO_BUTTON_HPP
#define TOKOTACHI_WIDGETS_RADIO_BUTTON_HPP

#include "button_base.hpp"

namespace tokotachi
{

class RadioButton : public ButtonBase
{
public:
    explicit RadioButton(Widget* parent = nullptr, const std::string& text = std::string(), int value = 0);

    /// このボタンが表す値。
    Property<int> value;
    /// グループで選ばれている値(既定 -1)。
    Property<int> group_value;
    /// group_value == value か。
    ReadOnlyProperty<bool> checked;

    /// checked が変わった(押した・group_value や value が変わったのどちらでも)。押したときは on_change の後に on_click を呼ぶ。
    Property<NotifyHandler> on_change;

protected:
    /// group_value に value を入れてから on_click を呼ぶ。
    void click() override;

    void paint(Canvas& canvas) override;
    Size compute_preferred_size() const override;

private:
    void set_value(const int& v);
    void set_group_value(const int& v);
    bool get_checked() const { return value_ == group_value_; }
    /// value_・group_value_ を変えた後に、変わったプロパティを知らせ、選択が変わっていれば on_change を呼ぶ。
    void finish(bool was_checked, const void* changed_property);

    int value_;
    int group_value_;
    NotifyHandler on_change_;
};

}  // namespace tokotachi

#endif
