#include <tokotachi/core/property.hpp>

#include <algorithm>

#include <tokotachi/core/application.hpp>

#include "application_impl.hpp"
#include "binding_registry.hpp"
#include "object_impl.hpp"

namespace tokotachi
{

Object::Object(Object* owner_object)
    : TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Object*, owner), d_(new Impl(this, owner_object))
{
    if (owner_object)
    {
        owner_object->d_->owned_.push_back(this);
    }
}

Object::~Object()
{
    if (d_->liveness_)
    {
        d_->liveness_->alive = false;
    }
    if (Application* app = Application::instance())
    {
        std::vector<Object*>& doomed = detail::impl_of(app)->doomed;
        if (!doomed.empty())
        {
            doomed.erase(std::remove(doomed.begin(), doomed.end(), this), doomed.end());
        }
    }
    delete_owned();
    if (d_->owner_)
    {
        std::vector<Object*>& siblings = d_->owner_->d_->owned_;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    }
    detail::remove_bindings_of(this);
    delete d_;
}

void Object::delete_later()
{
    Application* app = Application::instance();
    if (!app)
    {
        delete this;
        return;
    }
    std::vector<Object*>& doomed = detail::impl_of(app)->doomed;
    if (std::find(doomed.begin(), doomed.end(), this) == doomed.end())
    {
        doomed.push_back(this);
    }
}

void Object::Impl::change_owner(Object* new_owner)
{
    if (new_owner == owner_)
    {
        return;
    }
    if (owner_)
    {
        std::vector<Object*>& siblings = owner_->d_->owned_;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), q), siblings.end());
    }
    owner_ = new_owner;
    if (owner_)
    {
        owner_->d_->owned_.push_back(q);
    }
}

void Object::delete_owned()
{
    // 所有しているオブジェクトのデストラクタが、自分を一覧から外す。
    while (!d_->owned_.empty())
    {
        delete d_->owned_.back();
    }
}

namespace detail
{

std::shared_ptr<Liveness> liveness_of(Object* o)
{
    std::shared_ptr<Liveness>& l = ImplAccess<Object>::of(o)->liveness_;
    if (!l)
    {
        l = std::make_shared<Liveness>();
    }
    return l;
}

}  // namespace detail

}  // namespace tokotachi
