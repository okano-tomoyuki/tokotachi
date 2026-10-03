// メニュー(右クリックのメニューなど。docs/adr/0022)。
//
//   gui::Menu* menu = new gui::Menu(&window);                 // ウィンドウが所有する
//   menu->add_item("コピー", [](gui::Object*) { ... })->shortcut_text = "Ctrl+C";
//   menu->add_separator();
//   gui::Menu* sort = menu->add_submenu("並べ替え");
//   sort->add_item("名前順");
//   edit->on_mouse_down = [=](gui::Object*, gui::MouseEvent& e)
//   {
//       if (e.button == gui::MouseButton::RIGHT) { menu->popup(edit, e.pos); }
//   };
//
// ↑↓ で項目を動かし(区切りと無効な項目は飛ばす)、Enter で選び、→ でサブメニューを開き、← で閉じ、Esc で閉じる。
// 項目を選ぶと、メニューをすべて閉じてから on_click を呼ぶ。メニューは、開くウィジェットのウィンドウより先に破棄すること
// (ウィンドウに所有させればよい)。
#ifndef GUI_MENU_HPP
#define GUI_MENU_HPP

#include <string>
#include <vector>

#include "event.hpp"
#include "geometry.hpp"
#include "property.hpp"

namespace gui
{

class Menu;
class Popup;
class Widget;

namespace detail
{
class MenuView;
}

class MenuItem : public Object
{
public:
    Property<std::string> text;
    // 右に出す説明("Ctrl+C" など)。キーの割り当てはしない。
    Property<std::string> shortcut_text;
    Property<bool> enabled;
    // サブメニュー(無ければ nullptr)。
    ReadOnlyProperty<Menu*> submenu;

    // 選ばれた(メニューをすべて閉じた後に呼ぶ)。
    Property<NotifyHandler> on_click;

private:
    friend class Menu;
    friend class detail::MenuView;

    MenuItem(Menu* owner_menu, const std::string& text, Menu* submenu);

    void set_text(const std::string& v);
    void set_shortcut_text(const std::string& v);
    void set_enabled(const bool& v);
    Menu* get_submenu() const { return submenu_; }
    void changed();

    Menu* menu_;
    std::string text_;
    std::string shortcut_text_;
    bool enabled_;
    Menu* submenu_;
    NotifyHandler on_click_;
};

class Menu : public Object
{
public:
    explicit Menu(Object* owner = nullptr);
    ~Menu();

    MenuItem* add_item(const std::string& text, const NotifyHandler& on_click = NotifyHandler());
    void add_separator();
    // サブメニューを開く項目を加え、サブメニューを返す(このメニューが所有する)。
    Menu* add_submenu(const std::string& text);

    // anchor_widget(ウィンドウに属するウィジェット)の座標系の点 at に開く。
    void popup(Widget* anchor_widget, const Point& at);
    // このメニューと、開いているサブメニューを閉じる。
    void close();
    bool is_open() const;

private:
    friend class MenuItem;
    friend class detail::MenuView;

    // 項目(nullptr は区切り)。
    std::vector<MenuItem*> entries_;
    Menu* parent_menu_;
    Popup* popup_;
    detail::MenuView* view_;
    Widget* anchor_;  // popup_ を作ったときの基準のウィジェット

    // popup_ と view_ を、anchor のウィンドウの上に作る(作ってあれば作り直さない)。
    void ensure_popup(Widget* anchor);
    // 親のメニューの項目の右に開く(サブメニュー)。
    void open_beside(detail::MenuView* parent_view, const Rect& row);
    // 項目を選んだ: 一番上のメニューから閉じ、on_click を呼ぶ。
    void activate(MenuItem* item);
    Menu* root();
};

}  // namespace gui

#endif
