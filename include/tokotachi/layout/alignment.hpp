/// @file
/// 向きとそろえ方(レイアウトのセルの中・Label の文字など、そろえる場面すべてで使う。docs/adr/0017)。
#ifndef TOKOTACHI_LAYOUT_ALIGNMENT_HPP
#define TOKOTACHI_LAYOUT_ALIGNMENT_HPP

namespace tokotachi
{

enum class Orientation
{
    HORIZONTAL,
    VERTICAL
};

/// 始め・中央・終わりに寄せる。FILL は、レイアウトのセルでは最大の大きさまで広げ、広げられないもの(文字など)では START と同じ。
/// 「左・右」ではなく「始め・終わり」とするのは、右から左へ書く言語に対応したときに意味が変わらないようにするため。
enum class Align
{
    START,
    CENTER,
    END,
    FILL
};

}  // namespace tokotachi

#endif
