/// @file
/// プロパティどうしの結びつけ(docs/adr/0016)。
///
/// @code
/// tokotachi::bind(line_edit_name->text, name->value);   // 双方向。結んだ時点で第 1 引数を第 2 引数の値にする
/// tokotachi::bind(label_name->text, name->value);  // name を介して line_edit_name と label_name がそろう
/// tokotachi::bind(label_name->text, line_edit_name->selected_text);  // 読み取り専用のプロパティからは一方向
/// tokotachi::unbind(label_name->text);             // label_name->text の結びつきをすべて外す
/// @endcode
///
/// 結べるのは同じ型(== で比べられる型)のプロパティだけ。値が変わったときだけ相手へ伝える。
/// どちらかのオブジェクトが破棄されると、その結びつきは自動で外れる。結びつきが循環していても止まる。
#ifndef TOKOTACHI_CORE_BINDING_HPP
#define TOKOTACHI_CORE_BINDING_HPP

#include "property.hpp"

namespace tokotachi
{

namespace detail
{

struct PropertyAccess
{
    template <typename T>
    static Object* owner(const Property<T>& p)
    {
        return p.owner_;
    }

    template <typename T>
    static Object* owner(const ReadOnlyProperty<T>& p)
    {
        return p.owner_;
    }

    template <typename T>
    static typename Property<T>::Getter getter(const Property<T>& p)
    {
        return p.getter_;
    }

    template <typename T>
    static typename ReadOnlyProperty<T>::Getter getter(const ReadOnlyProperty<T>& p)
    {
        return p.getter_;
    }

    template <typename T>
    static typename Property<T>::Setter setter(const Property<T>& p)
    {
        return p.setter_;
    }
};

struct BindingEnd
{
    Object* owner;
    const void* property;
};

/// 結びつき(内部)。どちらかの端が設定されたら、もう一方の端へ値を書き込む。
class BindingBase
{
public:
    BindingBase(const BindingEnd& a_, const BindingEnd& b_) : a(a_), b(b_), active(false) {}
    virtual ~BindingBase() {}

    /// from_a なら a の値を b へ、そうでなければ b の値を a へ書き込む。書き込めない端・すでに同じ値の端へは何もしない
    /// (同じ値を書き込むと、LineEdit のキャレットが末尾へ動くなどの副作用があるため)。
    virtual void propagate(bool from_a) = 0;

    BindingEnd a, b;
    bool active;  ///< 伝えている最中(循環を止める)
};

template <typename T>
class TypedBinding : public BindingBase
{
public:
    typedef T (*Getter)(Object*);
    typedef void (*Setter)(Object*, const T&);

    TypedBinding(const BindingEnd& a_, Getter get_a_, Setter set_a_, const BindingEnd& b_, Getter get_b_,
                 Setter set_b_)
        : BindingBase(a_, b_), get_a(get_a_), set_a(set_a_), get_b(get_b_), set_b(set_b_)
    {
    }

    void propagate(bool from_a) override
    {
        if (from_a)
        {
            const T v = get_a(a.owner);
            if (set_b && !(get_b(b.owner) == v))
            {
                set_b(b.owner, v);
            }
        }
        else
        {
            const T v = get_b(b.owner);
            if (set_a && !(get_a(a.owner) == v))
            {
                set_a(a.owner, v);
            }
        }
    }

private:
    Getter get_a;
    Setter set_a;
    Getter get_b;
    Setter set_b;
};

/// 結びつきを登録し、第 1 の端を第 2 の端の値にそろえる。同じ組がすでにあれば何もしない(false)。
bool add_binding(BindingBase* binding);
void remove_bindings(Object* owner, const void* property);
void remove_binding_pair(const void* a, const void* b);

}  // namespace detail

/// target と source を双方向に結び、target を source の値にする。
template <typename T>
void bind(Property<T>& target, Property<T>& source)
{
    typedef detail::PropertyAccess A;
    const detail::BindingEnd a = {A::owner(target), &target};
    const detail::BindingEnd b = {A::owner(source), &source};
    detail::add_binding(
        new detail::TypedBinding<T>(a, A::getter(target), A::setter(target), b, A::getter(source), A::setter(source)));
}

/// 読み取り専用の source から target へ一方向に結び、target を source の値にする。
/// source の値が変わったことは、source を持つオブジェクトが知らせる(知らせないプロパティもある)。
template <typename T>
void bind(Property<T>& target, ReadOnlyProperty<T>& source)
{
    typedef detail::PropertyAccess A;
    const detail::BindingEnd a = {A::owner(target), &target};
    const detail::BindingEnd b = {A::owner(source), &source};
    detail::add_binding(new detail::TypedBinding<T>(a, A::getter(target), A::setter(target), b, A::getter(source),
                                                    nullptr));
}

/// a と b の結びつきを外す。
template <typename T, typename U>
void unbind(T& a, U& b)
{
    detail::remove_binding_pair(&a, &b);
}

/// property の結びつきをすべて外す。
template <typename T>
void unbind(Property<T>& property)
{
    detail::remove_bindings(detail::PropertyAccess::owner(property), &property);
}

template <typename T>
void unbind(ReadOnlyProperty<T>& property)
{
    detail::remove_bindings(detail::PropertyAccess::owner(property), &property);
}

}  // namespace tokotachi

#endif
