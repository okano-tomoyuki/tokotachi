/// @file
/// 標準の見た目を描く関数(tokotachi::draw。docs/adr/0058)。
#ifndef TOKOTACHI_WIDGETS_DRAW_HPP
#define TOKOTACHI_WIDGETS_DRAW_HPP

#include "../core/export.hpp"
#include "../core/geometry.hpp"
#include "../graphics/canvas.hpp"
#include "button.hpp"
#include "control_state.hpp"

namespace tokotachi
{

/// 組み込みの部品と同じ見た目を描く関数(docs/adr/0058)。自作の部品(ButtonBase・RangeBase・Widget の派生)の paint で使う。
///
/// 今のテーマ(部品ごとのテーマの上書きを含む。paint の中では、その部品に効いているテーマ)の色・角の丸みで描く。
/// 組み込みの部品も、これらの関数で自分を描く。
/// @code
/// class BadgeButton : public tokotachi::ButtonBase
/// {
/// public:
///     explicit BadgeButton(tokotachi::Widget* parent) : tokotachi::ButtonBase(parent, "受信") {}
///
/// protected:
///     void paint(tokotachi::Canvas& canvas) override
///     {
///         const tokotachi::Rect r = local_rect;
///         const tokotachi::ControlState s = state();
///         tokotachi::draw::button_face(canvas, r, s);                       // 標準のボタンの面
///         canvas.draw_text(text_layout(), tokotachi::Point(12, (r.height - text_layout().height()) / 2),
///                          tokotachi::draw::button_text_color(s));
///         canvas.fill_round_rect(tokotachi::Rect(r.width - 14, 4, 10, 10), 5, tokotachi::Color(220, 40, 40));  // 足したい印
///         if (s.focused)
///         {
///             tokotachi::draw::focus_ring(canvas, r);
///         }
///     }
/// };
/// @endcode
///
/// @ingroup widgets_drawing
namespace draw
{

/// ボタンの面(塗りと、SECONDARY なら枠)。state の有効・ホバー・押下で色を変える。フォーカスの枠は含まない(focus_ring)。
TOKOTACHI_API void button_face(Canvas& canvas, const Rect& rect, const ControlState& state,
                               ButtonStyle style = ButtonStyle::PRIMARY);

/// ボタンの面の上に描く文字・画像の色。
TOKOTACHI_API Color button_text_color(const ControlState& state, ButtonStyle style = ButtonStyle::PRIMARY);

/// チェックの印(CheckBox の左の四角)。rect は印の四角(組み込みの部品は 16dp)。state の checked で印を付ける。
TOKOTACHI_API void check_mark(Canvas& canvas, const Rect& rect, const ControlState& state);

/// ラジオの印(RadioButton の左の丸)。rect は印の四角(組み込みの部品は 16dp)。state の checked で中の点を付ける。
TOKOTACHI_API void radio_mark(Canvas& canvas, const Rect& rect, const ControlState& state);

/// キーボードのフォーカスの枠。rect(部品の外形)の少し内側に、角の丸みを合わせて引く。
TOKOTACHI_API void focus_ring(Canvas& canvas, const Rect& rect);

/// 入力欄の枠(地の塗りと枠線)。フォーカスがあれば、太い強調の色の枠にする(LineEdit と同じ)。
TOKOTACHI_API void field_frame(Canvas& canvas, const Rect& rect, const ControlState& state);

}  // namespace draw
}  // namespace tokotachi

#endif
