/// @file
/// tokotachi::Image の宣言。
#ifndef TOKOTACHI_GRAPHICS_IMAGE_HPP
#define TOKOTACHI_GRAPHICS_IMAGE_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "../core/export.hpp"
#include "../core/geometry.hpp"
#include "icon.hpp"

namespace tokotachi
{

namespace detail
{
struct ImageData;
}

/// 画像(アイコンなど)。PNG・JPEG などを読み込み、Canvas::draw_image で描く。
///
/// @code
/// tokotachi::Image icon = tokotachi::Image::load("folder.png");
/// if (!icon.is_null()) { item->icon = icon; }
/// action_save->icon = tokotachi::Image::icon(tokotachi::Icon::SAVE);   // よく使うアイコン(線画)
/// @endcode
///
/// 作った後は変更しない値として扱う(コピーは安い。画素は共有する)。
///
/// @ingroup graphics
class TOKOTACHI_API Image
{
public:
    /// 空の画像(is_null() が true)。
    Image();

    /// ファイル・メモリ上のデータから読み込む(PNG・JPEG・BMP・GIF の 1 枚目など)。読めなければ空の画像。
    static Image load(const std::string& path);
    static Image load_from_data(const void* data, std::size_t size);
    /// 画素(左上から行ごと。アルファは乗算していない色)から作る。数が width × height でなければ空の画像。
    static Image from_pixels(int width, int height, const std::vector<Color>& pixels);
    /// よく使うアイコン(線画。docs/adr/0054)。部品に描くときは部品の文字の色(無効なら薄く、強調した行なら強調の文字の色)で塗り、
    /// 描く大きさに合わせて線を引く(拡大してもぼやけない)。大きさは 24 × 24(ImageBox で元の大きさに出すとき)。
    static Image icon(Icon icon);
    /// 色を決めたアイコン(部品の文字の色によらず、いつもこの色で塗る)。
    static Image icon(Icon icon, const Color& color);

    bool is_null() const { return !d_; }
    /// アイコン(Image::icon で作ったもの)か。
    bool is_icon() const;
    /// 大きさ(ピクセル)。
    int width() const;
    int height() const;
    /// 位置 (x, y) の画素(範囲の外・空の画像なら透明)。
    Color pixel(int x, int y) const;

    bool operator==(const Image& other) const { return d_ == other.d_; }
    bool operator!=(const Image& other) const { return d_ != other.d_; }

    const detail::ImageData* data() const { return d_.get(); }

private:
    friend class Bitmap;

    explicit Image(const std::shared_ptr<detail::ImageData>& d) : d_(d) {}
    static Image make_icon(Icon icon, bool has_color, const Color& color);

    std::shared_ptr<detail::ImageData> d_;
};

}  // namespace tokotachi

#endif
