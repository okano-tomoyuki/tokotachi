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

#include "export.hpp"

namespace tokotachi
{

class Object;

namespace detail
{
struct PropertyAccess;
/// ライブラリの中で、ほかのクラスの実装(pimpl の Impl)に届くための窓口(内部。docs/adr/0057)。
template <typename C>
struct ImplAccess;
/// ライブラリの中だけで派生する基底のコンストラクタが取る印(内部。docs/adr/0058)。公開のヘッダでは不完全型なので、利用者は作れない。
struct InternalTag;
/// owner の property(Property・ReadOnlyProperty のアドレス)の値が変わったことを知らせる。
/// 結びつけた相手のプロパティへ値を伝える(docs/adr/0016)。
TOKOTACHI_API void property_changed(Object* owner, const void* property);

/// Object が生きているかの目印(WeakRef が共有する。docs/adr/0029)。メインスレッドだけで読み書きする。
struct TOKOTACHI_API Liveness
{
    bool alive;
    Liveness() : alive(true) {}
};
/// o の生きているかの目印(初めて求められたときに作る。WeakRef が使う)。メインスレッドで呼ぶ。
TOKOTACHI_API std::shared_ptr<Liveness> liveness_of(Object* o);
}  // namespace detail

template <typename T>
class WeakRef;

/// プロパティ(読み書きできる値。代入で設定し、変わると知らせる。@ref basics)。
///
/// @ingroup core_objects
template <typename T>
class Property
{
public:
    /// 値の型。
    typedef T ValueType;
    /// 値を読む関数(所有者を受け取る)。
    typedef T (*Getter)(Object*);
    /// 値を書く関数(所有者と新しい値を受け取る)。値が変わったら所有者が notify_changed で知らせる。
    typedef void (*Setter)(Object*, const T&);

    /// 所有者(ふつうは this)と、値を読む関数・書く関数から作る(@ref custom_widgets_guide)。
    Property(Object* owner, Getter getter, Setter setter) : owner_(owner), getter_(getter), setter_(setter) {}

    /// コピーの構築は所有者を取り違えるので禁止する。プロパティ同士の代入は値のコピーとして扱う(所有者は付け替えない)。
    Property(const Property&) = delete;
    /// other の値を代入する(所有者は付け替えない)。
    Property& operator=(const Property& other)
    {
        setter_(owner_, other.get());
        return *this;
    }

    /// 値を代入する(書く関数を呼ぶ)。
    Property& operator=(const T& value)
    {
        setter_(owner_, value);
        return *this;
    }

    /// 値を読む(暗黙の変換)。
    operator T() const { return getter_(owner_); }

    /// 値を取り出す(auto で受ける・値のメソッドを呼ぶ・テンプレートに渡すなど、暗黙の変換が効かない場面で使う)。
    T get() const { return getter_(owner_); }

    /// ポインタ型のプロパティで、メンバへ直接たどれるようにする(widget->parent->x)。
    T operator->() const { return getter_(owner_); }

    /// 今の値に演算した結果を代入する(読んでから書く)。
    template <typename U>
    Property& operator+=(const U& value)
    {
        T v = get();
        v += value;
        setter_(owner_, v);
        return *this;
    }

    /// 今の値に演算した結果を代入する(読んでから書く)。
    template <typename U>
    Property& operator-=(const U& value)
    {
        T v = get();
        v -= value;
        setter_(owner_, v);
        return *this;
    }

    /// 今の値に演算した結果を代入する(読んでから書く)。
    template <typename U>
    Property& operator*=(const U& value)
    {
        T v = get();
        v *= value;
        setter_(owner_, v);
        return *this;
    }

    /// 今の値に演算した結果を代入する(読んでから書く)。
    template <typename U>
    Property& operator/=(const U& value)
    {
        T v = get();
        v /= value;
        setter_(owner_, v);
        return *this;
    }

    /// 1 増やす(前置)。
    Property& operator++()
    {
        T v = get();
        ++v;
        setter_(owner_, v);
        return *this;
    }

    /// 1 減らす(前置)。
    Property& operator--()
    {
        T v = get();
        --v;
        setter_(owner_, v);
        return *this;
    }

    /// 1 増やし、増やす前の値を返す(後置)。
    T operator++(int)
    {
        T old = get();
        T v = old;
        ++v;
        setter_(owner_, v);
        return old;
    }

    /// 1 減らし、減らす前の値を返す(後置)。
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

/// 読むだけのプロパティ(部品の状態など。代入はできない)。
///
/// @ingroup core_objects
template <typename T>
class ReadOnlyProperty
{
public:
    /// 値の型。
    typedef T ValueType;
    /// 値を読む関数(所有者を受け取る)。
    typedef T (*Getter)(Object*);

    /// 所有者と、値を読む関数から作る。値が変わったら所有者が notify_changed で知らせる。
    ReadOnlyProperty(Object* owner, Getter getter) : owner_(owner), getter_(getter) {}

    ReadOnlyProperty(const ReadOnlyProperty&) = delete;
    ReadOnlyProperty& operator=(const ReadOnlyProperty&) = delete;

    /// 値を読む(暗黙の変換)。
    operator T() const { return getter_(owner_); }
    /// 値を取り出す(暗黙の変換が効かない場面で使う)。
    T get() const { return getter_(owner_); }
    /// ポインタ型のプロパティで、メンバへ直接たどれるようにする。
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
/// @ingroup core_objects
class TOKOTACHI_API Object
{
public:
    /// 所有者を渡すと、所有者が破棄されるときに一緒に破棄される(new で作る)。
    explicit Object(Object* owner_object = nullptr);
    virtual ~Object();

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;

    /// 所有者(無ければ nullptr)。部品では親(parent)と同じ。
    ReadOnlyProperty<Object*> owner;

    /// 今のイベントの処理が終わった後に破棄する(自分のハンドラの中から、自分を破棄したいときに使う)。
    /// それまでに破棄されたら何もしない。
    void delete_later();

protected:
    /// 所有しているオブジェクトをすべて破棄する。派生クラスのデストラクタが、自分がまだその型であるうちに呼べる
    /// (呼ばなければ Object のデストラクタが破棄する)。
    void delete_owned();

    /// プロパティの値が変わったことを知らせる(docs/adr/0016)。値が変わったときだけ呼ぶ。
    /// 書き込みの関数を自分で書くとき・利用者の操作など書き込みの関数を通らずに値が変わったときに呼ぶ。
    template <typename T>
    void notify_changed(const Property<T>& property)
    {
        detail::property_changed(this, &property);
    }

    /// プロパティの値が変わったことを知らせる(読むだけのプロパティ)。
    template <typename T>
    void notify_changed(const ReadOnlyProperty<T>& property)
    {
        detail::property_changed(this, &property);
    }

private:
    template <typename>
    friend struct detail::ImplAccess;
    class Impl;
    Impl* d_;
};

/// std::string のプロパティと文字列をつなぐ・比べる(std::string の演算子はテンプレートなので、プロキシからの暗黙の変換が効かないため)。
///@{
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
///@}

}  // namespace tokotachi

#endif
