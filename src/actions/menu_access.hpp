// Menu・MenuBar の内部に触るためのヘルパー(内部)。
#ifndef TOKOTACHI_SRC_ACTIONS_MENU_ACCESS_HPP
#define TOKOTACHI_SRC_ACTIONS_MENU_ACCESS_HPP

#include <tokotachi/actions/menu.hpp>
#include <tokotachi/windows/menu_bar.hpp>
#include "../core/impl.hpp"

namespace tokotachi
{

namespace detail
{
class MenuView;
}

class MenuBar::Impl
{
    TOKOTACHI_IMPL(MenuBar);

    explicit Impl(MenuBar* object) : q(object) {}

    std::vector<Menu*> get_menus() const { return menus_; }

    /// ウィンドウがフォーカスのあるウィジェットより先に渡すキー。使ったら true。
    bool handle_key(const KeyEvent& e, bool down);
    /// ショートカットが一致する Action を実行する(メニューとサブメニューをすべて探す)。実行したら true。
    bool trigger_shortcut(const KeyEvent& e);
    /// キーボードの操作の状態をやめる(ウィンドウを押した・フォーカスを失った)。
    void cancel();

    void open(int i, bool keyboard);
    void open_adjacent(Menu* from, int step);
    /// メニューが閉じた・Esc でメニューバーへ戻る・項目を選んだ。
    void menu_closed(Menu* menu);
    void menu_escaped(Menu* menu);
    void menu_activated();
    void enter_keyboard_mode(int i);
    void leave_keyboard_mode();
    int index_of(const Menu* menu) const;
    int mnemonic_at(Key key) const;

    float bar_height() const;
    Rect title_rect(int i) const;
    int title_at(const Point& p) const;

    std::vector<Menu*> menus_;
    int open_;     ///< 開いているメニュー(-1 なら無し)
    int hover_;    ///< ポインタが乗っている見出し
    int active_;   ///< キーボードで選んでいる見出し(-1 なら無し)
    bool alt_alone_;  ///< Alt を押してから他のキーを押していない
    bool cues_;       ///< ニーモニックの下線を出す
};

class Menu::Impl
{
    TOKOTACHI_IMPL(Menu);

    explicit Impl(Menu* object) : q(object) {}

    /// 項目: action・submenu のどちらか(両方 nullptr なら区切り)。
    struct Entry
    {
        Action* action;
        Menu* submenu;
        bool owned;  ///< action をこのメニューが作った
    };

    void set_title(const std::string& v);
    std::vector<Action*> get_actions() const;
    void changed();

    std::string title_;
    std::vector<Entry> entries_;
    Menu* parent_menu_;
    MenuBar* bar_;  ///< メニューバーのメニューならそのバー
    Popup* popup_;
    detail::MenuView* view_;
    Widget* anchor_;  ///< popup_ を作ったときの基準のウィジェット
    bool cues_;       ///< ニーモニックの下線を出す(キーボードで開いた)
    bool destroying_;

    /// popup_ と view_ を、anchor のウィンドウの上に作る(作ってあれば作り直さない)。
    void ensure_popup(Widget* anchor);
    /// anchor の座標系の矩形 r の下に開く(メニューバー)。keyboard なら最初の項目を強調する。
    void open_below(Widget* anchor, const Rect& r, bool keyboard);
    /// 親のメニューの項目の右に開く(サブメニュー)。
    void open_beside(detail::MenuView* parent_view, const Rect& row);
    /// 項目を選んだ: 一番上のメニューから閉じ、Action を実行する。
    void activate(Action* action);
    Menu* root();
};
namespace detail
{

template <>
struct ImplAccess<Menu>
{
    TOKOTACHI_IMPL_ACCESS(Menu)

    // Action の表示が変わった。
    static void action_changed(Menu* m) { of(m)->changed(); }
    // 破棄する Action の項目を除く(Action は破棄しない)。
    static void forget_action(Menu* m, Action* a);
};

typedef ImplAccess<Menu> MenuAccess;

template <>
struct ImplAccess<MenuBar>
{
    TOKOTACHI_IMPL_ACCESS(MenuBar)

    static bool handle_key(MenuBar* bar, const KeyEvent& e, bool down) { return of(bar)->handle_key(e, down); }
    static bool trigger_shortcut(MenuBar* bar, const KeyEvent& e) { return of(bar)->trigger_shortcut(e); }
    static void cancel(MenuBar* bar) { of(bar)->cancel(); }
};

typedef ImplAccess<MenuBar> MenuBarAccess;

}  // namespace detail
}  // namespace tokotachi

#endif
