// Action の内部の状態に触るためのヘルパー(内部)。
#ifndef TOKOTACHI_SRC_ACTIONS_ACTION_ACCESS_HPP
#define TOKOTACHI_SRC_ACTIONS_ACTION_ACCESS_HPP

#include <algorithm>
#include <vector>

#include <tokotachi/actions/action.hpp>
#include "../core/impl.hpp"

namespace tokotachi
{

class Action::Impl
{
    TOKOTACHI_IMPL(Action);

    explicit Impl(Action* object) : q(object) {}

    void set_text(const std::string& v);
    void set_shortcut(const Shortcut& v);
    void set_icon(const Image& v);
    void set_tooltip(const std::string& v);
    void set_enabled(const bool& v);
    void set_checkable(const bool& v);
    void set_checked(const bool& v);
    void changed();

    std::string text_;
    Shortcut shortcut_;
    Image icon_;
    std::string tooltip_;
    bool enabled_;
    bool checkable_;
    bool checked_;
    NotifyHandler on_trigger_;
    /// この Action を入れたメニュー・ウィジェット(破棄するときに外すため)。
    std::vector<Menu*> menus_;
    std::vector<Widget*> widgets_;
};
namespace detail
{

template <>
struct ImplAccess<Action>
{
    TOKOTACHI_IMPL_ACCESS(Action)

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

    static void attach_menu(Action* a, Menu* m) { add_unique(of(a)->menus_, m); }
    static void detach_menu(Action* a, Menu* m) { remove(of(a)->menus_, m); }
    static void attach_widget(Action* a, Widget* w) { add_unique(of(a)->widgets_, w); }
    static void detach_widget(Action* a, Widget* w) { remove(of(a)->widgets_, w); }

    static const std::string& text(const Action* a) { return of(a)->text_; }
    static const Shortcut& shortcut(const Action* a) { return of(a)->shortcut_; }
    static const Image& icon(const Action* a) { return of(a)->icon_; }
    static const std::string& tooltip(const Action* a) { return of(a)->tooltip_; }
    static bool enabled(const Action* a) { return of(a)->enabled_; }
    static bool checkable(const Action* a) { return of(a)->checkable_; }
    static bool checked(const Action* a) { return of(a)->checked_; }
};

typedef ImplAccess<Action> ActionAccess;

}  // namespace detail
}  // namespace tokotachi

#endif
