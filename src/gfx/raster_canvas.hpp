// plutovg と自前のグリフの合成で、ARGB32(アルファ乗算済み)のバッファに描く Canvas(内部。docs/adr/0006)。
// 座標は呼び出し側で dp からピクセルに直し、plutovg には常にピクセルの座標を渡す(plutovg の変換の行列は使わない)。
#ifndef GUI_SRC_GFX_RASTER_CANVAS_HPP
#define GUI_SRC_GFX_RASTER_CANVAS_HPP

#include <cstdint>
#include <vector>

#include <gui/canvas.hpp>

struct plutovg_surface;
struct plutovg_canvas;

namespace gui
{
namespace detail
{

struct GlyphBitmap;

struct PixelRect
{
    int x0, y0, x1, y1;  // [x0, x1) x [y0, y1)

    bool empty() const { return x1 <= x0 || y1 <= y0; }
};

class RasterCanvas : public Canvas
{
public:
    RasterCanvas();
    ~RasterCanvas();

    RasterCanvas(const RasterCanvas&) = delete;
    RasterCanvas& operator=(const RasterCanvas&) = delete;

    // 描画先を差し替える(バッファの大きさが変わったとき)。buffer は width * height の画素。
    void set_target(std::uint32_t* buffer, int width, int height);
    void set_scale(float scale) { scale_ = scale; }

    // 1 回の描画の始めと終わり。clip(ピクセル)の外には描かない。
    void begin(const PixelRect& clip);
    void end();

    float scale() const override { return scale_; }
    void save() override;
    void restore() override;
    void translate(float dx, float dy) override;
    void clip_rect(const Rect& r) override;
    Rect clip_bounds() const override;
    void fill_rect(const Rect& r, const Color& color) override;
    void fill_round_rect(const Rect& r, float radius, const Paint& paint) override;
    void stroke_round_rect(const Rect& r, float radius, float width, const Color& color) override;
    void fill_path(const Path& path, const Paint& paint) override;
    void stroke_path(const Path& path, float width, const Paint& paint, LineCap cap) override;
    void draw_text(const TextLayout& layout, const Point& pos, const Color& color) override;
    void draw_image(const Image& image, const Rect& dst, float opacity) override;

private:
    struct State
    {
        float tx, ty;  // dp
        PixelRect clip;
    };

    // dp(現在の原点からの相対)からピクセルへ。
    float px(float x) const { return (x + state_.tx) * scale_; }
    float py(float y) const { return (y + state_.ty) * scale_; }
    // 矩形の辺をピクセルの整数に揃える。
    PixelRect snap(const Rect& r) const;
    void set_path(const Path& path);
    void set_paint(const Paint& paint);
    // グリフのビットマップを (gx, gy)(ピクセル)に、現在のクリップの中で合成する。table は被覆率の補正の表。
    void blend_glyph(const GlyphBitmap& g, int gx, int gy, const Color& color, const std::uint8_t* table);
    void apply_clip();

    plutovg_surface* surface_;
    plutovg_canvas* canvas_;
    std::uint32_t* buffer_;
    int width_, height_;
    float scale_;
    State state_;
    std::vector<State> stack_;
};

}  // namespace detail
}  // namespace gui

#endif
