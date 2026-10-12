// 色を選ぶ部品の共通の部分(内部。docs/adr/0051): 16 進数の文字、HSV、透過を見せる市松模様。
#ifndef TOKOTACHI_SRC_WIDGETS_COLOR_UTIL_HPP
#define TOKOTACHI_SRC_WIDGETS_COLOR_UTIL_HPP

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include <tokotachi/core/geometry.hpp>
#include <tokotachi/graphics/canvas.hpp>

namespace tokotachi
{
namespace detail
{

// 「#RRGGBB」(alpha なら「#RRGGBBAA」)。
inline std::string color_to_hex(const Color& c, bool alpha)
{
    char buf[16];
    if (alpha)
    {
        std::snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", c.r, c.g, c.b, c.a);
    }
    else
    {
        std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", c.r, c.g, c.b);
    }
    return buf;
}

// 「#RGB」「#RRGGBB」「#RRGGBBAA」(# は無くてもよい。前後の空白は無視)を読む。読めれば true。
inline bool color_from_hex(const std::string& text, Color& out)
{
    const std::size_t first = text.find_first_not_of(" \t");
    if (first == std::string::npos)
    {
        return false;
    }
    std::string s = text.substr(first, text.find_last_not_of(" \t") - first + 1);
    if (!s.empty() && s[0] == '#')
    {
        s.erase(0, 1);
    }
    unsigned v[8];
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        const char c = s[i];
        if (i >= 8)
        {
            return false;
        }
        if (c >= '0' && c <= '9')
        {
            v[i] = static_cast<unsigned>(c - '0');
        }
        else if (c >= 'a' && c <= 'f')
        {
            v[i] = static_cast<unsigned>(c - 'a' + 10);
        }
        else if (c >= 'A' && c <= 'F')
        {
            v[i] = static_cast<unsigned>(c - 'A' + 10);
        }
        else
        {
            return false;
        }
    }
    if (s.size() == 3)
    {
        out = Color(static_cast<std::uint8_t>(v[0] * 17), static_cast<std::uint8_t>(v[1] * 17), static_cast<std::uint8_t>(v[2] * 17));
        return true;
    }
    if (s.size() == 6 || s.size() == 8)
    {
        out = Color(static_cast<std::uint8_t>(v[0] * 16 + v[1]), static_cast<std::uint8_t>(v[2] * 16 + v[3]),
                    static_cast<std::uint8_t>(v[4] * 16 + v[5]),
                    s.size() == 8 ? static_cast<std::uint8_t>(v[6] * 16 + v[7]) : static_cast<std::uint8_t>(255));
        return true;
    }
    return false;
}

// HSV(h は 0〜360、s・v は 0〜1)から色。
inline Color color_from_hsv(float h, float s, float v, std::uint8_t alpha = 255)
{
    h = std::fmod(std::max(0.0f, h), 360.0f) / 60.0f;
    const int i = static_cast<int>(std::floor(h));
    const float f = h - i;
    const float p = v * (1 - s), q = v * (1 - s * f), t = v * (1 - s * (1 - f));
    float r = v, g = t, b = p;
    switch (i)
    {
    case 1:
        r = q, g = v, b = p;
        break;
    case 2:
        r = p, g = v, b = t;
        break;
    case 3:
        r = p, g = q, b = v;
        break;
    case 4:
        r = t, g = p, b = v;
        break;
    case 5:
        r = v, g = p, b = q;
        break;
    default:
        break;
    }
    const auto byte = [](float x) { return static_cast<std::uint8_t>(std::floor(std::max(0.0f, std::min(1.0f, x)) * 255 + 0.5f)); };
    return Color(byte(r), byte(g), byte(b), alpha);
}

// 色から HSV。灰色(彩度 0)なら h は変えない(色相を覚えておくため)。
inline void color_to_hsv(const Color& c, float& h, float& s, float& v)
{
    const float r = c.r / 255.0f, g = c.g / 255.0f, b = c.b / 255.0f;
    const float mx = std::max(r, std::max(g, b)), mn = std::min(r, std::min(g, b));
    const float d = mx - mn;
    v = mx;
    s = mx > 0 ? d / mx : 0;
    if (d <= 0)
    {
        return;
    }
    float hh;
    if (mx == r)
    {
        hh = std::fmod((g - b) / d, 6.0f);
    }
    else if (mx == g)
    {
        hh = (b - r) / d + 2;
    }
    else
    {
        hh = (r - g) / d + 4;
    }
    h = hh * 60;
    if (h < 0)
    {
        h += 360;
    }
}

// 透過を見せる市松模様(r の中)。
inline void paint_checker(Canvas& canvas, const Rect& r, float cell = 6)
{
    canvas.save();
    canvas.clip_rect(r);
    canvas.fill_rect(r, Color(255, 255, 255));
    for (float y = r.y; y < r.bottom(); y += cell)
    {
        const int row = static_cast<int>((y - r.y) / cell);
        for (float x = r.x + (row % 2) * cell; x < r.right(); x += cell * 2)
        {
            canvas.fill_rect(Rect(x, y, cell, cell), Color(0xCC, 0xCC, 0xCC));
        }
    }
    canvas.restore();
}

}  // namespace detail
}  // namespace tokotachi

#endif
