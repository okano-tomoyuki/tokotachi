// テーマ: 標準のウィジェットの色と寸法の組み合わせ(docs/adr/0023)。
// アプリケーションが 1 つのテーマを持ち、すべてのウィンドウに効く。部分的に変えるときは、ウィジェットの色のプロパティで変える。
#ifndef GUI_THEME_HPP
#define GUI_THEME_HPP

#include "../core/geometry.hpp"

namespace gui
{

// ライト・ダークの選び方。
enum class ColorScheme
{
    SYSTEM,  // OS の設定に従う(分からなければライト)
    LIGHT,
    DARK
};

// テーマの中の色の役割。
enum class ColorRole
{
    WINDOW,               // ウィンドウの背景
    POPUP,                // ポップアップ(メニュー・一覧)の背景
    TEXT,                 // 文字
    TEXT_SECONDARY,       // 控えめな文字(説明・見出しの補足)
    TEXT_DISABLED,        // 無効なウィジェットの文字
    PLACEHOLDER,          // 入力欄の空のときの案内
    ACCENT,               // 強調(ボタン・チェック・フォーカスのある枠)
    ACCENT_HOVER,         // ポインタが乗った強調
    ACCENT_PRESSED,       // 押している強調
    ACCENT_DISABLED,      // 無効な強調
    ON_ACCENT,            // 強調の上の文字・印
    FOCUS_RING,           // フォーカスの輪
    FIELD,                // 入力欄・チェックボックスの地
    FIELD_DISABLED,       // 無効な入力欄の地
    BORDER,               // 枠・区切り
    SELECTION,            // 選択の範囲・選んでいる項目
    SELECTION_INACTIVE,   // フォーカスの無い入力欄の選択の範囲
    ON_SELECTION,         // 選択の上の文字
    PREEDIT_UNDERLINE,    // IME の変換中の下線
    SCROLL_TRACK,         // スクロールバーの溝
    SCROLL_THUMB,         // スクロールバーのつまみ
    SCROLL_THUMB_ACTIVE,  // ポインタが乗った・つかんでいるつまみ
    TOOLTIP,              // ツールチップの背景
    ON_TOOLTIP            // ツールチップの文字
};

const int COLOR_ROLE_COUNT = static_cast<int>(ColorRole::ON_TOOLTIP) + 1;

// 色の指定: テーマの役割か、具体的な色のどちらか。役割で指定した色は、テーマを切り替えると変わる。
//   label->color = ColorRole::TEXT_SECONDARY;   // テーマに従う
//   label->color = Color::rgb(0xdc2626);        // 固定の色
class ThemeColor
{
public:
    // 既定は役割の TEXT。
    ThemeColor() : role_(ColorRole::TEXT), is_role_(true) {}
    ThemeColor(const Color& c) : color_(c), role_(ColorRole::TEXT), is_role_(false) {}
    ThemeColor(ColorRole r) : role_(r), is_role_(true) {}

    bool is_role() const { return is_role_; }
    // 役割(is_role() が true のとき)。
    ColorRole role() const { return role_; }
    // 具体的な色(is_role() が false のとき)。
    Color color() const { return color_; }
    // 今のアプリケーションのテーマで、具体的な色にする。
    Color resolve() const;

    bool operator==(const ThemeColor& o) const
    {
        return is_role_ == o.is_role_ && (is_role_ ? role_ == o.role_ : color_ == o.color_);
    }
    bool operator!=(const ThemeColor& o) const { return !(*this == o); }

private:
    Color color_;
    ColorRole role_;
    bool is_role_;
};

// テーマ(値の型)。標準のライト・ダークから写して、一部を変えて使える。
//   Theme t = Theme::light();
//   t[ColorRole::ACCENT] = Color::rgb(0x059669);
//   app.light_theme = t;
class Theme
{
public:
    // ライトのテーマで初期化する。
    Theme();

    static Theme light();
    static Theme dark();

    Color& operator[](ColorRole role) { return colors_[static_cast<int>(role)]; }
    const Color& operator[](ColorRole role) const { return colors_[static_cast<int>(role)]; }

    bool dark_colors;     // 暗い背景の色の組み合わせか(OS のウィンドウの枠を合わせるときなどの目安)
    float radius;         // ボタンの角の丸み(dp)
    float field_radius;   // 入力欄・一覧の角の丸み(dp)
    float field_padding;  // 入力欄の左右の内側の余白(dp)

    bool operator==(const Theme& o) const;
    bool operator!=(const Theme& o) const { return !(*this == o); }

private:
    Color colors_[COLOR_ROLE_COUNT];
};

}  // namespace gui

#endif
