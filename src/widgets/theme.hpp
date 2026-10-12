// 標準のウィジェットが今のテーマの色・寸法を読むための関数(内部。docs/adr/0023)。
// 色は描くたびに読む(キャッシュしない)ので、テーマを切り替えたら描き直すだけで変わる。
#ifndef TOKOTACHI_SRC_WIDGETS_THEME_HPP
#define TOKOTACHI_SRC_WIDGETS_THEME_HPP

#include <tokotachi/graphics/theme.hpp>

namespace tokotachi
{
class Widget;

namespace detail
{
// アプリケーションのテーマ(Application が無ければ Theme::light())。
const Theme& application_theme();
// 今のテーマ: ThemeScope の中ならそのウィジェットに効いているテーマ、外ならアプリケーションのテーマ(docs/adr/0042)。
const Theme& current_theme();
// ThemeScope が今のテーマを切り替える(nullptr でアプリケーションのテーマ)。前のものを返す。
const Theme* swap_scoped_theme(const Theme* theme);

// w に効いているテーマ(w と祖先の theme_override をアプリケーションのテーマに重ねたもの)を、生きている間だけ今のテーマにする。
// 描く・配置する・イベントを処理するときに、ウィンドウの処理が作る。メインスレッドだけで使う。
class ThemeScope
{
public:
    explicit ThemeScope(const Widget* w);
    ~ThemeScope() { swap_scoped_theme(previous_); }

private:
    ThemeScope(const ThemeScope&);
    ThemeScope& operator=(const ThemeScope&);

    Theme theme_;
    const Theme* previous_;
};
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
