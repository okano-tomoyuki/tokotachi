// 標準のウィジェットが今のテーマの色・寸法を読むための関数(内部。docs/adr/0023)。
// 色は描くたびに読む(キャッシュしない)ので、テーマを切り替えたら描き直すだけで変わる。
#ifndef TOKOTACHI_SRC_WIDGETS_THEME_HPP
#define TOKOTACHI_SRC_WIDGETS_THEME_HPP

#include <tokotachi/graphics/theme.hpp>

namespace tokotachi
{
namespace detail
{
// 今のアプリケーションのテーマ(Application が無ければ Theme::light())。
const Theme& current_theme();
}  // namespace detail

namespace theme
{

inline Color color(ColorRole role) { return detail::current_theme()[role]; }

inline Color window() { return color(ColorRole::WINDOW); }
inline Color popup() { return color(ColorRole::POPUP); }
inline Color text() { return color(ColorRole::TEXT); }
inline Color text_disabled() { return color(ColorRole::TEXT_DISABLED); }
inline Color placeholder() { return color(ColorRole::PLACEHOLDER); }
inline Color accent() { return color(ColorRole::ACCENT); }
inline Color accent_hover() { return color(ColorRole::ACCENT_HOVER); }
inline Color accent_pressed() { return color(ColorRole::ACCENT_PRESSED); }
inline Color accent_disabled() { return color(ColorRole::ACCENT_DISABLED); }
inline Color on_accent() { return color(ColorRole::ON_ACCENT); }
inline Color focus_ring() { return color(ColorRole::FOCUS_RING); }
inline Color field() { return color(ColorRole::FIELD); }
inline Color field_disabled() { return color(ColorRole::FIELD_DISABLED); }
inline Color border() { return color(ColorRole::BORDER); }
inline Color selection() { return color(ColorRole::SELECTION); }
inline Color selection_inactive() { return color(ColorRole::SELECTION_INACTIVE); }
inline Color on_selection() { return color(ColorRole::ON_SELECTION); }
inline Color preedit_underline() { return color(ColorRole::PREEDIT_UNDERLINE); }
inline Color scroll_track() { return color(ColorRole::SCROLL_TRACK); }
inline Color scroll_thumb() { return color(ColorRole::SCROLL_THUMB); }
inline Color scroll_thumb_active() { return color(ColorRole::SCROLL_THUMB_ACTIVE); }

inline float radius() { return detail::current_theme().radius; }
inline float field_radius() { return detail::current_theme().field_radius; }
inline float field_padding() { return detail::current_theme().field_padding; }

}  // namespace theme
}  // namespace tokotachi

#endif
