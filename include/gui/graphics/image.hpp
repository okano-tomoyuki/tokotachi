// 画像(アイコンなど)。PNG・JPEG などを読み込み、Canvas::draw_image で描く。
//
//   gui::Image icon = gui::Image::load("folder.png");
//   if (!icon.is_null()) { item->icon = icon; }
//
// 作った後は変更しない値として扱う(コピーは安い。画素は共有する)。
#ifndef GUI_IMAGE_HPP
#define GUI_IMAGE_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "../core/geometry.hpp"

namespace gui
{

namespace detail
{
struct ImageData;
}

class Image
{
public:
    // 空の画像(is_null() が true)。
    Image();

    // ファイル・メモリ上のデータから読み込む(PNG・JPEG・BMP・GIF の 1 枚目など)。読めなければ空の画像。
    static Image load(const std::string& path);
    static Image load_from_data(const void* data, std::size_t size);
    // 画素(左上から行ごと。アルファは乗算していない色)から作る。数が width × height でなければ空の画像。
    static Image from_pixels(int width, int height, const std::vector<Color>& pixels);

    bool is_null() const { return !d_; }
    // 大きさ(ピクセル)。
    int width() const;
    int height() const;
    // 位置 (x, y) の画素(範囲の外・空の画像なら透明)。
    Color pixel(int x, int y) const;

    bool operator==(const Image& other) const { return d_ == other.d_; }
    bool operator!=(const Image& other) const { return d_ != other.d_; }

    const detail::ImageData* data() const { return d_.get(); }

private:
    friend class Bitmap;

    explicit Image(const std::shared_ptr<detail::ImageData>& d) : d_(d) {}

    std::shared_ptr<detail::ImageData> d_;
};

}  // namespace gui

#endif
