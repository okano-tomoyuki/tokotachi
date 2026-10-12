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
/// tokotachi::ColorButton* color_button_line = new tokotachi::ColorButton(&window);
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
    explicit ColorButton(Widget* parent = nullptr, const Color& color = Color(0, 0, 0));

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
    Color get_color() const { return color_; }
    void set_color(const Color& v);
    bool get_alpha_enabled() const { return alpha_enabled_; }
    void set_alpha_enabled(const bool& v);
    std::string get_dialog_title() const { return dialog_title_; }
    void set_dialog_title(const std::string& v) { dialog_title_ = v; }
    /// 出す文字(text が空なら 16 進数)。
    std::string label() const;

    Color color_;
    bool alpha_enabled_;
    std::string dialog_title_;
    NotifyHandler on_change_;
};

}  // namespace tokotachi

#endif
