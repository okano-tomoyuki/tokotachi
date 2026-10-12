/// @file
/// tokotachi::Menu の宣言。
#ifndef TOKOTACHI_ACTIONS_MENU_HPP
#define TOKOTACHI_ACTIONS_MENU_HPP

#include <string>
#include <vector>

#include "../core/export.hpp"
#include "action.hpp"
#include "../core/geometry.hpp"

namespace tokotachi
{

class MenuBar;
class Popup;
class Widget;

/// メニュー(右クリックのメニュー・メニューバーのメニュー。docs/adr/0022、0027)。項目は Action で表す。
///
/// @code
/// tokotachi::Menu* menu_edit = new tokotachi::Menu(&window);                 // ウィンドウが所有する
/// menu_edit->add_item("コピー(&C)", [](tokotachi::Object*) { ... })->shortcut = "Ctrl+C";
/// menu_edit->add_action(action_save);                                   // 既にある Action を入れる
/// menu_edit->add_separator();
/// tokotachi::Menu* menu_sort = menu_edit->add_submenu("並べ替え(&S)");
/// menu_sort->add_item("名前順");
/// text_edit_memo->on_mouse_down = [=](tokotachi::Object*, tokotachi::MouseEvent& e)
/// {
///     if (e.button == tokotachi::MouseButton::RIGHT) { menu_edit->popup(text_edit_memo, e.pos); }
/// };
/// @endcode
///
/// ↑↓ で項目を動かし(区切りと無効な項目は飛ばす)、Enter で選び、→ でサブメニューを開き、← で閉じ、Esc で閉じる。
/// 項目のニーモニック(text の & の次の文字)のキーを押すと、その項目を選ぶ。
/// 項目を選ぶと、メニューをすべて閉じてから Action を実行する。メニューは、開くウィジェットのウィンドウより先に破棄すること
/// (ウィンドウに所有させればよい)。
///
/// @ingroup actions
class TOKOTACHI_API Menu : public Object
{
public:
    explicit Menu(Object* owner = nullptr, const std::string& title = std::string());
    ~Menu();

    /// メニューバー・親のメニューの項目に出す名前(& でニーモニック)。
    Property<std::string> title;

    /// text の Action を作って最後に加え、返す(このメニューが所有する)。
    Action* add_item(const std::string& text, const NotifyHandler& on_trigger = NotifyHandler());
    /// 既にある Action を最後に加える(所有しない。1 つの Action を複数のメニューに入れてよい)。
    void add_action(Action* action);
    /// Action の項目を除く(このメニューが作った Action なら破棄する)。
    void remove_action(Action* action);
    void add_separator();
    /// サブメニューを開く項目を加え、サブメニューを返す(このメニューが所有する)。
    Menu* add_submenu(const std::string& title);
    /// 項目の Action(並んだ順。区切り・サブメニューを除く)。
    ReadOnlyProperty<std::vector<Action*> > actions;

    /// anchor_widget(ウィンドウに属するウィジェット)の座標系の点 at に開く。
    void popup(Widget* anchor_widget, const Point& at);
    /// このメニューと、開いているサブメニューを閉じる。
    void close();
    bool is_open() const;

private:
    friend class MenuBar;
    template <typename>
    friend struct detail::ImplAccess;
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
