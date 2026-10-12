/// @file
/// tokotachi::ColorDialog の宣言。
#ifndef TOKOTACHI_WINDOWS_COLOR_DIALOG_HPP
#define TOKOTACHI_WINDOWS_COLOR_DIALOG_HPP

#include <string>

#include "dialog.hpp"

namespace tokotachi
{

class LineEdit;
class SpinBox;
class Label;

namespace detail
{
class ColorPlane;
class ColorStrip;
class ColorPreview;
class ColorSwatches;
}

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
class ColorDialog : public Dialog
{
public:
    /// title が空なら、言語に合わせた標準の題(「色の選択」)。OK・キャンセルのボタンを持つ。
    explicit ColorDialog(Window* parent, const std::string& title = std::string());

    /// 選んでいる色。代入すると、開いたときの色(比べて見せる色)も同じにする(開いている間の利用者の操作では変えない)。
    Property<Color> color;
    /// 不透明度も選ぶか(既定 false。false なら color の不透明度は 255)。
    Property<bool> alpha_enabled;

    /// 作って initial の色で開き、閉じたら on_finish を呼んで破棄する。
    static ColorDialog* show(Window* parent, const Color& initial, const NotifyHandler& on_finish = NotifyHandler(),
                             bool alpha = false, const std::string& title = std::string());

private:
    friend class detail::ColorPlane;
    friend class detail::ColorStrip;
    friend class detail::ColorSwatches;
    friend class detail::ColorPreview;

    Color get_color() const { return color_; }
    void set_color(const Color& v);
    bool get_alpha_enabled() const { return alpha_enabled_; }
    void set_alpha_enabled(const bool& v);

    /// 色相・彩度・明度・不透明度から色を決め、ほかの欄を合わせる(changed_by の欄は変えない)。
    void apply_hsv(const void* changed_by);
    /// 色から色相などを決め、ほかの欄を合わせる。
    void apply_color(const Color& c, const void* changed_by);
    /// 欄(16 進数・数値・四角・帯・見本)を今の色に合わせる。
    void sync(const void* changed_by);

    Color color_;
    Color original_;    ///< 開いたときの色(比べて見せる)
    bool alpha_enabled_;
    float hue_;         ///< 0〜360
    float saturation_;  ///< 0〜1
    float value_;       ///< 0〜1
    bool syncing_;
    detail::ColorPlane* plane_;
    detail::ColorStrip* hue_strip_;
    detail::ColorStrip* alpha_strip_;
    detail::ColorPreview* preview_;
    detail::ColorSwatches* swatches_;
    LineEdit* hex_;
    SpinBox* channels_[4];  ///< R・G・B・A
    Label* alpha_label_;
};

}  // namespace tokotachi

#endif
