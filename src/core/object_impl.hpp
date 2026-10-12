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

/// Object が破棄されることを受け取る関数(内部。将来の C の層が、ほかの言語の側の参照を無効にするために使う。docs/adr/0057)。
/// ~Object の最初に、破棄される順に呼ぶ。そのときには派生クラスの部分はもう破棄されているので、object はポインタの同一性
/// (ハンドルを無効にする)にだけ使い、メンバを呼ばない。メインスレッドで呼ぶ。
typedef void (*DestroyObserver)(Object* object, void* data);

/// 破棄の通知を受け取る関数を加える・外す(同じ fn と data の組で外す)。メインスレッドで呼ぶ。通知の中で加える・外すこともできる
/// (その通知には反映しない)。
TOKOTACHI_API void add_destroy_observer(DestroyObserver fn, void* data);
TOKOTACHI_API void remove_destroy_observer(DestroyObserver fn, void* data);

}  // namespace detail
}  // namespace tokotachi

#endif
