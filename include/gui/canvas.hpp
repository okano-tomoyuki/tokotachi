// 描画の抽象(docs/adr/0006)。座標は dp で、実装が倍率を掛けてピクセルに直す。
// 矩形の描画は辺をピクセルの整数に揃える(分数の倍率でもにじまないように。docs/adr/0008)。
#ifndef GUI_CANVAS_HPP
#define GUI_CANVAS_HPP

#include <vector>

#include "geometry.hpp"

namespace gui
{

class Image;
class TextLayout;

class Path
{
public:
    enum class Op
    {
        MOVE_TO,
        LINE_TO,
        CUBIC_TO,
        CLOSE
    };

    struct Element
    {
        Op op;
        float pts[6];
    };

    void move_to(float x, float y);
    void line_to(float x, float y);
    void cubic_to(float x1, float y1, float x2, float y2, float x3, float y3);
    void close();
    void add_rect(const Rect& r);
    void add_round_rect(const Rect& r, float radius);
    void add_ellipse(const Point& center, float rx, float ry);

    bool empty() const { return elements_.empty(); }
    const std::vector<Element>& elements() const { return elements_; }

private:
    std::vector<Element> elements_;
};

struct GradientStop
{
    float offset;
    Color color;

    GradientStop() : offset(0) {}
    GradientStop(float offset_, const Color& color_) : offset(offset_), color(color_) {}
};

// 塗り(単色・線形グラデーション・放射グラデーション)。
struct Paint
{
    enum class Type
    {
        SOLID,
        LINEAR,
        RADIAL
    };

    Type type;
    Color color;
    Point p0, p1;  // LINEAR: 始点・終点。RADIAL: p0 が中心、p1 が焦点
    float radius;  // RADIAL
    std::vector<GradientStop> stops;

    Paint() : type(Type::SOLID), radius(0) {}
    // 色から暗黙に作れるようにする(canvas.fill_round_rect(r, 4, color))。
    Paint(const Color& c) : type(Type::SOLID), color(c), radius(0) {}

    static Paint linear(const Point& from, const Point& to, const std::vector<GradientStop>& stops);
    static Paint radial(const Point& center, float radius, const Point& focal, const std::vector<GradientStop>& stops);
};

enum class LineCap
{
    BUTT,
    ROUND,
    SQUARE
};

class Canvas
{
public:
    virtual ~Canvas() {}

    // dp からピクセルへの倍率。
    virtual float scale() const = 0;

    virtual void save() = 0;
    virtual void restore() = 0;
    virtual void translate(float dx, float dy) = 0;
    // 現在のクリップと r の共通部分に描画を限る。
    virtual void clip_rect(const Rect& r) = 0;
    // 現在のクリップ(dp、現在の原点からの相対)。
    virtual Rect clip_bounds() const = 0;

    virtual void fill_rect(const Rect& r, const Color& color) = 0;
    virtual void fill_round_rect(const Rect& r, float radius, const Paint& paint) = 0;
    // 線を r の内側に描く(線の外側の辺が r に一致する)。
    virtual void stroke_round_rect(const Rect& r, float radius, float width, const Color& color) = 0;
    virtual void fill_path(const Path& path, const Paint& paint) = 0;
    virtual void stroke_path(const Path& path, float width, const Paint& paint, LineCap cap = LineCap::BUTT) = 0;
    // pos はレイアウトの左上(ベースラインは pos.y + layout.ascent())。
    virtual void draw_text(const TextLayout& layout, const Point& pos, const Color& color) = 0;
    // 画像を矩形 dst(dp)に合わせて伸縮して描く。opacity は 0〜1(無効な部品のアイコンを薄くするときなど)。
    virtual void draw_image(const Image& image, const Rect& dst, float opacity = 1) = 0;
};

}  // namespace gui

#endif
