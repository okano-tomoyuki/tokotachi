/// @file
/// 値を 1 つ持つオブジェクト(docs/adr/0016)。プロパティに結びつけて、複数のウィジェットで値を共有する。
///
/// @code
/// tokotachi::Variable<std::string>* name = new tokotachi::Variable<std::string>(&window);
/// tokotachi::bind(line_edit_name->text, name->value);
/// tokotachi::bind(label_name->text, name->value);
/// name->value = "初期値";                             // line_edit_name と label_name の両方に伝わる
/// name->on_change = [](tokotachi::Object* sender) { ... };  // 値が変わったときに呼ぶ
/// name->on_set = [](tokotachi::Object* sender) { ... };     // 値が設定されるたびに呼ぶ(同じ値の代入でも)
/// @endcode
///
/// 値の型は == で比べられること。
#ifndef TOKOTACHI_CORE_VARIABLE_HPP
#define TOKOTACHI_CORE_VARIABLE_HPP

#include "event.hpp"
#include "property.hpp"

namespace tokotachi
{

template <typename T>
class Variable : public Object
{
public:
    explicit Variable(Object* owner_object = nullptr, const T& initial = T())
        : Object(owner_object),
          value(this, &Variable::get_value, &Variable::set_value),
          on_set(this, &Variable::get_on_set, &Variable::set_on_set),
          on_change(this, &Variable::get_on_change, &Variable::set_on_change),
          value_(initial)
    {
    }

    Property<T> value;
    /// value が設定された(同じ値の代入でも呼ぶ。docs/adr/0016)。
    Property<NotifyHandler> on_set;
    /// value が変わった(結びつけた相手から伝わったときも含む)。on_set の後に呼ぶ。
    Property<NotifyHandler> on_change;

private:
    static T get_value(Object* o) { return static_cast<Variable*>(o)->value_; }

    static void set_value(Object* o, const T& v)
    {
        Variable* self = static_cast<Variable*>(o);
        const bool changed = !(self->value_ == v);
        self->value_ = v;
        /// 結びつけた相手へ伝えてから、ハンドラを呼ぶ(ハンドラの中で相手の値がそろっているように)。
        if (changed)
        {
            self->notify_changed(self->value);
        }
        /// ハンドラの中でこの変数が破棄されてもよいよう、両方を写してから呼び、呼んだ後は self に触らない。
        const NotifyHandler on_set_handler = self->on_set_;
        const NotifyHandler on_change_handler = changed ? self->on_change_ : NotifyHandler();
        if (on_set_handler)
        {
            on_set_handler(o);
        }
        if (on_change_handler)
        {
            on_change_handler(o);
        }
    }

    static NotifyHandler get_on_set(Object* o) { return static_cast<Variable*>(o)->on_set_; }
    static void set_on_set(Object* o, const NotifyHandler& h) { static_cast<Variable*>(o)->on_set_ = h; }
    static NotifyHandler get_on_change(Object* o) { return static_cast<Variable*>(o)->on_change_; }
    static void set_on_change(Object* o, const NotifyHandler& h) { static_cast<Variable*>(o)->on_change_ = h; }

    T value_;
    NotifyHandler on_set_;
    NotifyHandler on_change_;
};

typedef Variable<std::string> StringVariable;
typedef Variable<int> IntVariable;
typedef Variable<bool> BoolVariable;
typedef Variable<double> DoubleVariable;

}  // namespace tokotachi

#endif
