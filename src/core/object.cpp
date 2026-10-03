#include <gui/property.hpp>

#include <algorithm>

#include "binding_registry.hpp"

namespace gui
{

Object::Object(Object* owner_object) : owner(this, &Object::get_owner), owner_(owner_object)
{
    if (owner_)
    {
        owner_->owned_.push_back(this);
    }
}

Object::~Object()
{
    delete_owned();
    if (owner_)
    {
        std::vector<Object*>& siblings = owner_->owned_;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    }
    detail::remove_bindings_of(this);
}

void Object::change_owner(Object* new_owner)
{
    if (new_owner == owner_)
    {
        return;
    }
    if (owner_)
    {
        std::vector<Object*>& siblings = owner_->owned_;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    }
    owner_ = new_owner;
    if (owner_)
    {
        owner_->owned_.push_back(this);
    }
}

void Object::delete_owned()
{
    // 所有しているオブジェクトのデストラクタが、自分を一覧から外す。
    while (!owned_.empty())
    {
        delete owned_.back();
    }
}

}  // namespace gui
