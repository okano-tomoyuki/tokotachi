/// @file
/// tokotachi::ColorDialog の宣言。
#ifndef TOKOTACHI_WINDOWS_COLOR_DIALOG_HPP
#define TOKOTACHI_WINDOWS_COLOR_DIALOG_HPP

#include <string>

#include "../core/export.hpp"
#include "dialog.hpp"

namespace tokotachi
{

/// 色を選ぶダイアログ(docs/adr/0051)。
///
/// @image html color_dialog.png
///
/// @code
/// tokotachi::ColorDialog::show(&window, paint_box_canvas_color, [=](tokotachi::Object* sender)
/// {
///     tokotachi::ColorDialog* color_dialog = static_cast<tokotachi::ColorDialog*>(sender);
///     if (color_dialog->result == tokotachi::DialogButton::OK) { ... color_dialog->color ... }
/// });
/// @endcode
///
/// 左の四角で彩度(横)と明度(縦)を、下の帯で色相を選び、右で 16 進数(\#RRGGBB)・RGB の数値を打ち込むか、見本の色を押す。
/// alpha_enabled なら、不透明度の帯と数値も出す(16 進数は \#RRGGBBAA)。右上に選んでいる色と、開いたときの色を並べて見せる。
/// show は開いたダイアログを返し、閉じたら on_finish を呼んでから破棄する。
///
/// @ingroup windows
class TOKOTACHI_API ColorDialog : public Dialog
{
public:
    /// title が空なら、言語に合わせた標準の題(「色の選択」)。OK・キャンセルのボタンを持つ。
    explicit ColorDialog(Window* parent, const std::string& title = std::string());
    ~ColorDialog() override;

    /// 選んでいる色。代入すると、開いたときの色(比べて見せる色)も同じにする(開いている間の利用者の操作では変えない)。
    Property<Color> color;
    /// 不透明度も選ぶか(既定 false。false なら color の不透明度は 255)。
    Property<bool> alpha_enabled;

    /// 作って initial の色で開き、閉じたら on_finish を呼んで破棄する。
    static ColorDialog* show(Window* parent, const Color& initial, const NotifyHandler& on_finish = NotifyHandler(),
                             bool alpha = false, const std::string& title = std::string());

private:
    template <typename>
    friend struct detail::ImplAccess;
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
