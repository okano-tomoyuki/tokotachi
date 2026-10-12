#include <tokotachi/core/property.hpp>

#include <algorithm>
#include <utility>

#include <tokotachi/core/application.hpp>

#include "application_impl.hpp"
#include "binding_registry.hpp"
#include "object_impl.hpp"

namespace tokotachi
{

namespace
{

// 破棄の通知を受け取る関数(メインスレッドだけで触る)。
std::vector<std::pair<detail::DestroyObserver, void*> >& destroy_observers()
{
    static std::vector<std::pair<detail::DestroyObserver, void*> > observers;
    return observers;
}

}  // namespace

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
    if (!destroy_observers().empty())
    {
        // 通知の中で加える・外すこともできるよう、写してから呼ぶ。
        const std::vector<std::pair<detail::DestroyObserver, void*> > observers = destroy_observers();
        for (std::size_t i = 0; i < observers.size(); ++i)
        {
            observers[i].first(this, observers[i].second);
        }
    }
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

void add_destroy_observer(DestroyObserver fn, void* data)
{
    if (fn)
    {
        destroy_observers().push_back(std::make_pair(fn, data));
    }
}

void remove_destroy_observer(DestroyObserver fn, void* data)
{
    std::vector<std::pair<DestroyObserver, void*> >& observers = destroy_observers();
    observers.erase(std::remove(observers.begin(), observers.end(), std::make_pair(fn, data)), observers.end());
}

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
