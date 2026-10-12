// Canvas の手軽な描画(基本の描画を組み合わせる)。
#include <tokotachi/graphics/canvas.hpp>

#include <tokotachi/graphics/image.hpp>
#include <tokotachi/graphics/text_layout.hpp>

#include <algorithm>
#include <cstdint>

#include "canvas_impl.hpp"
#include "icon_data.hpp"
#include "image_data.hpp"

namespace tokotachi
{

// ---- 描画先へ処理を渡す ----

float Canvas::scale() const { return d_->scale(); }

void Canvas::save() { d_->save(); }

void Canvas::restore() { d_->restore(); }

void Canvas::translate(float dx, float dy) { d_->translate(dx, dy); }

void Canvas::scale_by(float sx, float sy) { d_->scale_by(sx, sy); }

void Canvas::rotate(float radians) { d_->rotate(radians); }

void Canvas::clip_rect(const Rect& r) { d_->clip_rect(r); }

void Canvas::clip_path(const Path& path) { d_->clip_path(path); }

Rect Canvas::clip_bounds() const { return d_->clip_bounds(); }

void Canvas::begin_layer(float opacity) { d_->begin_layer(opacity); }

void Canvas::end_layer() { d_->end_layer(); }

void Canvas::fill_rect(const Rect& r, const Color& color) { d_->fill_rect(r, color); }

void Canvas::fill_round_rect(const Rect& r, float radius, const Paint& paint) { d_->fill_round_rect(r, radius, paint); }

void Canvas::stroke_round_rect(const Rect& r, float radius, float width, const Color& color) { d_->stroke_round_rect(r, radius, width, color); }

void Canvas::fill_path(const Path& path, const Paint& paint) { d_->fill_path(path, paint); }

void Canvas::stroke_path(const Path& path, const StrokeStyle& style, const Paint& paint) { d_->stroke_path(path, style, paint); }

void Canvas::draw_text(const TextLayout& layout, const Point& pos, const Color& color) { d_->draw_text(layout, pos, color); }

void Canvas::draw_image(const Image& image, const Rect& dst, float opacity) { d_->draw_image(image, dst, opacity); }

void Canvas::draw_image(const Image& image, const Rect& dst, const Color& tint, float opacity)
{
    if (!image.is_icon())
    {
        draw_image(image, dst, opacity);
        return;
    }
    const detail::ImageData* d = image.data();
    const Color c = d->has_color ? d->color : tint;
    const float a = std::max(0.0f, std::min(1.0f, opacity)) * c.a;
    detail::draw_icon(*this, d->icon, dst, c.with_alpha(static_cast<std::uint8_t>(a + 0.5f)));
}

void Canvas::stroke_path(const Path& path, float width, const Paint& paint, LineCap cap)
{
    stroke_path(path, StrokeStyle(width, cap), paint);
}

void Canvas::draw_line(const Point& from, const Point& to, float width, const Paint& paint, LineCap cap)
{
    draw_line(from, to, StrokeStyle(width, cap), paint);
}

void Canvas::draw_line(const Point& from, const Point& to, const StrokeStyle& style, const Paint& paint)
{
    Path p;
    p.move_to(from.x, from.y);
    p.line_to(to.x, to.y);
    stroke_path(p, style, paint);
}

void Canvas::stroke_rect(const Rect& r, float width, const Color& color) { stroke_round_rect(r, 0, width, color); }

void Canvas::fill_ellipse(const Rect& r, const Paint& paint)
{
    Path p;
    p.add_ellipse(Point(r.x + r.width / 2, r.y + r.height / 2), r.width / 2, r.height / 2);
    fill_path(p, paint);
}

void Canvas::stroke_ellipse(const Rect& r, const StrokeStyle& style, const Paint& paint)
{
    Path p;
    p.add_ellipse(Point(r.x + r.width / 2, r.y + r.height / 2), r.width / 2, r.height / 2);
    stroke_path(p, style, paint);
}

void Canvas::draw_text(const std::string& text, const Point& pos, const Color& color)
{
    draw_text(text, pos, color, Font());
}

void Canvas::draw_text(const std::string& text, const Point& pos, const Color& color, const Font& font)
{
    draw_text(TextLayout(text, font, scale()), pos, color);
}

}  // namespace tokotachi
