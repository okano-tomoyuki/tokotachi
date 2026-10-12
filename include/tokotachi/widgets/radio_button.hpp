/// @file
/// tokotachi::RadioButton の宣言。
#ifndef TOKOTACHI_WIDGETS_RADIO_BUTTON_HPP
#define TOKOTACHI_WIDGETS_RADIO_BUTTON_HPP

#include "../core/export.hpp"
#include "button_base.hpp"

namespace tokotachi
{

/// ラジオボタン。group_value が自分の value と等しいときに選ばれている(docs/adr/0016)。
///
/// @image html widget_radio_button.png
///
/// グループは、各ボタンの group_value を同じ IntVariable に結んで作る(型の変換は要らない):
/// @code
/// tokotachi::IntVariable* size = new tokotachi::IntVariable(window_main, 2);
/// tokotachi::RadioButton* radio_button_small = new tokotachi::RadioButton(window_main, "小", 1);
/// tokotachi::RadioButton* radio_button_large = new tokotachi::RadioButton(window_main, "大", 2);
/// tokotachi::bind(radio_button_small->group_value, size->value);
/// tokotachi::bind(radio_button_large->group_value, size->value);   // radio_button_large が選ばれた状態になる
/// @endcode
/// 押すと group_value に自分の value を入れ、結んだ変数を通じて他のボタンの選択が外れる。
///
/// @ingroup widgets_input
class TOKOTACHI_API RadioButton : public ButtonBase
{
public:
    explicit RadioButton(Widget* parent = nullptr, const std::string& text = std::string(), int value = 0);
    ~RadioButton() override;

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
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
