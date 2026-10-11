// Image の中身(内部): plutovg の画像(アルファ乗算済みの ARGB32)。
#ifndef TOKOTACHI_SRC_GRAPHICS_IMAGE_DATA_HPP
#define TOKOTACHI_SRC_GRAPHICS_IMAGE_DATA_HPP

#include <mutex>
#include <vector>

struct plutovg_surface;

namespace tokotachi
{
namespace detail
{

struct ImageData
{
    explicit ImageData(plutovg_surface* s) : surface(s) {}
    ~ImageData();

    ImageData(const ImageData&) = delete;
    ImageData& operator=(const ImageData&) = delete;

    /// width × height ピクセルに写した画像(縮めるときは面積の平均、広げるときは線形補間)。参照を 1 つ増やして返すので、
    /// 使い終えたら plutovg_surface_destroy で放す。最近使った大きさをいくつか覚えておく(ほかのスレッドからも呼べる)。
    plutovg_surface* scaled(int width, int height) const;

    plutovg_surface* surface;

private:
    struct Scaled
    {
        int width;
        int height;
        plutovg_surface* surface;
    };
    mutable std::mutex mutex_;
    mutable std::vector<Scaled> scaled_;  ///< 新しい順
};

}  // namespace detail
}  // namespace tokotachi

#endif
