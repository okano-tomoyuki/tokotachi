/// @file
/// tokotachi::MenuBar の宣言。
#ifndef TOKOTACHI_WINDOWS_MENU_BAR_HPP
#define TOKOTACHI_WINDOWS_MENU_BAR_HPP

#include <string>
#include <vector>

#include "../actions/menu.hpp"
#include "../core/export.hpp"
#include "../widgets/widget.hpp"

namespace tokotachi
{

namespace detail
{
struct MenuBarAccess;
}

/// ウィンドウの上端のメニューバー(docs/adr/0027)。
///
/// @image html widget_window_bars.png
///
/// @code
/// tokotachi::MenuBar* menu_bar = new tokotachi::MenuBar();
/// tokotachi::Menu* menu_file = menu_bar->add_menu("ファイル(&F)");
/// menu_file->add_action(action_save);
/// menu_file->add_item("終了(&X)", [&](tokotachi::Object*) { window.close(); });
/// window.menu_bar = menu_bar;   // ウィンドウの上端に置く(レイアウトの領域はその下になる)
/// @endcode
///
/// メニューバーのメニューの Action のショートカットは、ウィンドウ全体で効く。
/// キーボード: Alt+ニーモニックでそのメニューを開く。Alt を押して離す・F10 でメニューバーへ移り、←→ で選び、↓・Enter で開き、
/// Esc で戻る。開いたメニューの中の ←→ は隣のメニューへ移る。Alt を押している間は、ニーモニックの下線を出す。
///
/// @ingroup windows
class TOKOTACHI_API MenuBar : public Widget
{
public:
    explicit MenuBar(Widget* parent = nullptr);
    ~MenuBar();

    /// 最後にメニューを加え、返す(メニューバーが所有する)。
    Menu* add_menu(const std::string& title);
    /// メニューを除いて破棄する。
    void remove_menu(Menu* menu);
    ReadOnlyProperty<std::vector<Menu*> > menus;

protected:
    void paint(Canvas& canvas) override;
    void mouse_down(MouseEvent& e) override;
    void mouse_move(MouseEvent& e) override;
    void mouse_leave() override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    friend class Menu;
    friend class detail::MenuView;
    friend struct detail::MenuBarAccess;

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

}  // namespace tokotachi

#endif
