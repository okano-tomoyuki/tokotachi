// 押しボタン。
#ifndef GUI_BUTTON_HPP
#define GUI_BUTTON_HPP

#include "button_base.hpp"

namespace gui
{

// 見た目の種類。
enum class ButtonStyle
{
    PRIMARY,   // 強調の色で塗る(主な操作)
    SECONDARY  // 地の色に枠(その他の操作。ダイアログのキャンセルなど)
};

class Button : public ButtonBase
{
public:
    explicit Button(Widget* parent = nullptr, const std::string& text = std::string());

    // 既定は PRIMARY。
    Property<ButtonStyle> style;

protected:
    void paint(Canvas& canvas) override;
    Size compute_preferred_size() const override;

private:
    void set_style(const ButtonStyle& v);

    ButtonStyle style_;
};

}  // namespace gui

#endif
