/// @file
/// tokotachi::ColorButton の宣言。
#ifndef TOKOTACHI_WIDGETS_COLOR_BUTTON_HPP
#define TOKOTACHI_WIDGETS_COLOR_BUTTON_HPP

#include <string>

#include "../core/export.hpp"
#include "button_base.hpp"

namespace tokotachi
{

/// 色を見せるボタン(docs/adr/0051)。押すと ColorDialog を開き、OK で選んだ色にする。
///
/// @image html widget_color_button.png
///
/// @code
/// tokotachi::ColorButton* color_button_line = new tokotachi::ColorButton(window_main);
/// color_button_line->color = tokotachi::Color::rgb(0x2563EB);
/// color_button_line->on_change = [=](tokotachi::Object*) { ... color_button_line->color ... };
/// @endcode
///
/// 左に色の見本(透過は市松模様の上に)を、右に text(空なら色の 16 進数)を出す。
///
/// @ingroup widgets_input
class TOKOTACHI_API ColorButton : public ButtonBase
{
public:
    /// parent の子として作る(parent が所有する)。
    explicit ColorButton(Widget* parent = nullptr, const Color& color = Color(0, 0, 0));
    ~ColorButton() override;

    /// 選んでいる色。
    Property<Color> color;
    /// ダイアログで不透明度も選ぶか(既定 false)。false でも、代入した色の不透明度はそのまま。
    Property<bool> alpha_enabled;
    /// 開くダイアログの題(空なら標準の題)。
    Property<std::string> dialog_title;
    /// color が変わった(ダイアログで選んだ・代入のどちらでも)。
    Property<NotifyHandler> on_change;

    /// ダイアログを開く(押したときと同じ)。
    void open_dialog();

protected:
    void click() override;
    void paint(Canvas& canvas) override;
    Size compute_preferred_size() const override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
