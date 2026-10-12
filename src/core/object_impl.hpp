// Object の実装(内部。docs/adr/0057)。
#ifndef TOKOTACHI_SRC_CORE_OBJECT_IMPL_HPP
#define TOKOTACHI_SRC_CORE_OBJECT_IMPL_HPP

#include <memory>
#include <vector>

#include <tokotachi/core/property.hpp>

#include "impl.hpp"

namespace tokotachi
{

class Object::Impl
{
    TOKOTACHI_IMPL(Object);

    Impl(Object* object, Object* owner_object) : q(object), owner_(owner_object) {}

    Object* get_owner() const { return owner_; }

    /// 所有者を new_owner に移す(Widget の親の付け替え・ダイアログの持ち主の付け替えに使う。docs/adr/0018)。
    void change_owner(Object* new_owner);

    Object* owner_;
    std::vector<Object*> owned_;
    /// WeakRef が作られたときだけ持つ。破棄するときに alive を false にする。
    std::shared_ptr<detail::Liveness> liveness_;
};

namespace detail
{

/// o の所有者を new_owner に移す。
inline void change_owner(Object* o, Object* new_owner) { ImplAccess<Object>::of(o)->change_owner(new_owner); }

}  // namespace detail
}  // namespace tokotachi

#endif
