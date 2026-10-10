/// @file
/// Object とプロパティ(docs/adr/0013、0015、0016)。
///
/// プロパティは、所有者(Object*)と、値の読み取り・書き込みの関数ポインタを持つ軽量のプロキシ(ヒープの確保なし)。
/// @code
/// label_name->text = "abc";                  // 書き込み
/// std::string s = label_name->text;          // 読み取り(T への暗黙の変換)
/// label_name->text.get().size();             // 値のメソッドを呼ぶときは get()
/// button_ok->x += 10;                      // 複合代入
/// label_name->text = "名前: " + line_edit_name->text;  // std::string のプロパティは + で直接つなげる
/// @endcode
///
/// プロパティの値が変わったら、所有者が知らせ(Object::notify_changed)、結びつけた相手のプロパティへ伝わる(binding.hpp)。
#ifndef TOKOTACHI_CORE_PROPERTY_HPP
#define TOKOTACHI_CORE_PROPERTY_HPP

#include <memory>
#include <string>
#include <vector>

namespace tokotachi
{

class Object;

namespace detail
{
struct PropertyAccess;
/// owner の property(Property・ReadOnlyProperty のアドレス)の値が変わったことを知らせる。
/// 結びつけた相手のプロパティへ値を伝える(docs/adr/0016)。
void property_changed(Object* owner, const void* property);

/// Object が生きているかの目印(WeakRef が共有する。docs/adr/0029)。メインスレッドだけで読み書きする。
struct Liveness
{
    bool alive;
    Liveness() : alive(true) {}
};
}  // namespace detail

template <typename T>
class WeakRef;

template <typename T>
class Property
{
public:
    typedef T ValueType;
    typedef T (*Getter)(Object*);
    typedef void (*Setter)(Object*, const T&);

    Property(Object* owner, Getter getter, Setter setter) : owner_(owner), getter_(getter), setter_(setter) {}

    /// コピーの構築は所有者を取り違えるので禁止する。プロパティ同士の代入は値のコピーとして扱う(所有者は付け替えない)。
    Property(const Property&) = delete;
    Property& operator=(const Property& other)
    {
        setter_(owner_, other.get());
        return *this;
    }

    Property& operator=(const T& value)
    {
        setter_(owner_, value);
        return *this;
    }

    operator T() const { return getter_(owner_); }

    /// 値を取り出す(auto で受ける・値のメソッドを呼ぶ・テンプレートに渡すなど、暗黙の変換が効かない場面で使う)。
    T get() const { return getter_(owner_); }

    /// ポインタ型のプロパティで、メンバへ直接たどれるようにする(widget->parent->x)。
    T operator->() const { return getter_(owner_); }

    template <typename U>
    Property& operator+=(const U& value)
    {
        T v = get();
        v += value;
        setter_(owner_, v);
        return *this;
    }

    template <typename U>
    Property& operator-=(const U& value)
    {
        T v = get();
        v -= value;
        setter_(owner_, v);
        return *this;
    }

    template <typename U>
    Property& operator*=(const U& value)
    {
        T v = get();
        v *= value;
        setter_(owner_, v);
        return *this;
    }

    template <typename U>
    Property& operator/=(const U& value)
    {
        T v = get();
        v /= value;
        setter_(owner_, v);
        return *this;
    }

    Property& operator++()
    {
        T v = get();
        ++v;
        setter_(owner_, v);
        return *this;
    }

    Property& operator--()
    {
        T v = get();
        --v;
        setter_(owner_, v);
        return *this;
    }

    T operator++(int)
    {
        T old = get();
        T v = old;
        ++v;
        setter_(owner_, v);
        return old;
    }

    T operator--(int)
    {
        T old = get();
        T v = old;
        --v;
        setter_(owner_, v);
        return old;
    }

private:
    friend struct detail::PropertyAccess;

    Object* owner_;
    Getter getter_;
    Setter setter_;
};

template <typename T>
class ReadOnlyProperty
{
public:
    typedef T ValueType;
    typedef T (*Getter)(Object*);

    ReadOnlyProperty(Object* owner, Getter getter) : owner_(owner), getter_(getter) {}

    ReadOnlyProperty(const ReadOnlyProperty&) = delete;
    ReadOnlyProperty& operator=(const ReadOnlyProperty&) = delete;

    operator T() const { return getter_(owner_); }
    T get() const { return getter_(owner_); }
    T operator->() const { return getter_(owner_); }

private:
    friend struct detail::PropertyAccess;

    Object* owner_;
    Getter getter_;
};

/// プロパティを持つオブジェクトの基底(docs/adr/0015)。コピーできない(プロパティが所有者を指すため)。
///
/// 所有: owner を渡すと、owner が破棄されるときに一緒に破棄される(new で作る)。
/// owner が nullptr なら利用者が寿命を管理する(スタック・メンバ変数にも置ける)。
/// owner を持つオブジェクトをスタックに置いてはいけない(二重に破棄される)。
class Object
{
public:
    explicit Object(Object* owner_object = nullptr);
    virtual ~Object();

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;

    ReadOnlyProperty<Object*> owner;

    /// 今のイベントの処理が終わった後に破棄する(自分のハンドラの中から、自分を破棄したいときに使う)。
    /// それまでに破棄されたら何もしない。
    void delete_later();

protected:
    /// 所有しているオブジェクトをすべて破棄する。派生クラスのデストラクタが、自分がまだその型であるうちに呼べる
    /// (呼ばなければ Object のデストラクタが破棄する)。
    void delete_owned();

    /// 所有者を new_owner に移す(Widget の親の付け替えに使う。docs/adr/0018)。
    void change_owner(Object* new_owner);

    /// プロパティの値が変わったことを知らせる(docs/adr/0016)。値が変わったときだけ呼ぶ。
    /// 書き込みの関数を自分で書くとき・利用者の操作など書き込みの関数を通らずに値が変わったときに呼ぶ。
    template <typename T>
    void notify_changed(const Property<T>& property)
    {
        detail::property_changed(this, &property);
    }

    template <typename T>
    void notify_changed(const ReadOnlyProperty<T>& property)
    {
        detail::property_changed(this, &property);
    }

private:
    template <typename T>
    friend class WeakRef;

    static Object* get_owner(Object* self) { return self->owner_; }

    Object* owner_;
    std::vector<Object*> owned_;
    /// WeakRef が作られたときだけ持つ。破棄するときに alive を false にする。
    std::shared_ptr<detail::Liveness> liveness_;
};

/// std::string のプロパティを std::string の演算子と直接組み合わせる
/// (std::string の演算子はテンプレートなので、プロキシからの暗黙の変換が効かないため)。
inline std::string operator+(const Property<std::string>& a, const std::string& b) { return a.get() + b; }
inline std::string operator+(const std::string& a, const Property<std::string>& b) { return a + b.get(); }
inline std::string operator+(const Property<std::string>& a, const char* b) { return a.get() + b; }
inline std::string operator+(const char* a, const Property<std::string>& b) { return a + b.get(); }
inline std::string operator+(const ReadOnlyProperty<std::string>& a, const std::string& b) { return a.get() + b; }
inline std::string operator+(const std::string& a, const ReadOnlyProperty<std::string>& b) { return a + b.get(); }
inline std::string operator+(const ReadOnlyProperty<std::string>& a, const char* b) { return a.get() + b; }
inline std::string operator+(const char* a, const ReadOnlyProperty<std::string>& b) { return a + b.get(); }
inline bool operator==(const Property<std::string>& a, const std::string& b) { return a.get() == b; }
inline bool operator==(const std::string& a, const Property<std::string>& b) { return a == b.get(); }
inline bool operator==(const Property<std::string>& a, const char* b) { return a.get() == b; }
inline bool operator==(const char* a, const Property<std::string>& b) { return a == b.get(); }
inline bool operator!=(const Property<std::string>& a, const std::string& b) { return a.get() != b; }
inline bool operator!=(const std::string& a, const Property<std::string>& b) { return a != b.get(); }
inline bool operator!=(const Property<std::string>& a, const char* b) { return a.get() != b; }
inline bool operator!=(const char* a, const Property<std::string>& b) { return a != b.get(); }
inline bool operator==(const ReadOnlyProperty<std::string>& a, const std::string& b) { return a.get() == b; }
inline bool operator==(const std::string& a, const ReadOnlyProperty<std::string>& b) { return a == b.get(); }
inline bool operator==(const ReadOnlyProperty<std::string>& a, const char* b) { return a.get() == b; }
inline bool operator==(const char* a, const ReadOnlyProperty<std::string>& b) { return a == b.get(); }
inline bool operator!=(const ReadOnlyProperty<std::string>& a, const std::string& b) { return a.get() != b; }
inline bool operator!=(const std::string& a, const ReadOnlyProperty<std::string>& b) { return a != b.get(); }
inline bool operator!=(const ReadOnlyProperty<std::string>& a, const char* b) { return a.get() != b; }
inline bool operator!=(const char* a, const ReadOnlyProperty<std::string>& b) { return a != b.get(); }

}  // namespace tokotachi

#endif
