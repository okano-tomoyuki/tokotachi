/// @file
/// 押しボタン。icon を与えると、文字の前に画像を出す(text が空なら画像だけのボタン。docs/adr/0048)。
#ifndef TOKOTACHI_WIDGETS_BUTTON_HPP
#define TOKOTACHI_WIDGETS_BUTTON_HPP

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

class Button : public ButtonBase
{
public:
    explicit Button(Widget* parent = nullptr, const std::string& text = std::string());

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
    void set_style(const ButtonStyle& v);
    void set_icon(const Image& v);
    void set_icon_size(const float& v);
    /// 画像と、文字との間の幅(画像が無ければ 0)。
    float icon_extent() const;

    ButtonStyle style_;
    Image icon_;
    float icon_size_;
};

}  // namespace tokotachi

#endif
