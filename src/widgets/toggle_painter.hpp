// CheckBox・RadioButton の共通の描画(内部): 左の印と、その右の文字。
#ifndef TOKOTACHI_SRC_WIDGETS_TOGGLE_PAINTER_HPP
#define TOKOTACHI_SRC_WIDGETS_TOGGLE_PAINTER_HPP

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/graphics/text_layout.hpp>
#include <tokotachi/widgets/widget.hpp>

#include "theme.hpp"

namespace tokotachi
{
namespace detail
{

const float TOGGLE_INDICATOR = 16;  // 印の大きさ(dp)
const float TOGGLE_GAP = 8;         // 印と文字の間

inline Rect toggle_indicator_rect(const Rect& r)
{
    return Rect(0, std::floor((r.height - TOGGLE_INDICATOR) / 2), TOGGLE_INDICATOR, TOGGLE_INDICATOR);
}

inline Size toggle_preferred_size(const TextLayout& l)
{
    return Size(TOGGLE_INDICATOR + TOGGLE_GAP + std::ceil(l.width()), std::max(TOGGLE_INDICATOR, std::ceil(l.height())) + 8);
}

// 文字を描き、その位置を返す(ニーモニックの下線を引くため)。
inline Point paint_toggle_text(Canvas& canvas, const TextLayout& l, const Rect& r, bool focused, bool enabled)
{
    const float tx = TOGGLE_INDICATOR + TOGGLE_GAP;
    const Point at(tx, (r.height - l.height()) / 2);
    canvas.draw_text(l, at, enabled ? theme::text() : theme::text_disabled());
    if (focused)
    {
        // フォーカスは文字の下線で示す。
        canvas.fill_rect(Rect(tx, (r.height + l.height()) / 2, l.width(), 1), theme::focus_ring());
    }
    return at;
}

}  // namespace detail
}  // namespace tokotachi

#endif
