// アイコンの線画(内部。docs/adr/0054)。データは tools/icons/generate.mjs が生成する icon_data.cpp。
#ifndef TOKOTACHI_SRC_GRAPHICS_ICON_DATA_HPP
#define TOKOTACHI_SRC_GRAPHICS_ICON_DATA_HPP

#include <tokotachi/core/geometry.hpp>
#include <tokotachi/graphics/canvas.hpp>

namespace tokotachi
{
namespace detail
{

/// 1 つのアイコン: 命令(0 移動・1 直線・2 3 次ベジェ・3 閉じる)と、その点(24 × 24 の格子)の並び。
struct IconShape
{
    const float* data;
    int size;
};

extern const IconShape ICON_SHAPES[];
extern const int ICON_COUNT;

/// 格子の大きさと、線の太さ(格子の単位。Lucide の stroke-width)。
const float ICON_GRID = 24;
const float ICON_STROKE = 2;

/// index のアイコンを dst に収めたパス。
Path icon_path(int index, const Rect& dst);

/// index のアイコンを dst に color で描く(線の太さは大きさに合わせる。端と角は丸い)。
void draw_icon(Canvas& canvas, int index, const Rect& dst, const Color& color);

}  // namespace detail
}  // namespace tokotachi

#endif
