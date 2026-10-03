// 標準のウィジェットの見た目の定数(内部)。テーマの仕組みを作るまでの仮置き。
#ifndef GUI_SRC_WIDGETS_THEME_HPP
#define GUI_SRC_WIDGETS_THEME_HPP

#include <gui/geometry.hpp>

namespace gui
{
namespace theme
{

inline Color text() { return Color::rgb(0x111827); }
inline Color text_disabled() { return Color::rgb(0x9ca3af); }
inline Color placeholder() { return Color::rgb(0x9ca3af); }
inline Color accent() { return Color::rgb(0x2563eb); }
inline Color accent_hover() { return Color::rgb(0x1d4ed8); }
inline Color accent_pressed() { return Color::rgb(0x1e40af); }
inline Color accent_disabled() { return Color::rgb(0xbfdbfe); }
inline Color on_accent() { return Color::rgb(0xffffff); }
inline Color focus_ring() { return Color::rgb(0x93c5fd); }
inline Color field() { return Color::rgb(0xffffff); }
inline Color field_disabled() { return Color::rgb(0xf3f4f6); }
inline Color border() { return Color::rgb(0xc8ccd2); }
inline Color selection() { return Color::rgb(0x2563eb); }
inline Color selection_inactive() { return Color::rgb(0xd1d5db); }
inline Color on_selection() { return Color::rgb(0xffffff); }
inline Color preedit_underline() { return Color::rgb(0x334155); }
inline Color scroll_track() { return Color::rgb(0xeef0f3); }
inline Color scroll_thumb() { return Color::rgb(0xb8bec7); }
inline Color scroll_thumb_active() { return Color::rgb(0x8b93a0); }

const float RADIUS = 6;
const float FIELD_RADIUS = 4;
const float FIELD_PADDING_X = 8;

}  // namespace theme
}  // namespace gui

#endif
