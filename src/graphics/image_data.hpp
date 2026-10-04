// Image の中身(内部): plutovg の画像(アルファ乗算済みの ARGB32)。
#ifndef GUI_SRC_GFX_IMAGE_DATA_HPP
#define GUI_SRC_GFX_IMAGE_DATA_HPP

struct plutovg_surface;

namespace gui
{
namespace detail
{

struct ImageData
{
    explicit ImageData(plutovg_surface* s) : surface(s) {}
    ~ImageData();

    ImageData(const ImageData&) = delete;
    ImageData& operator=(const ImageData&) = delete;

    plutovg_surface* surface;
};

}  // namespace detail
}  // namespace gui

#endif
