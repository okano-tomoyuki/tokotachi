/// @file
/// 幾何の型と色。座標はすべて dp(docs/adr/0008)。値の型なので、プロパティではなく公開のメンバ変数で表す(docs/adr/0013)。
#ifndef TOKOTACHI_CORE_GEOMETRY_HPP
#define TOKOTACHI_CORE_GEOMETRY_HPP

#include <algorithm>
#include <cstdint>

namespace tokotachi
{

/// 点(dp)。
///
/// @ingroup core_values
struct Point
{
    float x, y;

    Point() : x(0), y(0) {}
    Point(float x_, float y_) : x(x_), y(y_) {}

    Point operator+(const Point& o) const { return Point(x + o.x, y + o.y); }
    Point operator-(const Point& o) const { return Point(x - o.x, y - o.y); }
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Point& o) const { return !(*this == o); }
};

/// 大きさ(dp)。
///
/// @ingroup core_values
struct Size
{
    float width, height;

    Size() : width(0), height(0) {}
    Size(float w, float h) : width(w), height(h) {}

    bool operator==(const Size& o) const { return width == o.width && height == o.height; }
    bool operator!=(const Size& o) const { return !(*this == o); }
};

/// 矩形(左上の点と大きさ。dp)。
///
/// @ingroup core_values
struct Rect
{
    float x, y, width, height;

    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(float x_, float y_, float w, float h) : x(x_), y(y_), width(w), height(h) {}
    Rect(const Point& pos, const Size& size) : x(pos.x), y(pos.y), width(size.width), height(size.height) {}

    float right() const { return x + width; }
    float bottom() const { return y + height; }
    Point position() const { return Point(x, y); }
    Size size() const { return Size(width, height); }
    Point center() const { return Point(x + width / 2, y + height / 2); }

    bool is_empty() const { return width <= 0 || height <= 0; }
    bool contains(const Point& p) const { return p.x >= x && p.y >= y && p.x < x + width && p.y < y + height; }

    bool intersects(const Rect& o) const
    {
        return !is_empty() && !o.is_empty() && x < o.right() && o.x < right() && y < o.bottom() && o.y < bottom();
    }

    Rect intersected(const Rect& o) const
    {
        const float l = std::max(x, o.x), t = std::max(y, o.y);
        const float r = std::min(right(), o.right()), b = std::min(bottom(), o.bottom());
        return r > l && b > t ? Rect(l, t, r - l, b - t) : Rect();
    }

    Rect united(const Rect& o) const
    {
        if (is_empty())
        {
            return o;
        }
        if (o.is_empty())
        {
            return *this;
        }
        const float l = std::min(x, o.x), t = std::min(y, o.y);
        const float r = std::max(right(), o.right()), b = std::max(bottom(), o.bottom());
        return Rect(l, t, r - l, b - t);
    }

    Rect translated(float dx, float dy) const { return Rect(x + dx, y + dy, width, height); }

    /// 各辺を内側へ動かす(負の値なら外側へ)。
    Rect adjusted(float dl, float dt, float dr, float db) const
    {
        return Rect(x + dl, y + dt, width - dl - dr, height - dt - db);
    }

    bool operator==(const Rect& o) const { return x == o.x && y == o.y && width == o.width && height == o.height; }
    bool operator!=(const Rect& o) const { return !(*this == o); }
};

/// 色(sRGB の 8 ビットの成分と、アルファ。アルファは乗算していない)。
///
/// @ingroup core_values
struct Color
{
    std::uint8_t r, g, b, a;

    Color() : r(0), g(0), b(0), a(255) {}
    Color(std::uint8_t r_, std::uint8_t g_, std::uint8_t b_, std::uint8_t a_ = 255) : r(r_), g(g_), b(b_), a(a_) {}

    /// 0xRRGGBB から作る。
    static Color rgb(std::uint32_t hex, std::uint8_t alpha = 255)
    {
        return Color(static_cast<std::uint8_t>(hex >> 16), static_cast<std::uint8_t>(hex >> 8),
                     static_cast<std::uint8_t>(hex), alpha);
    }

    Color with_alpha(std::uint8_t alpha) const { return Color(r, g, b, alpha); }

    bool operator==(const Color& o) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
    bool operator!=(const Color& o) const { return !(*this == o); }
};

}  // namespace tokotachi

#endif
