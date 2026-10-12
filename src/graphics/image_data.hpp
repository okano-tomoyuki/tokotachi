// Image の中身(内部): plutovg の画像(アルファ乗算済みの ARGB32)。
#ifndef TOKOTACHI_SRC_GRAPHICS_IMAGE_DATA_HPP
#define TOKOTACHI_SRC_GRAPHICS_IMAGE_DATA_HPP

#include <mutex>
#include <vector>

#include <tokotachi/core/geometry.hpp>

struct plutovg_surface;

namespace tokotachi
{
namespace detail
{

struct ImageData
{
    explicit ImageData(plutovg_surface* s) : surface(s), icon(-1), has_color(false) {}
    ~ImageData();

    ImageData(const ImageData&) = delete;
    ImageData& operator=(const ImageData&) = delete;

    /// width × height ピクセルに写した画像(縮めるときは面積の平均、広げるときは線形補間)。参照を 1 つ増やして返すので、
    /// 使い終えたら plutovg_surface_destroy で放す。最近使った大きさをいくつか覚えておく(ほかのスレッドからも呼べる)。
    plutovg_surface* scaled(int width, int height) const;

    plutovg_surface* surface;
    /// アイコン(Image::icon)なら、その番号(線画で描く。surface は 24 × 24 に描いたもの)。ほかは -1。
    int icon;
    /// アイコンの色を決めたか(決めていなければ、描くときに渡す色で塗る)。
    bool has_color;
    Color color;

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
