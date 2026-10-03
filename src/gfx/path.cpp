#include <gui/canvas.hpp>

#include <algorithm>

namespace gui
{

namespace
{

// 4 分の 1 の円弧を 3 次ベジェで近似するときの制御点の比。
const float KAPPA = 0.5522847498f;

Path::Element element(Path::Op op, float a = 0, float b = 0, float c = 0, float d = 0, float e = 0, float f = 0)
{
    Path::Element el;
    el.op = op;
    el.pts[0] = a;
    el.pts[1] = b;
    el.pts[2] = c;
    el.pts[3] = d;
    el.pts[4] = e;
    el.pts[5] = f;
    return el;
}

}  // namespace

void Path::move_to(float x, float y) { elements_.push_back(element(Op::MOVE_TO, x, y)); }

void Path::line_to(float x, float y) { elements_.push_back(element(Op::LINE_TO, x, y)); }

void Path::cubic_to(float x1, float y1, float x2, float y2, float x3, float y3)
{
    elements_.push_back(element(Op::CUBIC_TO, x1, y1, x2, y2, x3, y3));
}

void Path::close() { elements_.push_back(element(Op::CLOSE)); }

void Path::add_rect(const Rect& r)
{
    move_to(r.x, r.y);
    line_to(r.right(), r.y);
    line_to(r.right(), r.bottom());
    line_to(r.x, r.bottom());
    close();
}

void Path::add_round_rect(const Rect& r, float radius)
{
    const float rad = std::min(radius, std::min(r.width, r.height) / 2);
    if (rad <= 0)
    {
        add_rect(r);
        return;
    }
    const float k = rad * (1 - KAPPA);
    const float l = r.x, t = r.y, rt = r.right(), b = r.bottom();
    move_to(l + rad, t);
    line_to(rt - rad, t);
    cubic_to(rt - k, t, rt, t + k, rt, t + rad);
    line_to(rt, b - rad);
    cubic_to(rt, b - k, rt - k, b, rt - rad, b);
    line_to(l + rad, b);
    cubic_to(l + k, b, l, b - k, l, b - rad);
    line_to(l, t + rad);
    cubic_to(l, t + k, l + k, t, l + rad, t);
    close();
}

void Path::add_ellipse(const Point& c, float rx, float ry)
{
    const float kx = rx * KAPPA, ky = ry * KAPPA;
    move_to(c.x + rx, c.y);
    cubic_to(c.x + rx, c.y + ky, c.x + kx, c.y + ry, c.x, c.y + ry);
    cubic_to(c.x - kx, c.y + ry, c.x - rx, c.y + ky, c.x - rx, c.y);
    cubic_to(c.x - rx, c.y - ky, c.x - kx, c.y - ry, c.x, c.y - ry);
    cubic_to(c.x + kx, c.y - ry, c.x + rx, c.y - ky, c.x + rx, c.y);
    close();
}

Paint Paint::linear(const Point& from, const Point& to, const std::vector<GradientStop>& stops)
{
    Paint p;
    p.type = Type::LINEAR;
    p.p0 = from;
    p.p1 = to;
    p.stops = stops;
    return p;
}

Paint Paint::radial(const Point& center, float radius, const Point& focal, const std::vector<GradientStop>& stops)
{
    Paint p;
    p.type = Type::RADIAL;
    p.p0 = center;
    p.p1 = focal;
    p.radius = radius;
    p.stops = stops;
    return p;
}

}  // namespace gui
