#include <tokotachi/actions/action.hpp>

#include <tokotachi/actions/menu.hpp>
#include <tokotachi/widgets/widget.hpp>

#include "menu_access.hpp"
#include "action_access.hpp"
#include "../widgets/widget_access.hpp"
#include "../core/impl.hpp"

namespace tokotachi
{

Action::Action(Object* owner_object, const std::string& initial_text)
    : Object(owner_object),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, text),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, Shortcut, shortcut),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, Image, icon),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, tooltip),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, enabled),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, checkable),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, checked),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_trigger),
      d_(new Impl(this))
{
    d_->text_ = initial_text;
    d_->enabled_ = true;
    d_->checkable_ = false;
    d_->checked_ = false;
}

Action::~Action()
{
    // 入れてあるメニュー・ウィジェットから外す(それぞれが自分の一覧を書き換えるので、写しを回す)。
    const std::vector<Menu*> menus = d_->menus_;
    for (std::size_t i = 0; i < menus.size(); ++i)
    {
        detail::MenuAccess::forget_action(menus[i], this);
    }
    const std::vector<Widget*> widgets = d_->widgets_;
    for (std::size_t i = 0; i < widgets.size(); ++i)
    {
        widgets[i]->remove_action(this);
    }
    delete d_;
}

void Action::Impl::changed()
{
    for (std::size_t i = 0; i < menus_.size(); ++i)
    {
        detail::MenuAccess::action_changed(menus_[i]);
    }
    const std::vector<Widget*> widgets = widgets_;
    for (std::size_t i = 0; i < widgets.size(); ++i)
    {
        detail::WidgetAccess::action_changed(widgets[i], q);
    }
}

void Action::Impl::set_text(const std::string& v)
{
    text_ = v;
    changed();
}

void Action::Impl::set_shortcut(const Shortcut& v)
{
    shortcut_ = v;
    changed();
}

void Action::Impl::set_icon(const Image& v)
{
    icon_ = v;
    changed();
}

void Action::Impl::set_tooltip(const std::string& v)
{
    tooltip_ = v;
    changed();
}

void Action::Impl::set_enabled(const bool& v)
{
    enabled_ = v;
    changed();
}

void Action::Impl::set_checkable(const bool& v)
{
    checkable_ = v;
    changed();
}

void Action::Impl::set_checked(const bool& v)
{
    checked_ = v;
    changed();
}

void Action::trigger()
{
    if (!d_->enabled_)
    {
        return;
    }
    if (d_->checkable_)
    {
        checked = !d_->checked_;
    }
    Object* self = this;
    detail::fire(d_->on_trigger_, self);
}

}  // namespace tokotachi
