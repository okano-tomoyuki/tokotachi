#include <tokotachi/actions/action.hpp>

#include <tokotachi/actions/menu.hpp>
#include <tokotachi/widgets/widget.hpp>

#include "menu_access.hpp"
#include "action_access.hpp"
#include "../widgets/widget_access.hpp"
#include "../core/property_impl.hpp"

namespace tokotachi
{

Action::Action(Object* owner_object, const std::string& initial_text)
    : Object(owner_object),
      TOKOTACHI_PROPERTY_MEMBER(Action, std::string, text),
      TOKOTACHI_PROPERTY_MEMBER(Action, Shortcut, shortcut),
      TOKOTACHI_PROPERTY_MEMBER(Action, Image, icon),
      TOKOTACHI_PROPERTY_MEMBER(Action, std::string, tooltip),
      TOKOTACHI_PROPERTY_MEMBER(Action, bool, enabled),
      TOKOTACHI_PROPERTY_MEMBER(Action, bool, checkable),
      TOKOTACHI_PROPERTY_MEMBER(Action, bool, checked),
      TOKOTACHI_HANDLER_PROPERTY(Action, NotifyHandler, on_trigger),
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
    const std::vector<Widget*> widgets = widgets_;
    for (std::size_t i = 0; i < widgets.size(); ++i)
    {
        detail::WidgetAccess::action_changed(widgets[i], this);
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

void Action::set_icon(const Image& v)
{
    icon_ = v;
    changed();
}

void Action::set_tooltip(const std::string& v)
{
    tooltip_ = v;
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

}  // namespace tokotachi
