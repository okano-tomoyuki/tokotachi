/// @file
/// tokotachi::ImageBox の宣言。
#ifndef TOKOTACHI_WIDGETS_IMAGE_BOX_HPP
#define TOKOTACHI_WIDGETS_IMAGE_BOX_HPP

#include "../core/export.hpp"
#include "../graphics/image.hpp"
#include "widget.hpp"

namespace tokotachi
{

/// 画像を部品の大きさにどう合わせるか。どれも部品の中ほどに置き、はみ出した部分は描かない。
enum class ImageFit
{
    NONE,     ///< 元の大きさのまま
    CONTAIN,  ///< 縦横の比を保ち、全体が収まる大きさにする
    COVER,    ///< 縦横の比を保ち、隙間なく覆う大きさにする(はみ出した部分は切れる)
    STRETCH   ///< 縦横の比を変えて、部品の大きさにする
};

/// 画像を表示する部品(docs/adr/0048)。
///
/// @image html widget_image_box.png
///
/// @code
/// tokotachi::ImageBox* image_box_logo = new tokotachi::ImageBox(&window);
/// image_box_logo->image = tokotachi::Image::load("logo.png");
/// image_box_logo->fit = tokotachi::ImageFit::CONTAIN;  // 大きさに合わせて縦横の比を保って拡大・縮小する
/// @endcode
///
/// 画像の 1 ピクセルを、論理座標の 1(倍率 2 の画面では 2 ピクセル)として描く。望む大きさは画像の大きさ。
///
/// @ingroup widgets_display
class TOKOTACHI_API ImageBox : public Widget
{
public:
    explicit ImageBox(Widget* parent = nullptr, const Image& image = Image());

    Property<Image> image;
    /// 既定は NONE。
    Property<ImageFit> fit;

protected:
    void paint(Canvas& canvas) override;
    Size compute_preferred_size() const override;

private:
    void set_image(const Image& v);
    void set_fit(const ImageFit& v);

    Image image_;
    ImageFit fit_;
};

}  // namespace tokotachi

#endif
