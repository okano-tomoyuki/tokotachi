// plutovg と自前のグリフの合成で、ARGB32(アルファ乗算済み)のバッファに描く Canvas(内部。docs/adr/0006、0032)。
// 座標は呼び出し側で dp からピクセルに直し、plutovg には常にピクセルの座標を渡す(plutovg の変換の行列は使わない。
// グラデーション・画像の模様の行列は除く)。
#ifndef GUI_SRC_GFX_RASTER_CANVAS_HPP
#define GUI_SRC_GFX_RASTER_CANVAS_HPP

#include <cstdint>
#include <vector>

#include <gui/graphics/canvas.hpp>

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

    using Canvas::draw_text;
    using Canvas::stroke_path;

    float scale() const override { return scale_; }
    void save() override;
    void restore() override;
    void translate(float dx, float dy) override;
    void scale_by(float sx, float sy) override;
    void rotate(float radians) override;
    void clip_rect(const Rect& r) override;
    void clip_path(const Path& path) override;
    Rect clip_bounds() const override;
    void begin_layer(float opacity) override;
    void end_layer() override;
    void fill_rect(const Rect& r, const Color& color) override;
    void fill_round_rect(const Rect& r, float radius, const Paint& paint) override;
    void stroke_round_rect(const Rect& r, float radius, float width, const Color& color) override;
    void fill_path(const Path& path, const Paint& paint) override;
    void stroke_path(const Path& path, const StrokeStyle& style, const Paint& paint) override;
    void draw_text(const TextLayout& layout, const Point& pos, const Color& color) override;
    void draw_image(const Image& image, const Rect& dst, float opacity) override;

private:
    struct State
    {
        // dp の座標の変換(x' = a x + c y + e、y' = b x + d y + f)。ピクセルはこれに倍率を掛けたもの。
        float a, b, c, d, e, f;
        PixelRect clip;     // クリップを囲む矩形(ピクセル)
        bool clip_is_rect;  // クリップが clip の矩形そのものか(回転した矩形・パスでクリップしていない)
    };

    // 変換が平行移動だけか(矩形と文字をピクセルに揃えて描ける)。
    bool axis_aligned() const { return state_.a == 1 && state_.b == 0 && state_.c == 0 && state_.d == 1; }
    // dp(今の座標系)からピクセルへ。
    float px(float x, float y) const { return (state_.a * x + state_.c * y + state_.e) * scale_; }
    float py(float x, float y) const { return (state_.b * x + state_.d * y + state_.f) * scale_; }
    // 変換の拡大の大きさ(線の太さに掛ける)。
    float linear_scale() const;
    // 矩形の辺をピクセルの整数に揃える(axis_aligned のときだけ使う)。
    PixelRect snap(const Rect& r) const;
    // path をピクセルの座標で plutovg に渡す。bounds があれば、点の範囲(ピクセル)を返す。
    void set_path(const Path& path, PixelRect* bounds = nullptr);
    void set_paint(const Paint& paint);
    void set_stroke(const StrokeStyle& style);
    // 今のクリップの内側に、plutovg のクリップを合わせる(begin・矩形のクリップの後)。
    void apply_clip();
    // グリフを輪郭として描く(拡大・回転しているとき・パスでクリップしているとき)。
    void draw_text_outlines(const TextLayout& layout, const Point& pos, const Color& color);
    // グリフのビットマップを (gx, gy)(ピクセル)に、現在のクリップの中で合成する。table は被覆率の補正の表。
    void blend_glyph(const GlyphBitmap& g, int gx, int gy, const Color& color, const std::uint8_t* table);

    // begin_layer の一時的な描き先と、切り替える前の描き先。
    struct Layer
    {
        std::vector<std::uint32_t> pixels;
        plutovg_surface* surface;
        plutovg_canvas* canvas;
        int x, y, width, height;  // 描き先の全体の座標での位置と大きさ(ピクセル)
        float opacity;
        std::size_t depth;  // begin_layer の時点の stack_ の深さ
        plutovg_surface* parent_surface;
        plutovg_canvas* parent_canvas;
        std::uint32_t* parent_buffer;
        int parent_width, parent_height, parent_x, parent_y;
    };

    plutovg_surface* surface_;
    plutovg_canvas* canvas_;
    std::uint32_t* buffer_;  // 今の描き先(レイヤーならその一時的な画像)
    int width_, height_;
    // 今の描き先の、全体の座標での左上(ピクセル)。座標はいつも全体の座標で計算し、plutovg にはこの分の移動を行列で、
    // 自前のグリフの合成ではこの分を引いて渡す。
    int origin_x_, origin_y_;
    float scale_;
    State state_;
    std::vector<State> stack_;
    std::vector<Layer*> layers_;
};

}  // namespace detail
}  // namespace gui

#endif
