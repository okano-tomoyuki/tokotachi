#include "raster_canvas.hpp"

#include <plutovg.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include <gui/image.hpp>
#include <gui/text_layout.hpp>

#include "image_data.hpp"

#include "../text/font_cache.hpp"
#include "../text/text_layout_data.hpp"

namespace gui
{
namespace detail
{

namespace
{

int round_px(float v) { return static_cast<int>(std::floor(v + 0.5f)); }

PixelRect intersect(const PixelRect& a, const PixelRect& b)
{
    PixelRect r = {std::max(a.x0, b.x0), std::max(a.y0, b.y0), std::min(a.x1, b.x1), std::min(a.y1, b.y1)};
    if (r.empty())
    {
        r.x1 = r.x0;
        r.y1 = r.y0;
    }
    return r;
}

plutovg_color_t to_plutovg(const Color& c)
{
    plutovg_color_t out;
    out.r = c.r / 255.0f;
    out.g = c.g / 255.0f;
    out.b = c.b / 255.0f;
    out.a = c.a / 255.0f;
    return out;
}

void set_color(plutovg_canvas_t* canvas, const Color& c)
{
    plutovg_canvas_set_rgba(canvas, c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
}

}  // namespace

RasterCanvas::RasterCanvas() : surface_(nullptr), canvas_(nullptr), buffer_(nullptr), width_(0), height_(0), scale_(1)
{
    state_.tx = state_.ty = 0;
    state_.clip.x0 = state_.clip.y0 = state_.clip.x1 = state_.clip.y1 = 0;
}

RasterCanvas::~RasterCanvas()
{
    if (canvas_)
    {
        plutovg_canvas_destroy(canvas_);
    }
    if (surface_)
    {
        plutovg_surface_destroy(surface_);
    }
}

void RasterCanvas::set_target(std::uint32_t* buffer, int width, int height)
{
    if (canvas_)
    {
        plutovg_canvas_destroy(canvas_);
    }
    if (surface_)
    {
        plutovg_surface_destroy(surface_);
    }
    // plutovg の画素はアルファ乗算済みの 0xAARRGGBB で、SDL の SDL_PIXELFORMAT_ARGB8888 と同じ並び。
    surface_ = plutovg_surface_create_for_data(reinterpret_cast<unsigned char*>(buffer), width, height, width * 4);
    canvas_ = plutovg_canvas_create(surface_);
    buffer_ = buffer;
    width_ = width;
    height_ = height;
}

void RasterCanvas::apply_clip()
{
    plutovg_canvas_clip_rect(canvas_, static_cast<float>(state_.clip.x0), static_cast<float>(state_.clip.y0),
                             static_cast<float>(state_.clip.x1 - state_.clip.x0),
                             static_cast<float>(state_.clip.y1 - state_.clip.y0));
}

void RasterCanvas::begin(const PixelRect& clip)
{
    const PixelRect all = {0, 0, width_, height_};
    state_.tx = state_.ty = 0;
    state_.clip = intersect(clip, all);
    stack_.clear();
    plutovg_canvas_save(canvas_);
    apply_clip();
}

void RasterCanvas::end()
{
    while (!stack_.empty())
    {
        restore();
    }
    plutovg_canvas_restore(canvas_);
}

void RasterCanvas::save()
{
    stack_.push_back(state_);
    plutovg_canvas_save(canvas_);
}

void RasterCanvas::restore()
{
    if (stack_.empty())
    {
        return;
    }
    state_ = stack_.back();
    stack_.pop_back();
    plutovg_canvas_restore(canvas_);
}

void RasterCanvas::translate(float dx, float dy)
{
    state_.tx += dx;
    state_.ty += dy;
}

PixelRect RasterCanvas::snap(const Rect& r) const
{
    const PixelRect p = {round_px(px(r.x)), round_px(py(r.y)), round_px(px(r.right())), round_px(py(r.bottom()))};
    return p;
}

void RasterCanvas::clip_rect(const Rect& r)
{
    state_.clip = intersect(state_.clip, snap(r));
    apply_clip();
}

Rect RasterCanvas::clip_bounds() const
{
    return Rect(state_.clip.x0 / scale_ - state_.tx, state_.clip.y0 / scale_ - state_.ty,
                (state_.clip.x1 - state_.clip.x0) / scale_, (state_.clip.y1 - state_.clip.y0) / scale_);
}

void RasterCanvas::fill_rect(const Rect& r, const Color& color)
{
    const PixelRect p = snap(r);
    if (p.empty())
    {
        return;
    }
    set_color(canvas_, color);
    plutovg_canvas_fill_rect(canvas_, static_cast<float>(p.x0), static_cast<float>(p.y0),
                             static_cast<float>(p.x1 - p.x0), static_cast<float>(p.y1 - p.y0));
}

void RasterCanvas::fill_round_rect(const Rect& r, float radius, const Paint& paint)
{
    const PixelRect p = snap(r);
    if (p.empty())
    {
        return;
    }
    const float rad = radius * scale_;
    plutovg_canvas_new_path(canvas_);
    plutovg_canvas_round_rect(canvas_, static_cast<float>(p.x0), static_cast<float>(p.y0),
                              static_cast<float>(p.x1 - p.x0), static_cast<float>(p.y1 - p.y0), rad, rad);
    set_paint(paint);
    plutovg_canvas_fill(canvas_);
}

void RasterCanvas::stroke_round_rect(const Rect& r, float radius, float width, const Color& color)
{
    const PixelRect p = snap(r);
    if (p.empty())
    {
        return;
    }
    // 線幅をピクセルの整数にし、線の中心を半ピクセルずらしてにじまないようにする。
    const float sw = std::max(1.0f, std::floor(width * scale_ + 0.5f));
    const float x0 = p.x0 + sw / 2, y0 = p.y0 + sw / 2, x1 = p.x1 - sw / 2, y1 = p.y1 - sw / 2;
    if (x1 <= x0 || y1 <= y0)
    {
        return;
    }
    const float rad = std::max(0.0f, radius * scale_ - sw / 2);
    plutovg_canvas_new_path(canvas_);
    plutovg_canvas_round_rect(canvas_, x0, y0, x1 - x0, y1 - y0, rad, rad);
    set_color(canvas_, color);
    plutovg_canvas_set_line_width(canvas_, sw);
    plutovg_canvas_stroke(canvas_);
}

void RasterCanvas::set_path(const Path& path)
{
    plutovg_canvas_new_path(canvas_);
    const std::vector<Path::Element>& els = path.elements();
    for (std::size_t i = 0; i < els.size(); ++i)
    {
        const float* p = els[i].pts;
        switch (els[i].op)
        {
        case Path::Op::MOVE_TO:
            plutovg_canvas_move_to(canvas_, px(p[0]), py(p[1]));
            break;
        case Path::Op::LINE_TO:
            plutovg_canvas_line_to(canvas_, px(p[0]), py(p[1]));
            break;
        case Path::Op::CUBIC_TO:
            plutovg_canvas_cubic_to(canvas_, px(p[0]), py(p[1]), px(p[2]), py(p[3]), px(p[4]), py(p[5]));
            break;
        case Path::Op::CLOSE:
            plutovg_canvas_close_path(canvas_);
            break;
        }
    }
}

void RasterCanvas::set_paint(const Paint& paint)
{
    if (paint.type == Paint::Type::SOLID)
    {
        set_color(canvas_, paint.color);
        return;
    }
    std::vector<plutovg_gradient_stop_t> stops(paint.stops.size());
    for (std::size_t i = 0; i < stops.size(); ++i)
    {
        stops[i].offset = paint.stops[i].offset;
        stops[i].color = to_plutovg(paint.stops[i].color);
    }
    const plutovg_gradient_stop_t* data = stops.empty() ? nullptr : &stops[0];
    const int n = static_cast<int>(stops.size());
    if (paint.type == Paint::Type::LINEAR)
    {
        plutovg_canvas_set_linear_gradient(canvas_, px(paint.p0.x), py(paint.p0.y), px(paint.p1.x), py(paint.p1.y),
                                           PLUTOVG_SPREAD_METHOD_PAD, data, n, nullptr);
    }
    else
    {
        plutovg_canvas_set_radial_gradient(canvas_, px(paint.p0.x), py(paint.p0.y), paint.radius * scale_,
                                           px(paint.p1.x), py(paint.p1.y), 0, PLUTOVG_SPREAD_METHOD_PAD, data, n,
                                           nullptr);
    }
}

void RasterCanvas::fill_path(const Path& path, const Paint& paint)
{
    if (path.empty())
    {
        return;
    }
    set_path(path);
    set_paint(paint);
    plutovg_canvas_fill(canvas_);
}

void RasterCanvas::stroke_path(const Path& path, float width, const Paint& paint, LineCap cap)
{
    if (path.empty())
    {
        return;
    }
    set_path(path);
    set_paint(paint);
    plutovg_canvas_set_line_width(canvas_, width * scale_);
    plutovg_line_cap_t c = PLUTOVG_LINE_CAP_BUTT;
    if (cap == LineCap::ROUND)
    {
        c = PLUTOVG_LINE_CAP_ROUND;
    }
    else if (cap == LineCap::SQUARE)
    {
        c = PLUTOVG_LINE_CAP_SQUARE;
    }
    plutovg_canvas_set_line_cap(canvas_, c);
    plutovg_canvas_stroke(canvas_);
    plutovg_canvas_set_line_cap(canvas_, PLUTOVG_LINE_CAP_BUTT);
}

void RasterCanvas::draw_image(const Image& image, const Rect& dst, float opacity)
{
    const PixelRect p = snap(dst);
    if (image.is_null() || p.empty() || opacity <= 0)
    {
        return;
    }
    // 画像の画素を、ピクセルに揃えた dst へ写す行列。
    const float sx = static_cast<float>(p.x1 - p.x0) / image.width();
    const float sy = static_cast<float>(p.y1 - p.y0) / image.height();
    plutovg_matrix_t m;
    plutovg_matrix_init(&m, sx, 0, 0, sy, static_cast<float>(p.x0), static_cast<float>(p.y0));
    plutovg_canvas_set_texture(canvas_, image.data()->surface, PLUTOVG_TEXTURE_TYPE_PLAIN, std::min(opacity, 1.0f),
                               &m);
    plutovg_canvas_fill_rect(canvas_, static_cast<float>(p.x0), static_cast<float>(p.y0),
                             static_cast<float>(p.x1 - p.x0), static_cast<float>(p.y1 - p.y0));
}

// グリフの被覆率の補正(docs/adr/0020)。sRGB の値をそのまま線形に混ぜると、文字が細く見える(明るい文字では特に)。
// 被覆率 c を c^(1/k) に持ち上げて補う。k は文字色の明るさで、暗い文字 TEXT_BOOST_DARK から明るい文字 TEXT_BOOST_LIGHT まで変える。
// 環境変数 GUI_TEXT_BOOST で k を固定できる(見え方の確認用。1 で補正なし)。
const float TEXT_BOOST_DARK = 1.25f;
const float TEXT_BOOST_LIGHT = 1.6f;

void text_coverage_table(const Color& color, std::uint8_t* table)
{
    const float luminance = (0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b) / 255.0f;
    float k = TEXT_BOOST_DARK + (TEXT_BOOST_LIGHT - TEXT_BOOST_DARK) * luminance;
    if (const char* fixed = std::getenv("GUI_TEXT_BOOST"))
    {
        k = static_cast<float>(std::atof(fixed));
    }
    if (k <= 0)
    {
        k = 1;
    }
    for (int i = 0; i < 256; ++i)
    {
        table[i] = static_cast<std::uint8_t>(std::floor(255.0f * std::pow(i / 255.0f, 1.0f / k) + 0.5f));
    }
}

void RasterCanvas::blend_glyph(const GlyphBitmap& g, int gx, int gy, const Color& color, const std::uint8_t* table)
{
    const PixelRect& clip = state_.clip;
    const std::uint32_t sr = color.r, sg = color.g, sb = color.b, sa = color.a;
    const int y_begin = std::max(clip.y0, gy), y_end = std::min(clip.y1, gy + g.height);
    const int x_begin = std::max(clip.x0, gx), x_end = std::min(clip.x1, gx + g.width);
    for (int y = y_begin; y < y_end; ++y)
    {
        std::uint32_t* row = buffer_ + static_cast<std::size_t>(y) * static_cast<std::size_t>(width_);
        const std::uint8_t* cov = &g.coverage[static_cast<std::size_t>(y - gy) * static_cast<std::size_t>(g.width)];
        for (int x = x_begin; x < x_end; ++x)
        {
            const std::uint32_t a = table[cov[x - gx]] * sa / 255;
            if (a == 0)
            {
                continue;
            }
            // アルファ乗算済みの source-over(ガンマ補正なし。docs/adr/0007)
            const std::uint32_t d32 = row[x];
            const std::uint32_t ia = 255 - a;
            const std::uint32_t da = (d32 >> 24) & 0xff, dr = (d32 >> 16) & 0xff, dg = (d32 >> 8) & 0xff,
                                db = d32 & 0xff;
            const std::uint32_t oa = a + (da * ia + 127) / 255;
            const std::uint32_t orr = (sr * a + dr * ia + 127) / 255;
            const std::uint32_t og = (sg * a + dg * ia + 127) / 255;
            const std::uint32_t ob = (sb * a + db * ia + 127) / 255;
            row[x] = (oa << 24) | (orr << 16) | (og << 8) | ob;
        }
    }
}

void RasterCanvas::draw_text(const TextLayout& layout, const Point& pos, const Color& color)
{
    const TextLayoutData* d = layout.data();
    if (!d || color.a == 0 || state_.clip.empty())
    {
        return;
    }
    const PixelRect& clip = state_.clip;
    std::uint8_t table[256];
    text_coverage_table(color, table);
    for (std::size_t li = 0; li < d->lines.size(); ++li)
    {
        const LineData& line = d->lines[li];
        const float ox = std::floor(px(pos.x) + line.x + 0.5f);
        const float baseline = std::floor(py(pos.y) + line.y + d->ascent + 0.5f);
        if (baseline - d->ascent >= clip.y1 || baseline + d->descent < clip.y0)
        {
            continue;  // クリップの外の行は描かない
        }
        for (std::size_t r = 0; r < line.runs.size(); ++r)
        {
            const GlyphRun& run = line.runs[r];
            for (std::size_t i = 0; i < run.glyphs.size(); ++i)
            {
                // 横は 1/4 ピクセル単位で置く(docs/adr/0020)。縦はベースラインをピクセルに揃える。
                const int steps = FontInstance::SUBPIXEL_STEPS;
                const float gx = ox + run.glyphs[i].x;
                int ix = static_cast<int>(std::floor(gx));
                int sub = static_cast<int>(std::floor((gx - ix) * steps + 0.5f));
                if (sub == steps)
                {
                    ++ix;
                    sub = 0;
                }
                const GlyphBitmap& g = run.font->glyph(run.glyphs[i].id, sub);
                if (g.width > 0)
                {
                    blend_glyph(g, ix + g.left, round_px(baseline + run.glyphs[i].y) - g.top, color, table);
                }
            }
        }
    }
}

}  // namespace detail
}  // namespace gui
