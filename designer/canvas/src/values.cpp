#include "values.hpp"

#include <cstdio>

namespace ttc
{

namespace
{

/// オブジェクトのメンバの値(無ければ out のまま)。型が違えば false。
bool member(const Json& j, const char* name, float& out)
{
    if (!j.has(name))
    {
        return true;
    }
    return from_json(j[name], out);
}

bool member(const Json& j, const char* name, bool& out)
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
    return j.is_object() && member(j, "size", out.size) && member(j, "bold", out.bold) &&
           member(j, "italic", out.italic);
}

bool from_json(const Json& j, tokotachi::Color& out)
{
    // 「#RRGGBB」・「#RRGGBBAA」か、メンバ(r・g・b・a)のオブジェクト
    if (j.is_string())
    {
        tokotachi::ThemeColor c;
        if (j.as_string().empty() || j.as_string()[0] != '#' || !from_json(j, c))
        {
            return false;
        }
        out = c.color();
        return true;
    }
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

namespace
{
// "#RRGGBB"・"#RRGGBBAA"(役割の名前は受け付けない)。
bool hex_color(const Json& j, tokotachi::Color& out)
{
    tokotachi::ThemeColor c;
    if (!j.is_string() || j.as_string().empty() || j.as_string()[0] != '#' || !from_json(j, c))
    {
        return false;
    }
    out = c.color();
    return true;
}
}  // namespace

bool from_json(const Json& j, tokotachi::ThemeOverride& out)
{
    if (!j.is_object())
    {
        return false;
    }
    tokotachi::ThemeOverride t;
    const std::vector<std::pair<std::string, Json> >& members = j.members();
    for (std::size_t i = 0; i < members.size(); ++i)
    {
        const std::string& key = members[i].first;
        const Json& v = members[i].second;
        float f = 0;
        if (key == "colors")
        {
            if (!v.is_object())
            {
                return false;
            }
            for (std::size_t k = 0; k < v.members().size(); ++k)
            {
                tokotachi::ColorRole role;
                tokotachi::Color light, dark;
                const Json& c = v.members()[k].second;
                if (!from_json(Json(v.members()[k].first), role))
                {
                    return false;
                }
                if (hex_color(c, light))
                {
                    t.set(role, light);
                }
                else if (c.is_object() && hex_color(c["light"], light) && hex_color(c["dark"], dark))
                {
                    t.set(role, light, dark);
                }
                else
                {
                    return false;
                }
            }
        }
        else if (key == "radius" && from_json(v, f))
        {
            t.set_radius(f);
        }
        else if (key == "field_radius" && from_json(v, f))
        {
            t.set_field_radius(f);
        }
        else if (key == "field_padding" && from_json(v, f))
        {
            t.set_field_padding(f);
        }
        else
        {
            return false;
        }
    }
    out = t;
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
    o.set("bold", to_json(v.bold));
    o.set("italic", to_json(v.italic));
    return o;
}

bool from_json(const Json& j, tokotachi::Date& out)
{
    // 「2026-10-11」(空の文字は空の日付)
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (s.empty())
    {
        out = tokotachi::Date();
        return true;
    }
    int y = 0, m = 0, d = 0;
    if (std::sscanf(s.c_str(), "%d-%d-%d", &y, &m, &d) != 3 || !tokotachi::Date(y, m, d).is_valid())
    {
        return false;
    }
    out = tokotachi::Date(y, m, d);
    return true;
}

Json to_json(const tokotachi::Date& v)
{
    if (v.is_null())
    {
        return Json(std::string());
    }
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", v.year, v.month, v.day);
    return Json(std::string(buf));
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

Json to_json(const tokotachi::ThemeOverride& v)
{
    Json out = Json::object();
    Json colors = Json::object();
    bool any = false;
    for (int i = 0; i < tokotachi::COLOR_ROLE_COUNT; ++i)
    {
        const tokotachi::ColorRole role = static_cast<tokotachi::ColorRole>(i);
        if (!v.has(role))
        {
            continue;
        }
        Json c = to_json(v.light(role));
        if (v.light(role) != v.dark(role))
        {
            c = Json::object();
            c.set("light", to_json(v.light(role)));
            c.set("dark", to_json(v.dark(role)));
        }
        colors.set(to_json(role).as_string(), c);
        any = true;
    }
    if (any)
    {
        out.set("colors", colors);
    }
    if (v.radius() >= 0)
    {
        out.set("radius", to_json(v.radius()));
    }
    if (v.field_radius() >= 0)
    {
        out.set("field_radius", to_json(v.field_radius()));
    }
    if (v.field_padding() >= 0)
    {
        out.set("field_padding", to_json(v.field_padding()));
    }
    return out;
}

Json to_json(const tokotachi::Shortcut& v)
{
    return Json(v.to_string());
}

// ---- 画像 ----

namespace
{

std::map<std::string, tokotachi::Image>& image_store()
{
    static std::map<std::string, tokotachi::Image> images;
    return images;
}

int base64_digit(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c - 'A';
    }
    if (c >= 'a' && c <= 'z')
    {
        return c - 'a' + 26;
    }
    if (c >= '0' && c <= '9')
    {
        return c - '0' + 52;
    }
    if (c == '+')
    {
        return 62;
    }
    if (c == '/')
    {
        return 63;
    }
    return -1;
}

}  // namespace

bool from_json(const Json& j, tokotachi::Image& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::map<std::string, tokotachi::Image>& images = image_store();
    std::map<std::string, tokotachi::Image>::const_iterator it = images.find(j.as_string());
    out = it == images.end() ? tokotachi::Image() : it->second;
    return true;
}

Json to_json(const tokotachi::Image&)
{
    return Json();
}

void set_images(const std::map<std::string, tokotachi::Image>& images)
{
    image_store() = images;
}

bool decode_base64(const std::string& text, std::vector<unsigned char>& out)
{
    out.clear();
    unsigned int bits = 0;
    int count = 0;
    for (std::size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == '=')
        {
            break;
        }
        const int d = base64_digit(text[i]);
        if (d < 0)
        {
            return false;
        }
        bits = (bits << 6) | static_cast<unsigned int>(d);
        count += 6;
        if (count >= 8)
        {
            count -= 8;
            out.push_back(static_cast<unsigned char>((bits >> count) & 0xFF));
        }
    }
    return true;
}

}  // namespace ttc
