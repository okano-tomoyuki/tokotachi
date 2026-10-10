// Menu・MenuBar の内部に触るためのヘルパー(内部)。
#ifndef TOKOTACHI_SRC_ACTIONS_MENU_ACCESS_HPP
#define TOKOTACHI_SRC_ACTIONS_MENU_ACCESS_HPP

#include <tokotachi/actions/menu.hpp>
#include <tokotachi/windows/menu_bar.hpp>

namespace tokotachi
{
namespace detail
{

struct MenuAccess
{
    // Action の表示が変わった。
    static void action_changed(Menu* m) { m->changed(); }
    // 破棄する Action の項目を除く(Action は破棄しない)。
    static void forget_action(Menu* m, Action* a);
};

struct MenuBarAccess
{
    static bool handle_key(MenuBar* bar, const KeyEvent& e, bool down) { return bar->handle_key(e, down); }
    static bool trigger_shortcut(MenuBar* bar, const KeyEvent& e) { return bar->trigger_shortcut(e); }
    static void cancel(MenuBar* bar) { bar->cancel(); }
};

}  // namespace detail
}  // namespace tokotachi

#endif
