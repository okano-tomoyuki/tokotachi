// 公開のクラスの実装のクラス(pimpl。内部。docs/adr/0057)。
//
// 公開のクラスは、非公開のデータと関数を入れ子の実装のクラス Impl に移し、ヘッダには宣言とポインタ d_ だけを残す:
//   class TOKOTACHI_API Button : public ButtonBase
//   {
//       ...
//   private:
//       class Impl;
//       Impl* d_;
//   };
// 入れ子のクラスは外側のクラスのメンバと同じ扱いなので、Impl は friend 無しで Button の private のメンバに触れ、
// q を通して基底の protected のメンバも呼べる。Impl は段ごとに持つ(Widget::Impl と Button::Impl は別の型)。
//
// 実装のクラスは TOKOTACHI_IMPL(Button) で、公開のクラスへのポインタ q と、公開のクラスから自分を得る of を持つ:
//   class Button::Impl
//   {
//       TOKOTACHI_IMPL(Button);
//       ...
//   };
//
// ほかのクラスの実装のコードから届くクラスは、ヘッダで窓口を friend にする:
//   template <typename> friend struct detail::ImplAccess;
// 窓口は ImplAccess<Layout>::of(layout) で Layout::Impl を返す(private の入れ子の型なので、呼ぶ側は auto で受けるか、
// 窓口の Impl の別名を使う)。protected のメンバを呼ぶなどの窓口の関数が要れば、ImplAccess<Layout> を特殊化し、
// TOKOTACHI_IMPL_ACCESS(Layout) で of を持たせる。
//
// プロパティは、実装のクラスのメンバ・関数から作る(property_impl.hpp の TOKOTACHI_PROPERTY_... の実装のクラス版):
//   TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, ButtonStyle, style)   読み取りは Impl::style_、書き込みは Impl::set_style
//   TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, int, interval)        読み取りは Impl::get_interval、書き込みは Impl::set_interval
//   TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Window*, window)    読み取りは Impl::get_window
//   TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_click)  読み書きとも Impl::on_click_
// 変更の通知の決まりは property_impl.hpp と同じ。
#ifndef TOKOTACHI_SRC_CORE_IMPL_HPP
#define TOKOTACHI_SRC_CORE_IMPL_HPP

#include <tokotachi/core/property.hpp>

#include "property_impl.hpp"

// 実装のクラス(P::Impl)の中に書く。P は公開のクラス。
#define TOKOTACHI_IMPL(P)                                                                                                    \
public:                                                                                                                      \
    typedef ::tokotachi::P Public;                                                                                           \
    static Impl* of(::tokotachi::Object* o) { return static_cast<::tokotachi::P*>(o)->d_; }                                  \
    static const Impl* of(const ::tokotachi::Object* o) { return static_cast<const ::tokotachi::P*>(o)->d_; }                \
    ::tokotachi::P* q

// ImplAccess<P> を特殊化するときに、その中に書く。
#define TOKOTACHI_IMPL_ACCESS(P)                                                                                             \
    typedef ::tokotachi::P::Impl Impl;                                                                                       \
    static Impl* of(::tokotachi::P* p) { return p->d_; }                                                                     \
    static const Impl* of(const ::tokotachi::P* p) { return p->d_; }

namespace tokotachi
{
namespace detail
{

// 窓口の既定(of だけを持つ)。
template <typename C>
struct ImplAccess
{
    typedef typename C::Impl Impl;
    static Impl* of(C* c) { return c->d_; }
    static const Impl* of(const C* c) { return c->d_; }
};

// c の実装のクラス(C::Impl。C の窓口を通す)。
template <typename C>
typename ImplAccess<C>::Impl* impl_of(C* c)
{
    return ImplAccess<C>::of(c);
}

template <typename C>
const typename ImplAccess<C>::Impl* impl_of(const C* c)
{
    return ImplAccess<C>::of(c);
}

template <typename I, typename T, T I::*M>
T read_impl_member(Object* o)
{
    return I::of(o)->*M;
}

template <typename I, typename T, T I::*M>
void write_impl_member(Object* o, const T& v)
{
    I::of(o)->*M = v;
}

template <typename I, typename T, T (I::*G)() const>
T call_impl_getter(Object* o)
{
    return (I::of(o)->*G)();
}

template <typename I, typename T, void (I::*S)(const T&)>
void call_impl_setter(Object* o, const T& v)
{
    (I::of(o)->*S)(v);
}

// 書き込みの関数を呼び、公開のクラスのプロパティ P の値が変わっていれば知らせる。
template <typename I, typename T, void (I::*S)(const T&), Property<T> I::Public::*P>
void call_impl_setter_and_notify(Object* o, const T& v)
{
    typename I::Public* c = static_cast<typename I::Public*>(o);
    const Property<T>& property = c->*P;
    const T old = property.get();
    (I::of(o)->*S)(v);
    if (!(property.get() == old))
    {
        property_changed(o, &property);
    }
}

}  // namespace detail
}  // namespace tokotachi

#define TOKOTACHI_IMPL_PROPERTY_MEMBER(I, T, name)                                                                           \
    name(this, &::tokotachi::detail::read_impl_member<I, T, &I::name##_>,                                                    \
         &::tokotachi::detail::call_impl_setter_and_notify<I, T, &I::set_##name, &I::Public::name>)

#define TOKOTACHI_IMPL_PROPERTY_GETTER(I, T, name)                                                                           \
    name(this, &::tokotachi::detail::call_impl_getter<I, T, &I::get_##name>,                                                 \
         &::tokotachi::detail::call_impl_setter_and_notify<I, T, &I::set_##name, &I::Public::name>)

#define TOKOTACHI_IMPL_PROPERTY_MEMBER_SELF_NOTIFY(I, T, name)                                                               \
    name(this, &::tokotachi::detail::read_impl_member<I, T, &I::name##_>,                                                    \
         &::tokotachi::detail::call_impl_setter<I, T, &I::set_##name>)

#define TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(I, T, name)                                                               \
    name(this, &::tokotachi::detail::call_impl_getter<I, T, &I::get_##name>,                                                 \
         &::tokotachi::detail::call_impl_setter<I, T, &I::set_##name>)

#define TOKOTACHI_IMPL_READONLY_PROPERTY(I, T, name) name(this, &::tokotachi::detail::call_impl_getter<I, T, &I::get_##name>)

#define TOKOTACHI_IMPL_HANDLER_PROPERTY(I, T, name)                                                                          \
    name(this, &::tokotachi::detail::read_impl_member<I, T, &I::name##_>,                                                    \
         &::tokotachi::detail::write_impl_member<I, T, &I::name##_>)

#endif
