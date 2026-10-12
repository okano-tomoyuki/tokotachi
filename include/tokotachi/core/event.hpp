/// @file
/// イベントの値とハンドラの型(docs/adr/0013)。位置はそのウィジェットの左上を原点とする dp。
#ifndef TOKOTACHI_CORE_EVENT_HPP
#define TOKOTACHI_CORE_EVENT_HPP

#include <cstdint>
#include <functional>
#include <string>

#include "geometry.hpp"

namespace tokotachi
{

class Canvas;
class Object;

enum class MouseButton
{
    NONE,
    LEFT,
    MIDDLE,
    RIGHT
};

/// 修飾キー(ビットの組み合わせ)。
typedef unsigned Modifiers;
const Modifiers MODIFIER_NONE = 0;
const Modifiers MODIFIER_SHIFT = 1u << 0;
const Modifiers MODIFIER_CONTROL = 1u << 1;
const Modifiers MODIFIER_ALT = 1u << 2;
const Modifiers MODIFIER_META = 1u << 3;  ///< Windows キー / Super

/// マクロとぶつかる名前は別名にする(DELETE は winnt.h のマクロ。docs/adr/0013)。
enum class Key
{
    UNKNOWN,
    BACKSPACE, TAB, RETURN, ESCAPE, SPACE, DEL, INSERT,
    LEFT, RIGHT, UP, DOWN, HOME, END, PAGE_UP, PAGE_DOWN,
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    DIGIT_0, DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4, DIGIT_5, DIGIT_6, DIGIT_7, DIGIT_8, DIGIT_9,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    /// 修飾キーそのもの(左右を区別しない)
    SHIFT, CONTROL, ALT, META
};

/// accepted が false のまま返ったイベントは親のウィジェットへ送られる(マウスのボタン・キー)。
/// @ingroup core_events
struct MouseEvent
{
    Point pos;
    MouseButton button;  ///< 押された・離されたボタン。移動のイベントでは NONE
    Modifiers modifiers;
    int clicks;  ///< 連続して押した回数(ダブルクリックなら 2)
    bool accepted;

    MouseEvent(const Point& pos_, MouseButton button_, Modifiers modifiers_, int clicks_)
        : pos(pos_), button(button_), modifiers(modifiers_), clicks(clicks_), accepted(true)
    {
    }

    bool has_modifier(Modifiers m) const { return (modifiers & m) == m; }
};

/// キーの押下・離し(Widget::key_down・key_up に渡る)。
///
/// @ingroup core_events
struct KeyEvent
{
    Key key;
    Modifiers modifiers;
    bool repeat;
    std::uint32_t native_key;  ///< SDL のキーコード(Key に無いキーを扱うため)
    bool accepted;

    KeyEvent(Key key_, Modifiers modifiers_, bool repeat_, std::uint32_t native_key_)
        : key(key_), modifiers(modifiers_), repeat(repeat_), native_key(native_key_), accepted(true)
    {
    }

    bool has_modifier(Modifiers m) const { return (modifiers & m) == m; }
};

/// マウスのホイール。delta は回した量(目盛りの数。滑らかに回るホイールでは小数)。y が正なら奥へ回した(上へスクロールする)、
/// x が正なら右へ。OS の「逆向きのスクロール」の設定は反映済み。
/// @ingroup core_events
struct WheelEvent
{
    Point pos;
    Point delta;
    Modifiers modifiers;
    bool accepted;

    WheelEvent(const Point& pos_, const Point& delta_, Modifiers modifiers_)
        : pos(pos_), delta(delta_), modifiers(modifiers_), accepted(true)
    {
    }

    bool has_modifier(Modifiers m) const { return (modifiers & m) == m; }
};

/// IME の確定やキーボードからの文字の入力。
/// @ingroup core_events
struct TextInputEvent
{
    std::string text;
    bool accepted;

    explicit TextInputEvent(const std::string& text_) : text(text_), accepted(true) {}
};

/// IME の変換中の文字列(docs/adr/0009)。start・length は変換対象の文節(コードポイントの単位。無ければ -1)。
/// @ingroup core_events
struct TextEditingEvent
{
    std::string text;
    int start;
    int length;
    bool accepted;

    TextEditingEvent(const std::string& text_, int start_, int length_)
        : text(text_), start(start_), length(length_), accepted(true)
    {
    }
};

/// ハンドラの型。第 1 引数は常にイベントの発生元(Widget・Timer 等。docs/adr/0013)。
/// 発生元の機能を使うときは dynamic_cast で型を確かめる。
typedef std::function<void(Object* sender)> NotifyHandler;
typedef std::function<void(Object* sender, MouseEvent& e)> MouseHandler;
typedef std::function<void(Object* sender, KeyEvent& e)> KeyHandler;
typedef std::function<void(Object* sender, WheelEvent& e)> WheelHandler;
typedef std::function<void(Object* sender, Canvas& canvas)> PaintHandler;
/// can_close を false にすると閉じない。
typedef std::function<void(Object* sender, bool& can_close)> CloseHandler;

}  // namespace tokotachi

#endif
