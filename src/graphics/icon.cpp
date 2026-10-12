// アイコンの線画を描く(docs/adr/0054)。
#include <algorithm>

#include "icon_data.hpp"

namespace tokotachi
{
namespace detail
{

Path icon_path(int index, const Rect& dst)
{
    Path path;
    if (index < 0 || index >= ICON_COUNT)
    {
        return path;
    }
    const IconShape& shape = ICON_SHAPES[index];
    // 格子(24 × 24)を dst に。縦横の比が違えば、短い方に合わせて中央に置く
    const float k = std::min(dst.width, dst.height) / ICON_GRID;
    const float ox = dst.x + (dst.width - ICON_GRID * k) / 2;
    const float oy = dst.y + (dst.height - ICON_GRID * k) / 2;
    const float* p = shape.data;
    const float* end = shape.data + shape.size;
    while (p < end)
    {
        const int op = static_cast<int>(*p++);
        switch (op)
        {
        case 0:
            path.move_to(ox + p[0] * k, oy + p[1] * k);
            p += 2;
            break;
        case 1:
            path.line_to(ox + p[0] * k, oy + p[1] * k);
            p += 2;
            break;
        case 2:
            path.cubic_to(ox + p[0] * k, oy + p[1] * k, ox + p[2] * k, oy + p[3] * k, ox + p[4] * k, oy + p[5] * k);
            p += 6;
            break;
        default:
            path.close();
            break;
        }
    }
    return path;
}

void draw_icon(Canvas& canvas, int index, const Rect& dst, const Color& color)
{
    if (dst.width <= 0 || dst.height <= 0 || color.a == 0)
    {
        return;
    }
    const float k = std::min(dst.width, dst.height) / ICON_GRID;
    canvas.stroke_path(icon_path(index, dst), StrokeStyle(ICON_STROKE * k, LineCap::ROUND, LineJoin::ROUND), Paint(color));
}

}  // namespace detail
}  // namespace tokotachi
