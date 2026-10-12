// Canvas の手軽な描画(基本の描画を組み合わせる)。
#include <tokotachi/graphics/canvas.hpp>

#include <tokotachi/graphics/image.hpp>
#include <tokotachi/graphics/text_layout.hpp>

#include <algorithm>
#include <cstdint>

#include "icon_data.hpp"
#include "image_data.hpp"

namespace tokotachi
{

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
