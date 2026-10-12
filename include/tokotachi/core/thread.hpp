/// @file
/// 別のスレッドからメインスレッドへ処理を頼む(docs/adr/0029)。
///
/// ウィジェット・Object・プロパティはメインスレッド(Application を作ったスレッド)からだけ触る。別のスレッドで
/// 重い処理をしたら、結果を画面に出す処理を post でメインスレッドへ頼む:
///
/// @code
/// tokotachi::WeakRef<tokotachi::Label> status(label_status);           // メインスレッドで作る
/// std::thread([status]()
/// {
///     const std::string result = compute();          // 別のスレッド
///     tokotachi::post(status, [result](tokotachi::Label* l) { l->text = result; });   // label_status が残っていれば呼ぶ
/// }).detach();
/// @endcode
///
/// 頼んだ処理は、イベントの処理の中で、頼んだ順に呼ぶ。終わるまで待つ呼び出しは用意しない(デッドロックを避けるため)。
#ifndef TOKOTACHI_CORE_THREAD_HPP
#define TOKOTACHI_CORE_THREAD_HPP

#include <functional>
#include <memory>

#include "export.hpp"
#include "property.hpp"

namespace tokotachi
{

/// 今のスレッドがメインスレッド(Application を作ったスレッド)か。
TOKOTACHI_API bool is_main_thread();

/// どのスレッドからでも呼べる: fn をメインスレッドで呼ぶよう頼む。Application が無い・終わった後なら頼まずに false を返す。
/// Application が終わるまでに呼ばれなかった fn は、呼ばずに捨てる。
TOKOTACHI_API bool post(const std::function<void()>& fn);

/// Object への弱い参照: 指す Object が破棄されたかを確かめられる。
/// 作るのと get はメインスレッドで行う。コピー・破棄はどのスレッドからでもよい(別のスレッドへ渡すために使う)。
/// @ingroup core_threads
template <typename T>
class WeakRef
{
public:
    WeakRef() : object_(nullptr) {}

    /// メインスレッドで作る。
    WeakRef(T* object) : object_(object)
    {
        if (object)
        {
            Object* o = object;
            if (!o->liveness_)
            {
                o->liveness_ = std::make_shared<detail::Liveness>();
            }
            liveness_ = o->liveness_;
        }
    }

    /// 指す Object(破棄されていれば nullptr)。メインスレッドで呼ぶ。
    T* get() const { return liveness_ && liveness_->alive ? object_ : nullptr; }
    bool expired() const { return get() == nullptr; }

private:
    T* object_;
    std::shared_ptr<detail::Liveness> liveness_;
};

/// どのスレッドからでも呼べる: context の Object がその時点で残っていれば、メインスレッドで fn(Object へのポインタ) を呼ぶ。
/// 破棄されていれば呼ばない。
template <typename T, typename F>
bool post(const WeakRef<T>& context, F fn)
{
    return post(std::function<void()>(
        [context, fn]()
        {
            if (T* object = context.get())
            {
                fn(object);
            }
        }));
}

}  // namespace tokotachi

#endif
