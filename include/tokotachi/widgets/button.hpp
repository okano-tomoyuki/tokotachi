/// @file
/// tokotachi::Button の宣言。
#ifndef TOKOTACHI_WIDGETS_BUTTON_HPP
#define TOKOTACHI_WIDGETS_BUTTON_HPP

#include "../core/export.hpp"
#include "../graphics/image.hpp"
#include "button_base.hpp"

namespace tokotachi
{

/// 見た目の種類。
enum class ButtonStyle
{
    PRIMARY,   ///< 強調の色で塗る(主な操作)
    SECONDARY  ///< 地の色に枠(その他の操作。ダイアログのキャンセルなど)
};

/// 押しボタン。icon を与えると、文字の前に画像を出す(text が空なら画像だけのボタン。docs/adr/0048)。
///
/// @image html widget_button.png
///
/// @ingroup widgets_input
class TOKOTACHI_API Button : public ButtonBase
{
public:
    explicit Button(Widget* parent = nullptr, const std::string& text = std::string());
    ~Button() override;

    /// 既定は PRIMARY。
    Property<ButtonStyle> style;
    /// 文字の前に出す画像(既定は空で、出さない)。
    Property<Image> icon;
    /// icon を描く大きさ(論理座標の幅・高さ。既定 16)。
    Property<float> icon_size;

protected:
    void paint(Canvas& canvas) override;
    Size compute_preferred_size() const override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
