// Canvas の描画先(内部。docs/adr/0060)。公開の Canvas の描く関数は、ここへ処理を渡す。
// 描画先の関数は公開の ABI に含まれないので、後から足してよい。
#ifndef TOKOTACHI_SRC_GRAPHICS_CANVAS_IMPL_HPP
#define TOKOTACHI_SRC_GRAPHICS_CANVAS_IMPL_HPP

#include <tokotachi/graphics/canvas.hpp>

#include "../core/impl.hpp"

namespace tokotachi
{

class Canvas::Impl
{
public:
    virtual ~Impl() {}

    virtual float scale() const = 0;
    virtual void save() = 0;
    virtual void restore() = 0;
    virtual void translate(float dx, float dy) = 0;
    virtual void scale_by(float sx, float sy) = 0;
    virtual void rotate(float radians) = 0;
    virtual void clip_rect(const Rect& r) = 0;
    virtual void clip_path(const Path& path) = 0;
    virtual Rect clip_bounds() const = 0;
    virtual void begin_layer(float opacity) = 0;
    virtual void end_layer() = 0;
    virtual void fill_rect(const Rect& r, const Color& color) = 0;
    virtual void fill_round_rect(const Rect& r, float radius, const Paint& paint) = 0;
    virtual void stroke_round_rect(const Rect& r, float radius, float width, const Color& color) = 0;
    virtual void fill_path(const Path& path, const Paint& paint) = 0;
    virtual void stroke_path(const Path& path, const StrokeStyle& style, const Paint& paint) = 0;
    virtual void draw_text(const TextLayout& layout, const Point& pos, const Color& color) = 0;
    virtual void draw_image(const Image& image, const Rect& dst, float opacity) = 0;
};

namespace detail
{

template <>
struct ImplAccess<Canvas>
{
    TOKOTACHI_IMPL_ACCESS(Canvas)

    /// backend に描く Canvas を作る(呼んだ側が破棄する)。
    static Canvas* create(Impl* backend) { return new Canvas(backend); }
};

}  // namespace detail
}  // namespace tokotachi

#endif
