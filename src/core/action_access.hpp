// Action の内部の状態に触るためのヘルパー(内部)。
#ifndef GUI_SRC_CORE_ACTION_ACCESS_HPP
#define GUI_SRC_CORE_ACTION_ACCESS_HPP

#include <algorithm>
#include <vector>

#include <gui/action.hpp>

namespace gui
{
namespace detail
{

struct ActionAccess
{
    template <typename T>
    static void add_unique(std::vector<T*>& list, T* v)
    {
        if (std::find(list.begin(), list.end(), v) == list.end())
        {
            list.push_back(v);
        }
    }

    template <typename T>
    static void remove(std::vector<T*>& list, T* v)
    {
        list.erase(std::remove(list.begin(), list.end(), v), list.end());
    }

    static void attach_menu(Action* a, Menu* m) { add_unique(a->menus_, m); }
    static void detach_menu(Action* a, Menu* m) { remove(a->menus_, m); }
    static void attach_widget(Action* a, Widget* w) { add_unique(a->widgets_, w); }
    static void detach_widget(Action* a, Widget* w) { remove(a->widgets_, w); }

    static const std::string& text(const Action* a) { return a->text_; }
    static const Shortcut& shortcut(const Action* a) { return a->shortcut_; }
    static bool enabled(const Action* a) { return a->enabled_; }
    static bool checkable(const Action* a) { return a->checkable_; }
    static bool checked(const Action* a) { return a->checked_; }
};

}  // namespace detail
}  // namespace gui

#endif
