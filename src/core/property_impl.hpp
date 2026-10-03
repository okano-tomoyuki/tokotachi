// ライブラリの中でプロパティを定義するためのヘルパー(内部。docs/adr/0013、0016)。
//
// コンストラクタの初期化子で、メンバ変数・private の getter/setter からプロパティの関数を作る:
//   GUI_PROPERTY_MEMBER(Label, std::string, text)      読み取りは text_、書き込みは set_text
//   GUI_PROPERTY_GETTER(Timer, int, interval)          読み取りは get_interval、書き込みは set_interval
//   GUI_READONLY_PROPERTY(Widget, Window*, window)     読み取りは get_window
//   GUI_HANDLER_PROPERTY(Button, NotifyHandler, on_click)  読み書きとも on_click_
//
// 変更の通知(docs/adr/0016): 値が変わったときだけ知らせる。
//   GUI_PROPERTY_MEMBER・GUI_PROPERTY_GETTER は、書き込みの関数の前後で値を比べ、変わっていれば自動で知らせる
//   (値の型に == が要る)。書き込みの関数は、一緒に変わった他のプロパティだけを知らせる。
//   この書き込みの関数の中で利用者のハンドラを呼んではいけない(ハンドラがオブジェクトを破棄すると、後で値を読めない)。
//   ..._SELF_NOTIFY は自動で知らせない。書き込みの関数が自分で知らせる(ハンドラを呼ぶとき・一緒に変わるプロパティと
//   まとめて、ハンドラより先に知らせたいとき)。
#ifndef GUI_SRC_CORE_PROPERTY_IMPL_HPP
#define GUI_SRC_CORE_PROPERTY_IMPL_HPP

#include <gui/property.hpp>

namespace gui
{
namespace detail
{

template <typename C, typename T, T C::*M>
T read_member(Object* o)
{
    return static_cast<C*>(o)->*M;
}

template <typename C, typename T, T C::*M>
void write_member(Object* o, const T& v)
{
    static_cast<C*>(o)->*M = v;
}

template <typename C, typename T, T (C::*G)() const>
T call_getter(Object* o)
{
    return (static_cast<C*>(o)->*G)();
}

template <typename C, typename T, void (C::*S)(const T&)>
void call_setter(Object* o, const T& v)
{
    (static_cast<C*>(o)->*S)(v);
}

// 書き込みの関数を呼び、プロパティ P の値が変わっていれば知らせる。
template <typename C, typename T, void (C::*S)(const T&), Property<T> C::*P>
void call_setter_and_notify(Object* o, const T& v)
{
    C* c = static_cast<C*>(o);
    const Property<T>& property = c->*P;
    const T old = property.get();
    (c->*S)(v);
    if (!(property.get() == old))
    {
        property_changed(o, &property);
    }
}

// ハンドラを写してから呼ぶ(ハンドラの中でハンドラを差し替えてもよいように。docs/adr/0013)。
template <typename H, typename... A>
void fire(const H& handler, A&... args)
{
    const H copy(handler);
    if (copy)
    {
        copy(args...);
    }
}

}  // namespace detail
}  // namespace gui

#define GUI_PROPERTY_MEMBER(C, T, name)                                                                                \
    name(this, &::gui::detail::read_member<C, T, &C::name##_>,                                                         \
         &::gui::detail::call_setter_and_notify<C, T, &C::set_##name, &C::name>)

#define GUI_PROPERTY_GETTER(C, T, name)                                                                                \
    name(this, &::gui::detail::call_getter<C, T, &C::get_##name>,                                                      \
         &::gui::detail::call_setter_and_notify<C, T, &C::set_##name, &C::name>)

#define GUI_PROPERTY_MEMBER_SELF_NOTIFY(C, T, name)                                                                    \
    name(this, &::gui::detail::read_member<C, T, &C::name##_>, &::gui::detail::call_setter<C, T, &C::set_##name>)

#define GUI_PROPERTY_GETTER_SELF_NOTIFY(C, T, name)                                                                    \
    name(this, &::gui::detail::call_getter<C, T, &C::get_##name>, &::gui::detail::call_setter<C, T, &C::set_##name>)

#define GUI_READONLY_PROPERTY(C, T, name) name(this, &::gui::detail::call_getter<C, T, &C::get_##name>)

#define GUI_HANDLER_PROPERTY(C, T, name)                                                                               \
    name(this, &::gui::detail::read_member<C, T, &C::name##_>, &::gui::detail::write_member<C, T, &C::name##_>)

#endif
