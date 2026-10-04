#include <gui/graphics/canvas.hpp>

#include <algorithm>
#include <cmath>

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

void Path::quad_to(float x1, float y1, float x2, float y2)
{
    // 今の点(無ければ制御点)から、3 次ベジェの制御点を求める。
    float x0 = x1, y0 = y1;
    for (std::size_t i = elements_.size(); i-- > 0;)
    {
        const Element& e = elements_[i];
        if (e.op == Op::CLOSE)
        {
            continue;  // 閉じた図形の始点は追わない(次は move_to から始めるのが普通)
        }
        const int last = e.op == Op::CUBIC_TO ? 4 : 0;
        x0 = e.pts[last];
        y0 = e.pts[last + 1];
        break;
    }
    cubic_to(x0 + 2.0f / 3 * (x1 - x0), y0 + 2.0f / 3 * (y1 - y0), x2 + 2.0f / 3 * (x1 - x2), y2 + 2.0f / 3 * (y1 - y2),
             x2, y2);
}

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

void Path::add_arc(const Point& c, float rx, float ry, float start, float sweep, bool new_figure)
{
    // 90 度以下に分け、それぞれを 3 次ベジェで近似する。
    const float quarter = 1.5707963f;
    const int segments = std::max(1, static_cast<int>(std::ceil(std::fabs(sweep) / quarter - 1e-4f)));
    const float step = sweep / static_cast<float>(segments);
    const float k = 4.0f / 3.0f * std::tan(step / 4);
    float a0 = start;
    const float x0 = c.x + rx * std::cos(a0), y0 = c.y + ry * std::sin(a0);
    if (new_figure || elements_.empty())
    {
        move_to(x0, y0);
    }
    else
    {
        line_to(x0, y0);
    }
    for (int i = 0; i < segments; ++i)
    {
        const float a1 = a0 + step;
        const float c0 = std::cos(a0), s0 = std::sin(a0), c1 = std::cos(a1), s1 = std::sin(a1);
        cubic_to(c.x + rx * (c0 - k * s0), c.y + ry * (s0 + k * c0), c.x + rx * (c1 + k * s1), c.y + ry * (s1 - k * c1),
                 c.x + rx * c1, c.y + ry * s1);
        a0 = a1;
    }
}

void Path::add_polyline(const std::vector<Point>& points, bool close_figure)
{
    for (std::size_t i = 0; i < points.size(); ++i)
    {
        if (i == 0)
        {
            move_to(points[i].x, points[i].y);
        }
        else
        {
            line_to(points[i].x, points[i].y);
        }
    }
    if (close_figure && !points.empty())
    {
        close();
    }
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
