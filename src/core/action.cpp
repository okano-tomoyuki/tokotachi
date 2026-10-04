#include <gui/action.hpp>

#include <gui/menu.hpp>
#include <gui/widget.hpp>

#include "../widgets/menu_access.hpp"
#include "action_access.hpp"
#include "property_impl.hpp"

namespace gui
{

Action::Action(Object* owner_object, const std::string& initial_text)
    : Object(owner_object),
      GUI_PROPERTY_MEMBER(Action, std::string, text),
      GUI_PROPERTY_MEMBER(Action, Shortcut, shortcut),
      GUI_PROPERTY_MEMBER(Action, bool, enabled),
      GUI_PROPERTY_MEMBER(Action, bool, checkable),
      GUI_PROPERTY_MEMBER(Action, bool, checked),
      GUI_HANDLER_PROPERTY(Action, NotifyHandler, on_trigger),
      text_(initial_text),
      enabled_(true),
      checkable_(false),
      checked_(false)
{
}

Action::~Action()
{
    // 入れてあるメニュー・ウィジェットから外す(それぞれが自分の一覧を書き換えるので、写しを回す)。
    const std::vector<Menu*> menus = menus_;
    for (std::size_t i = 0; i < menus.size(); ++i)
    {
        detail::MenuAccess::forget_action(menus[i], this);
    }
    const std::vector<Widget*> widgets = widgets_;
    for (std::size_t i = 0; i < widgets.size(); ++i)
    {
        widgets[i]->remove_action(this);
    }
}

void Action::changed()
{
    for (std::size_t i = 0; i < menus_.size(); ++i)
    {
        detail::MenuAccess::action_changed(menus_[i]);
    }
}

void Action::set_text(const std::string& v)
{
    text_ = v;
    changed();
}

void Action::set_shortcut(const Shortcut& v)
{
    shortcut_ = v;
    changed();
}

void Action::set_enabled(const bool& v)
{
    enabled_ = v;
    changed();
}

void Action::set_checkable(const bool& v)
{
    checkable_ = v;
    changed();
}

void Action::set_checked(const bool& v)
{
    checked_ = v;
    changed();
}

void Action::trigger()
{
    if (!enabled_)
    {
        return;
    }
    if (checkable_)
    {
        checked = !checked_;
    }
    Object* self = this;
    detail::fire(on_trigger_, self);
}

}  // namespace gui
