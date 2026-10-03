// 押しボタン。
#ifndef GUI_BUTTON_HPP
#define GUI_BUTTON_HPP

#include "button_base.hpp"

namespace gui
{

class Button : public ButtonBase
{
public:
    explicit Button(Widget* parent = nullptr, const std::string& text = std::string());

protected:
    void paint(Canvas& canvas) override;
    Size compute_preferred_size() const override;
};

}  // namespace gui

#endif
