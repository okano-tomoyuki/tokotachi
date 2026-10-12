#include "raster_canvas.hpp"

#include <plutovg.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include <tokotachi/graphics/image.hpp>
#include <tokotachi/graphics/text_layout.hpp>

#include "image_data.hpp"

#include "font_cache.hpp"
#include "text_layout_data.hpp"

namespace tokotachi
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

// 点の範囲を広げる。
void extend(float x, float y, float& x0, float& y0, float& x1, float& y1)
{
    x0 = std::min(x0, x);
    y0 = std::min(y0, y);
    x1 = std::max(x1, x);
    y1 = std::max(y1, y);
}

}  // namespace

RasterCanvas::RasterCanvas()
    : api_(ImplAccess<Canvas>::create(this)),
      surface_(nullptr), canvas_(nullptr), buffer_(nullptr), width_(0), height_(0), origin_x_(0), origin_y_(0), scale_(1)
{
    state_.a = state_.d = 1;
    state_.b = state_.c = state_.e = state_.f = 0;
    state_.clip.x0 = state_.clip.y0 = state_.clip.x1 = state_.clip.y1 = 0;
    state_.clip_is_rect = true;
}

RasterCanvas::~RasterCanvas()
{
    while (!layers_.empty())
    {
        end_layer();
    }
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

// ---- 状態 ----

void RasterCanvas::apply_clip()
{
    plutovg_canvas_clip_rect(canvas_, static_cast<float>(state_.clip.x0), static_cast<float>(state_.clip.y0),
                             static_cast<float>(state_.clip.x1 - state_.clip.x0),
                             static_cast<float>(state_.clip.y1 - state_.clip.y0));
}

void RasterCanvas::begin(const PixelRect& clip)
{
    const PixelRect all = {0, 0, width_, height_};
    state_.a = state_.d = 1;
    state_.b = state_.c = state_.e = state_.f = 0;
    state_.clip = intersect(clip, all);
    state_.clip_is_rect = true;
    stack_.clear();
    plutovg_canvas_save(canvas_);
    apply_clip();
}

void RasterCanvas::end()
{
    while (!layers_.empty())
    {
        end_layer();
    }
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
    if (stack_.empty() || (!layers_.empty() && stack_.size() <= layers_.back()->depth))
    {
        return;  // レイヤーの中では、begin_layer より前の状態へは戻らない(end_layer で戻す)
    }
    state_ = stack_.back();
    stack_.pop_back();
    plutovg_canvas_restore(canvas_);
}

void RasterCanvas::begin_layer(float opacity)
{
    save();
    Layer* layer = new Layer();
    const PixelRect area = state_.clip;
    layer->x = area.x0;
    layer->y = area.y0;
    layer->width = std::max(1, area.x1 - area.x0);
    layer->height = std::max(1, area.y1 - area.y0);
    layer->opacity = std::max(0.0f, std::min(opacity, 1.0f));
    layer->depth = stack_.size();
    layer->pixels.assign(static_cast<std::size_t>(layer->width) * static_cast<std::size_t>(layer->height), 0);
    layer->surface = plutovg_surface_create_for_data(reinterpret_cast<unsigned char*>(&layer->pixels[0]), layer->width,
                                                     layer->height, layer->width * 4);
    layer->canvas = plutovg_canvas_create(layer->surface);
    plutovg_canvas_translate(layer->canvas, static_cast<float>(-layer->x), static_cast<float>(-layer->y));
    layer->parent_surface = surface_;
    layer->parent_canvas = canvas_;
    layer->parent_buffer = buffer_;
    layer->parent_width = width_;
    layer->parent_height = height_;
    layer->parent_x = origin_x_;
    layer->parent_y = origin_y_;
    layers_.push_back(layer);
    surface_ = layer->surface;
    canvas_ = layer->canvas;
    buffer_ = &layer->pixels[0];
    width_ = layer->width;
    height_ = layer->height;
    origin_x_ = layer->x;
    origin_y_ = layer->y;
    // 一時的な画像の外には描けないので、クリップは矩形のまま(元の描き先のパスのクリップは、重ねるときに効く)。
    state_.clip_is_rect = true;
    plutovg_canvas_save(canvas_);
    apply_clip();
}

void RasterCanvas::end_layer()
{
    if (layers_.empty())
    {
        return;
    }
    Layer* layer = layers_.back();
    while (stack_.size() > layer->depth)
    {
        restore();
    }
    plutovg_canvas_restore(canvas_);
    layers_.pop_back();
    surface_ = layer->parent_surface;
    canvas_ = layer->parent_canvas;
    buffer_ = layer->parent_buffer;
    width_ = layer->parent_width;
    height_ = layer->parent_height;
    origin_x_ = layer->parent_x;
    origin_y_ = layer->parent_y;
    // 一時的な画像を、元の描き先の同じ位置に opacity で重ねる(元の描き先のクリップが効く)。
    if (layer->opacity > 0)
    {
        plutovg_matrix_t m;
        plutovg_matrix_init_translate(&m, static_cast<float>(layer->x), static_cast<float>(layer->y));
        plutovg_canvas_set_texture(canvas_, layer->surface, PLUTOVG_TEXTURE_TYPE_PLAIN, layer->opacity, &m);
        plutovg_canvas_fill_rect(canvas_, static_cast<float>(layer->x), static_cast<float>(layer->y),
                                 static_cast<float>(layer->width), static_cast<float>(layer->height));
    }
    plutovg_canvas_destroy(layer->canvas);
    plutovg_surface_destroy(layer->surface);
    delete layer;
    restore();  // begin_layer の save
}

void RasterCanvas::translate(float dx, float dy)
{
    state_.e += state_.a * dx + state_.c * dy;
    state_.f += state_.b * dx + state_.d * dy;
}

void RasterCanvas::scale_by(float sx, float sy)
{
    state_.a *= sx;
    state_.b *= sx;
    state_.c *= sy;
    state_.d *= sy;
}

void RasterCanvas::rotate(float radians)
{
    const float cs = std::cos(radians), sn = std::sin(radians);
    const float a = state_.a, b = state_.b, c = state_.c, d = state_.d;
    state_.a = a * cs + c * sn;
    state_.b = b * cs + d * sn;
    state_.c = c * cs - a * sn;
    state_.d = d * cs - b * sn;
}

float RasterCanvas::linear_scale() const { return std::sqrt(std::fabs(state_.a * state_.d - state_.b * state_.c)); }

PixelRect RasterCanvas::snap(const Rect& r) const
{
    const PixelRect p = {round_px(px(r.x, r.y)), round_px(py(r.x, r.y)), round_px(px(r.right(), r.bottom())),
                         round_px(py(r.right(), r.bottom()))};
    return p;
}

void RasterCanvas::clip_rect(const Rect& r)
{
    if (axis_aligned())
    {
        state_.clip = intersect(state_.clip, snap(r));
        apply_clip();
        return;
    }
    Path p;
    p.add_rect(r);
    clip_path(p);
}

void RasterCanvas::clip_path(const Path& path)
{
    PixelRect bounds;
    set_path(path, &bounds);
    plutovg_canvas_clip(canvas_);
    state_.clip = intersect(state_.clip, bounds);
    state_.clip_is_rect = false;
}

Rect RasterCanvas::clip_bounds() const
{
    const PixelRect& c = state_.clip;
    if (axis_aligned())
    {
        return Rect(c.x0 / scale_ - state_.e, c.y0 / scale_ - state_.f, (c.x1 - c.x0) / scale_, (c.y1 - c.y0) / scale_);
    }
    // クリップの矩形の角を、今の座標系に戻して囲む。
    const float det = state_.a * state_.d - state_.b * state_.c;
    if (det == 0)
    {
        return Rect();
    }
    float x0 = 1e30f, y0 = 1e30f, x1 = -1e30f, y1 = -1e30f;
    const float xs[2] = {static_cast<float>(c.x0), static_cast<float>(c.x1)};
    const float ys[2] = {static_cast<float>(c.y0), static_cast<float>(c.y1)};
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            const float u = xs[i] / scale_ - state_.e, v = ys[j] / scale_ - state_.f;
            extend((state_.d * u - state_.c * v) / det, (-state_.b * u + state_.a * v) / det, x0, y0, x1, y1);
        }
    }
    return Rect(x0, y0, x1 - x0, y1 - y0);
}

// ---- 描画 ----

void RasterCanvas::set_path(const Path& path, PixelRect* bounds)
{
    float x0 = 1e30f, y0 = 1e30f, x1 = -1e30f, y1 = -1e30f;
    plutovg_canvas_new_path(canvas_);
    const std::vector<Path::Element>& els = path.elements();
    for (std::size_t i = 0; i < els.size(); ++i)
    {
        const float* p = els[i].pts;
        const int points = els[i].op == Path::Op::CUBIC_TO ? 3 : els[i].op == Path::Op::CLOSE ? 0 : 1;
        float q[6];
        for (int k = 0; k < points; ++k)
        {
            q[k * 2] = px(p[k * 2], p[k * 2 + 1]);
            q[k * 2 + 1] = py(p[k * 2], p[k * 2 + 1]);
            extend(q[k * 2], q[k * 2 + 1], x0, y0, x1, y1);
        }
        switch (els[i].op)
        {
        case Path::Op::MOVE_TO:
            plutovg_canvas_move_to(canvas_, q[0], q[1]);
            break;
        case Path::Op::LINE_TO:
            plutovg_canvas_line_to(canvas_, q[0], q[1]);
            break;
        case Path::Op::CUBIC_TO:
            plutovg_canvas_cubic_to(canvas_, q[0], q[1], q[2], q[3], q[4], q[5]);
            break;
        case Path::Op::CLOSE:
            plutovg_canvas_close_path(canvas_);
            break;
        }
    }
    if (bounds)
    {
        if (x1 < x0)
        {
            const PixelRect none = {0, 0, 0, 0};
            *bounds = none;
        }
        else
        {
            const PixelRect r = {static_cast<int>(std::floor(x0)), static_cast<int>(std::floor(y0)),
                                 static_cast<int>(std::ceil(x1)), static_cast<int>(std::ceil(y1))};
            *bounds = r;
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
    // グラデーションは dp の座標で指定し、変換と倍率は行列で渡す(回転・拡大しても形が正しい)。
    plutovg_matrix_t m;
    plutovg_matrix_init(&m, state_.a * scale_, state_.b * scale_, state_.c * scale_, state_.d * scale_,
                        state_.e * scale_, state_.f * scale_);
    if (paint.type == Paint::Type::LINEAR)
    {
        plutovg_canvas_set_linear_gradient(canvas_, paint.p0.x, paint.p0.y, paint.p1.x, paint.p1.y,
                                           PLUTOVG_SPREAD_METHOD_PAD, data, n, &m);
    }
    else
    {
        plutovg_canvas_set_radial_gradient(canvas_, paint.p0.x, paint.p0.y, paint.radius, paint.p1.x, paint.p1.y, 0,
                                           PLUTOVG_SPREAD_METHOD_PAD, data, n, &m);
    }
}

void RasterCanvas::set_stroke(const StrokeStyle& style)
{
    const float k = scale_ * linear_scale();
    plutovg_canvas_set_line_width(canvas_, style.width * k);
    plutovg_line_cap_t cap = PLUTOVG_LINE_CAP_BUTT;
    if (style.cap == LineCap::ROUND)
    {
        cap = PLUTOVG_LINE_CAP_ROUND;
    }
    else if (style.cap == LineCap::SQUARE)
    {
        cap = PLUTOVG_LINE_CAP_SQUARE;
    }
    plutovg_canvas_set_line_cap(canvas_, cap);
    plutovg_line_join_t join = PLUTOVG_LINE_JOIN_MITER;
    if (style.join == LineJoin::ROUND)
    {
        join = PLUTOVG_LINE_JOIN_ROUND;
    }
    else if (style.join == LineJoin::BEVEL)
    {
        join = PLUTOVG_LINE_JOIN_BEVEL;
    }
    plutovg_canvas_set_line_join(canvas_, join);
    if (style.dashes.empty())
    {
        plutovg_canvas_set_dash(canvas_, 0, nullptr, 0);
    }
    else
    {
        std::vector<float> dashes(style.dashes.size());
        for (std::size_t i = 0; i < dashes.size(); ++i)
        {
            dashes[i] = style.dashes[i] * k;
        }
        plutovg_canvas_set_dash(canvas_, style.dash_offset * k, &dashes[0], static_cast<int>(dashes.size()));
    }
}

void RasterCanvas::fill_rect(const Rect& r, const Color& color)
{
    if (!axis_aligned())
    {
        Path p;
        p.add_rect(r);
        fill_path(p, color);
        return;
    }
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
    if (!axis_aligned())
    {
        Path p;
        p.add_round_rect(r, radius);
        fill_path(p, paint);
        return;
    }
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
    if (!axis_aligned())
    {
        // 線を r の内側に描く(線の中心を太さの半分だけ内側へ)。
        Path p;
        p.add_round_rect(r.adjusted(width / 2, width / 2, width / 2, width / 2), std::max(0.0f, radius - width / 2));
        stroke_path(p, StrokeStyle(width), color);
        return;
    }
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
    plutovg_canvas_set_line_join(canvas_, PLUTOVG_LINE_JOIN_MITER);
    plutovg_canvas_set_dash(canvas_, 0, nullptr, 0);
    plutovg_canvas_stroke(canvas_);
}

void RasterCanvas::fill_path(const Path& path, const Paint& paint)
{
    if (path.is_empty())
    {
        return;
    }
    set_path(path);
    set_paint(paint);
    plutovg_canvas_fill(canvas_);
}

void RasterCanvas::stroke_path(const Path& path, const StrokeStyle& style, const Paint& paint)
{
    if (path.is_empty())
    {
        return;
    }
    set_path(path);
    set_paint(paint);
    set_stroke(style);
    plutovg_canvas_stroke(canvas_);
}

void RasterCanvas::draw_image(const Image& image, const Rect& dst, float opacity)
{
    if (image.is_null() || dst.width <= 0 || dst.height <= 0 || opacity <= 0)
    {
        return;
    }
    if (image.is_icon())
    {
        // アイコンは線画で描く(色を決めていなければ黒)
        api_->draw_image(image, dst, Color(0, 0, 0), opacity);
        return;
    }
    plutovg_matrix_t m;
    PixelRect area;
    if (axis_aligned())
    {
        // ピクセルに揃えた dst へ写す。大きさが違えば、先に滑らかに写した画像(覚えておく)を使う
        // (plutovg の拡大・縮小は最も近い画素を取るだけで、ぎざぎざになるため)。
        area = snap(dst);
        if (area.empty())
        {
            return;
        }
        const int w = area.x1 - area.x0, h = area.y1 - area.y0;
        plutovg_canvas_new_path(canvas_);
        plutovg_canvas_rect(canvas_, static_cast<float>(area.x0), static_cast<float>(area.y0), static_cast<float>(w),
                            static_cast<float>(h));
        if (w != image.width() || h != image.height())
        {
            plutovg_surface_t* s = image.data()->scaled(w, h);
            if (s)
            {
                plutovg_matrix_init_translate(&m, static_cast<float>(area.x0), static_cast<float>(area.y0));
                plutovg_canvas_set_texture(canvas_, s, PLUTOVG_TEXTURE_TYPE_PLAIN, std::min(opacity, 1.0f), &m);
                plutovg_canvas_fill(canvas_);
                plutovg_surface_destroy(s);
                return;
            }
        }
        plutovg_matrix_init(&m, static_cast<float>(w) / image.width(), 0, 0, static_cast<float>(h) / image.height(),
                            static_cast<float>(area.x0), static_cast<float>(area.y0));
    }
    else
    {
        // 画像の画素 → dst(dp)→ 変換 → ピクセル。
        const float sx = dst.width / image.width(), sy = dst.height / image.height();
        const float k = scale_;
        plutovg_matrix_init(&m, state_.a * sx * k, state_.b * sx * k, state_.c * sy * k, state_.d * sy * k,
                            (state_.a * dst.x + state_.c * dst.y + state_.e) * k,
                            (state_.b * dst.x + state_.d * dst.y + state_.f) * k);
        Path p;
        p.add_rect(dst);
        set_path(p);
    }
    plutovg_canvas_set_texture(canvas_, image.data()->surface, PLUTOVG_TEXTURE_TYPE_PLAIN, std::min(opacity, 1.0f), &m);
    plutovg_canvas_fill(canvas_);
}

// ---- 文字 ----

// グリフの被覆率の補正(docs/adr/0020)。sRGB の値をそのまま線形に混ぜると、文字が細く見える(明るい文字では特に)。
// 被覆率 c を c^(1/k) に持ち上げて補う。k は文字色の明るさで、暗い文字 TEXT_BOOST_DARK から明るい文字 TEXT_BOOST_LIGHT まで変える。
// 環境変数 TOKOTACHI_TEXT_BOOST で k を固定できる(見え方の確認用。1 で補正なし)。
const float TEXT_BOOST_DARK = 1.25f;
const float TEXT_BOOST_LIGHT = 1.6f;

void text_coverage_table(const Color& color, std::uint8_t* table)
{
    const float luminance = (0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b) / 255.0f;
    float k = TEXT_BOOST_DARK + (TEXT_BOOST_LIGHT - TEXT_BOOST_DARK) * luminance;
    if (const char* fixed = std::getenv("TOKOTACHI_TEXT_BOOST"))
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
        // 今の描き先の中の位置(レイヤーなら、その左上を引く)。
        std::uint32_t* row = buffer_ + static_cast<std::size_t>(y - origin_y_) * static_cast<std::size_t>(width_);
        const std::uint8_t* cov = &g.coverage[static_cast<std::size_t>(y - gy) * static_cast<std::size_t>(g.width)];
        for (int x = x_begin; x < x_end; ++x)
        {
            const std::uint32_t a = table[cov[x - gx]] * sa / 255;
            if (a == 0)
            {
                continue;
            }
            // アルファ乗算済みの source-over(ガンマ補正なし。docs/adr/0007)
            std::uint32_t& dst = row[x - origin_x_];
            const std::uint32_t d32 = dst;
            const std::uint32_t ia = 255 - a;
            const std::uint32_t da = (d32 >> 24) & 0xff, dr = (d32 >> 16) & 0xff, dg = (d32 >> 8) & 0xff,
                                db = d32 & 0xff;
            const std::uint32_t oa = a + (da * ia + 127) / 255;
            const std::uint32_t orr = (sr * a + dr * ia + 127) / 255;
            const std::uint32_t og = (sg * a + dg * ia + 127) / 255;
            const std::uint32_t ob = (sb * a + db * ia + 127) / 255;
            dst = (oa << 24) | (orr << 16) | (og << 8) | ob;
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
    if (!axis_aligned() || !state_.clip_is_rect)
    {
        draw_text_outlines(layout, pos, color);
        return;
    }
    const PixelRect& clip = state_.clip;
    std::uint8_t table[256];
    text_coverage_table(color, table);
    for (std::size_t li = 0; li < d->lines.size(); ++li)
    {
        const LineData& line = d->lines[li];
        const float ox = std::floor(px(pos.x, pos.y) + line.x + 0.5f);
        const float baseline = std::floor(py(pos.x, pos.y) + line.y + d->ascent + 0.5f);
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

void RasterCanvas::draw_text_outlines(const TextLayout& layout, const Point& pos, const Color& color)
{
    // レイアウトはピクセル(レイアウトの倍率)で持っているので、dp に戻してから今の変換で写す。
    const TextLayoutData* d = layout.data();
    const float ls = d->scale > 0 ? d->scale : 1;
    Path all;
    for (std::size_t li = 0; li < d->lines.size(); ++li)
    {
        const LineData& line = d->lines[li];
        for (std::size_t r = 0; r < line.runs.size(); ++r)
        {
            const GlyphRun& run = line.runs[r];
            for (std::size_t i = 0; i < run.glyphs.size(); ++i)
            {
                const Path& outline = run.font->outline(run.glyphs[i].id);
                const float ox = pos.x + (line.x + run.glyphs[i].x) / ls;
                const float oy = pos.y + (line.y + d->ascent + run.glyphs[i].y) / ls;
                const std::vector<Path::Element>& els = outline.elements();
                for (std::size_t k = 0; k < els.size(); ++k)
                {
                    const float* p = els[k].pts;
                    switch (els[k].op)
                    {
                    case Path::Op::MOVE_TO:
                        all.move_to(ox + p[0] / ls, oy + p[1] / ls);
                        break;
                    case Path::Op::LINE_TO:
                        all.line_to(ox + p[0] / ls, oy + p[1] / ls);
                        break;
                    case Path::Op::CUBIC_TO:
                        all.cubic_to(ox + p[0] / ls, oy + p[1] / ls, ox + p[2] / ls, oy + p[3] / ls, ox + p[4] / ls,
                                     oy + p[5] / ls);
                        break;
                    case Path::Op::CLOSE:
                        all.close();
                        break;
                    }
                }
            }
        }
    }
    fill_path(all, color);
}

}  // namespace detail
}  // namespace tokotachi
