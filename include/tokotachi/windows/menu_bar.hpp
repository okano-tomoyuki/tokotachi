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
    /// ふつうは nullptr で作り、Window::menu_bar に代入する。
    explicit MenuBar(Widget* parent = nullptr);
    ~MenuBar();

    /// 最後にメニューを加え、返す(メニューバーが所有する)。
    Menu* add_menu(const std::string& title);
    /// メニューを除いて破棄する。
    void remove_menu(Menu* menu);
    /// メニュー(並んだ順)。
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
    template <typename>
    friend struct detail::ImplAccess;
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
