/// @file
/// tokotachi::ControlState の宣言。
#ifndef TOKOTACHI_WIDGETS_CONTROL_STATE_HPP
#define TOKOTACHI_WIDGETS_CONTROL_STATE_HPP

#include "../core/export.hpp"

namespace tokotachi
{

/// 部品の状態(標準の見た目で描くときに使う。tokotachi::draw、ButtonBase::state。docs/adr/0058)。
/// @ingroup widgets_base
struct TOKOTACHI_API ControlState
{
    bool enabled;  ///< 有効か(既定 true。親が無効なら無効)
    bool hovered;  ///< マウスが上にあるか
    bool pressed;  ///< 押されているか(押したまま上にある)
    bool focused;  ///< キーボードのフォーカスがあるか
    bool checked;  ///< チェックが付いているか(チェック・ラジオの印)

    ControlState() : enabled(true), hovered(false), pressed(false), focused(false), checked(false) {}
};

}  // namespace tokotachi

#endif
