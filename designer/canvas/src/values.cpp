#include "values.hpp"

#include <cstdio>

namespace ttc
{

namespace
{

/// オブジェクトのメンバの数値(無ければ fallback のまま)。数値でなければ false。
bool member(const Json& j, const char* name, float& out)
{
    if (!j.has(name))
    {
        return true;
    }
    return from_json(j[name], out);
}

int hex_digit(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    return -1;
}

}  // namespace

bool from_json(const Json& j, bool& out)
{
    if (!j.is_bool())
    {
        return false;
    }
    out = j.as_bool();
    return true;
}

bool from_json(const Json& j, int& out)
{
    if (!j.is_number())
    {
        return false;
    }
    out = static_cast<int>(j.as_number());
    return true;
}

bool from_json(const Json& j, float& out)
{
    if (!j.is_number())
    {
        return false;
    }
    out = static_cast<float>(j.as_number());
    return true;
}

bool from_json(const Json& j, double& out)
{
    if (!j.is_number())
    {
        return false;
    }
    out = j.as_number();
    return true;
}

bool from_json(const Json& j, std::string& out)
{
    if (!j.is_string())
    {
        return false;
    }
    out = j.as_string();
    return true;
}

bool from_json(const Json& j, std::vector<std::string>& out)
{
    if (!j.is_array())
    {
        return false;
    }
    out.clear();
    for (std::size_t i = 0; i < j.items().size(); ++i)
    {
        std::string s;
        if (!from_json(j.items()[i], s))
        {
            return false;
        }
        out.push_back(s);
    }
    return true;
}

bool from_json(const Json& j, std::vector<float>& out)
{
    if (!j.is_array())
    {
        return false;
    }
    out.clear();
    for (std::size_t i = 0; i < j.items().size(); ++i)
    {
        float f = 0;
        if (!from_json(j.items()[i], f))
        {
            return false;
        }
        out.push_back(f);
    }
    return true;
}

bool from_json(const Json& j, tokotachi::Margins& out)
{
    // 数値 1 つなら 4 辺に同じ値。オブジェクトは書いたメンバだけ(ほかは 0。生成するコードと同じ)
    if (j.is_number())
    {
        out = tokotachi::Margins(static_cast<float>(j.as_number()));
        return true;
    }
    if (!j.is_object())
    {
        return false;
    }
    out = tokotachi::Margins();
    return member(j, "left", out.left) && member(j, "top", out.top) && member(j, "right", out.right) &&
           member(j, "bottom", out.bottom);
}

bool from_json(const Json& j, tokotachi::Size& out)
{
    out = tokotachi::Size();
    return j.is_object() && member(j, "width", out.width) && member(j, "height", out.height);
}

bool from_json(const Json& j, tokotachi::Point& out)
{
    out = tokotachi::Point();
    return j.is_object() && member(j, "x", out.x) && member(j, "y", out.y);
}

bool from_json(const Json& j, tokotachi::Rect& out)
{
    out = tokotachi::Rect();
    return j.is_object() && member(j, "x", out.x) && member(j, "y", out.y) && member(j, "width", out.width) &&
           member(j, "height", out.height);
}

bool from_json(const Json& j, tokotachi::Font& out)
{
    out = tokotachi::Font();
    return j.is_object() && member(j, "size", out.size);
}

bool from_json(const Json& j, tokotachi::Color& out)
{
    if (!j.is_object())
    {
        return false;
    }
    out = tokotachi::Color();
    float r = 0, g = 0, b = 0, a = 255;
    if (!member(j, "r", r) || !member(j, "g", g) || !member(j, "b", b) || !member(j, "a", a))
    {
        return false;
    }
    out = tokotachi::Color(static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b),
                           static_cast<std::uint8_t>(a));
    return true;
}

bool from_json(const Json& j, tokotachi::ThemeColor& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (!s.empty() && s[0] == '#')
    {
        if (s.size() != 7 && s.size() != 9)
        {
            return false;
        }
        int v[4] = {0, 0, 0, 255};
        for (std::size_t i = 0; i * 2 + 1 < s.size(); ++i)
        {
            const int hi = hex_digit(s[1 + i * 2]);
            const int lo = hex_digit(s[2 + i * 2]);
            if (hi < 0 || lo < 0)
            {
                return false;
            }
            v[i] = hi * 16 + lo;
        }
        out = tokotachi::ThemeColor(tokotachi::Color(static_cast<std::uint8_t>(v[0]), static_cast<std::uint8_t>(v[1]),
                                                     static_cast<std::uint8_t>(v[2]), static_cast<std::uint8_t>(v[3])));
        return true;
    }
    tokotachi::ColorRole role;
    if (!from_json(j, role))
    {
        return false;
    }
    out = tokotachi::ThemeColor(role);
    return true;
}

bool from_json(const Json& j, tokotachi::Shortcut& out)
{
    if (!j.is_string())
    {
        return false;
    }
    out = tokotachi::Shortcut(j.as_string());
    return true;
}

bool from_json(const Json& j, tokotachi::Placement& out)
{
    if (!j.is_object())
    {
        return false;
    }
    out = tokotachi::Placement();
    if (!member(j, "x", out.x) || !member(j, "y", out.y) || !member(j, "relative_x", out.relative_x) ||
        !member(j, "relative_y", out.relative_y) || !member(j, "width", out.width) ||
        !member(j, "height", out.height) || !member(j, "relative_width", out.relative_width) ||
        !member(j, "relative_height", out.relative_height))
    {
        return false;
    }
    return !j.has("anchor") || from_json(j["anchor"], out.anchor);
}

Json to_json(bool v)
{
    return Json(v);
}

Json to_json(int v)
{
    return Json(static_cast<double>(v));
}

Json to_json(float v)
{
    return Json(static_cast<double>(v));
}

Json to_json(double v)
{
    return Json(v);
}

Json to_json(const std::string& v)
{
    return Json(v);
}

Json to_json(const std::vector<std::string>& v)
{
    Json a = Json::array();
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        a.push(Json(v[i]));
    }
    return a;
}

Json to_json(const std::vector<float>& v)
{
    Json a = Json::array();
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        a.push(to_json(v[i]));
    }
    return a;
}

Json to_json(const tokotachi::Margins& v)
{
    Json o = Json::object();
    o.set("left", to_json(v.left));
    o.set("top", to_json(v.top));
    o.set("right", to_json(v.right));
    o.set("bottom", to_json(v.bottom));
    return o;
}

Json to_json(const tokotachi::Size& v)
{
    Json o = Json::object();
    o.set("width", to_json(v.width));
    o.set("height", to_json(v.height));
    return o;
}

Json to_json(const tokotachi::Point& v)
{
    Json o = Json::object();
    o.set("x", to_json(v.x));
    o.set("y", to_json(v.y));
    return o;
}

Json to_json(const tokotachi::Rect& v)
{
    Json o = Json::object();
    o.set("x", to_json(v.x));
    o.set("y", to_json(v.y));
    o.set("width", to_json(v.width));
    o.set("height", to_json(v.height));
    return o;
}

Json to_json(const tokotachi::Font& v)
{
    Json o = Json::object();
    o.set("size", to_json(v.size));
    return o;
}

Json to_json(const tokotachi::Color& v)
{
    char buf[16];
    if (v.a == 255)
    {
        std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", v.r, v.g, v.b);
    }
    else
    {
        std::snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", v.r, v.g, v.b, v.a);
    }
    return Json(std::string(buf));
}

Json to_json(const tokotachi::ThemeColor& v)
{
    return v.is_role() ? to_json(v.role()) : to_json(v.color());
}

Json to_json(const tokotachi::Shortcut& v)
{
    return Json(v.to_string());
}

}  // namespace ttc
