/// @file
/// テーマ: 標準のウィジェットの色と寸法の組み合わせ(docs/adr/0023)。
/// アプリケーションが 1 つのテーマを持ち、すべてのウィンドウに効く。ウィジェットごとに一部を上書きできる(ThemeOverride。docs/adr/0042)。
#ifndef TOKOTACHI_GRAPHICS_THEME_HPP
#define TOKOTACHI_GRAPHICS_THEME_HPP

#include "../core/export.hpp"
#include "../core/geometry.hpp"

namespace tokotachi
{

/// ライト・ダークの選び方。
enum class ColorScheme
{
    SYSTEM,  ///< OS の設定に従う(分からなければライト)
    LIGHT,
    DARK
};

/// テーマの中の色の役割。
enum class ColorRole
{
    WINDOW,               ///< ウィンドウの背景
    POPUP,                ///< ポップアップ(メニュー・一覧)の背景
    TEXT,                 ///< 文字
    TEXT_SECONDARY,       ///< 控えめな文字(説明・見出しの補足)
    TEXT_DISABLED,        ///< 無効なウィジェットの文字
    PLACEHOLDER,          ///< 入力欄の空のときの案内
    ACCENT,               ///< 強調(ボタン・チェック・フォーカスのある枠)
    ACCENT_HOVER,         ///< ポインタが乗った強調
    ACCENT_PRESSED,       ///< 押している強調
    ACCENT_DISABLED,      ///< 無効な強調
    ON_ACCENT,            ///< 強調の上の文字・印
    FOCUS_RING,           ///< フォーカスの輪
    FIELD,                ///< 入力欄・チェックボックスの地
    FIELD_DISABLED,       ///< 無効な入力欄の地
    BORDER,               ///< 枠・区切り
    SELECTION,            ///< 選択の範囲・選んでいる項目
    SELECTION_INACTIVE,   ///< フォーカスの無い入力欄の選択の範囲
    ON_SELECTION,         ///< 選択の上の文字
    PREEDIT_UNDERLINE,    ///< IME の変換中の下線
    SCROLL_TRACK,         ///< スクロールバーの溝
    SCROLL_THUMB,         ///< スクロールバーのつまみ
    SCROLL_THUMB_ACTIVE,  ///< ポインタが乗った・つかんでいるつまみ
    TOOLTIP,              ///< ツールチップの背景
    ON_TOOLTIP            ///< ツールチップの文字
};

/// 色の役割の数。
const int COLOR_ROLE_COUNT = static_cast<int>(ColorRole::ON_TOOLTIP) + 1;

/// 色の指定: テーマの役割か、具体的な色のどちらか。役割で指定した色は、テーマを切り替えると変わる。
/// @code
/// label_note->text_color = ColorRole::TEXT_SECONDARY;   // テーマに従う
/// label_note->text_color = Color::rgb(0xdc2626);        // 固定の色
/// @endcode
/// @ingroup graphics
class TOKOTACHI_API ThemeColor
{
public:
    /// 既定は役割の TEXT。
    ThemeColor() : role_(ColorRole::TEXT), is_role_(true) {}
    /// 具体的な色(テーマを切り替えても変わらない)。
    ThemeColor(const Color& c) : color_(c), role_(ColorRole::TEXT), is_role_(false) {}
    /// テーマの役割(テーマを切り替えると変わる)。
    ThemeColor(ColorRole r) : role_(r), is_role_(true) {}

    /// 役割で指定しているか。
    bool is_role() const { return is_role_; }
    /// 役割(is_role() が true のとき)。
    ColorRole role() const { return role_; }
    /// 具体的な色(is_role() が false のとき)。
    Color color() const { return color_; }
    /// 今のアプリケーションのテーマで、具体的な色にする。
    Color resolve() const;

    /// 指定が同じか(役割どうし・色どうしで比べる)。
    ///@{
    bool operator==(const ThemeColor& o) const
    {
        return is_role_ == o.is_role_ && (is_role_ ? role_ == o.role_ : color_ == o.color_);
    }
    bool operator!=(const ThemeColor& o) const { return !(*this == o); }
    ///@}

private:
    Color color_;
    ColorRole role_;
    bool is_role_;
};

/// テーマ(値の型)。標準のライト・ダークから写して、一部を変えて使える。
/// @code
/// Theme t = Theme::light();
/// t[ColorRole::ACCENT] = Color::rgb(0x059669);
/// app.light_theme = t;
/// @endcode
/// @ingroup graphics
class TOKOTACHI_API Theme
{
public:
    /// ライトのテーマで初期化する。
    Theme();

    /// 標準のライトのテーマ。
    static Theme light();
    /// 標準のダークのテーマ。
    static Theme dark();

    /// 役割の色。
    ///@{
    Color& operator[](ColorRole role) { return colors_[static_cast<int>(role)]; }
    const Color& operator[](ColorRole role) const { return colors_[static_cast<int>(role)]; }
    ///@}

    bool dark_colors;     ///< 暗い背景の色の組み合わせか(OS のウィンドウの枠を合わせるときなどの目安)
    float radius;         ///< ボタンの角の丸み(dp)
    float field_radius;   ///< 入力欄・一覧の角の丸み(dp)
    float field_padding;  ///< 入力欄の左右の内側の余白(dp)

    /// 色と寸法がすべて等しいか。
    ///@{
    bool operator==(const Theme& o) const;
    bool operator!=(const Theme& o) const { return !(*this == o); }
    ///@}

private:
    Color colors_[COLOR_ROLE_COUNT];
};

/// テーマの一部の上書き(値の型。docs/adr/0042)。ウィジェットの theme_override に設定すると、そのウィジェットと子孫に効く。
/// 上書きしていない役割・寸法は、親(ウィンドウはアプリケーション、ポップアップは開いた元のウィジェット)から継ぐ。
/// 色は、ライトとダークで同じにも、別々にもできる(どちらを使うかは、継いだテーマの dark_colors で決める)。
/// @code
/// tokotachi::ThemeOverride danger;
/// danger.set(tokotachi::ColorRole::ACCENT, tokotachi::Color::rgb(0xdc2626));  // ライト・ダークで同じ色
/// danger.set(tokotachi::ColorRole::FIELD, tokotachi::Color::rgb(0xfef2f2), tokotachi::Color::rgb(0x450a0a));  // ライト・ダーク
/// danger.set_radius(0);                                                       // 角を丸めない
/// area_danger->theme_override = danger;
/// @endcode
/// @ingroup graphics
class TOKOTACHI_API ThemeOverride
{
public:
    /// 何も上書きしない。
    ThemeOverride();

    /// 役割の色を上書きする(ライト・ダークで同じ色)。
    ThemeOverride& set(ColorRole role, const Color& color);
    /// 役割の色を上書きする(ライト・ダークで別の色)。
    ThemeOverride& set(ColorRole role, const Color& light, const Color& dark);
    /// 役割の上書きをやめる(継ぐ)。
    ThemeOverride& unset(ColorRole role);
    /// 役割の色を上書きしているか。
    bool has(ColorRole role) const { return has_[static_cast<int>(role)]; }
    /// ライトのときの色(has(role) が true のとき)。
    Color light(ColorRole role) const { return light_[static_cast<int>(role)]; }
    /// ダークのときの色(has(role) が true のとき)。
    Color dark(ColorRole role) const { return dark_[static_cast<int>(role)]; }

    /// 寸法(Theme の radius・field_radius・field_padding。dp)を上書きする。負の値で上書きをやめる。
    ThemeOverride& set_radius(float v);
    /// 入力欄・一覧の角の丸みを上書きする(負の値で上書きをやめる)。
    ThemeOverride& set_field_radius(float v);
    /// 入力欄の左右の内側の余白を上書きする(負の値で上書きをやめる)。
    ThemeOverride& set_field_padding(float v);
    /// 上書きしている寸法(上書きしていなければ負の値)。
    float radius() const { return radius_; }
    /// 上書きしている入力欄・一覧の角の丸み(上書きしていなければ負の値)。
    float field_radius() const { return field_radius_; }
    /// 上書きしている入力欄の余白(上書きしていなければ負の値)。
    float field_padding() const { return field_padding_; }

    /// 何も上書きしていないか。
    bool is_empty() const;
    /// theme に重ねる(theme.dark_colors で、ライト・ダークのどちらの色を使うかを選ぶ)。
    void apply_to(Theme& theme) const;

    /// 上書きの内容が等しいか。
    ///@{
    bool operator==(const ThemeOverride& o) const;
    bool operator!=(const ThemeOverride& o) const { return !(*this == o); }
    ///@}

private:
    Color light_[COLOR_ROLE_COUNT];
    Color dark_[COLOR_ROLE_COUNT];
    bool has_[COLOR_ROLE_COUNT];
    float radius_;
    float field_radius_;
    float field_padding_;
};

}  // namespace tokotachi

#endif
