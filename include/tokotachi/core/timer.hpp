/// @file
/// tokotachi::Timer の宣言。
#ifndef TOKOTACHI_CORE_TIMER_HPP
#define TOKOTACHI_CORE_TIMER_HPP

#include "event.hpp"
#include "export.hpp"
#include "property.hpp"

namespace tokotachi
{

/// タイマー: 一定の間隔で on_timeout をメインスレッドで呼ぶ(docs/adr/0014)。
///
/// @code
/// tokotachi::Timer* timer_clock = new tokotachi::Timer(window_main);   // window_main が所有する
/// timer_clock->interval = 500;                                          // ミリ秒
/// timer_clock->on_timeout = [=](tokotachi::Object* sender) { ... };
/// timer_clock->enabled = true;                                          // 動かす(false で止める)
/// @endcode
///
/// 所有者を渡すと所有者と一緒に破棄される(docs/adr/0015)。所有者が無ければ利用者が破棄する(docs/adr/0062)。
/// 破棄すると止まる。Application を作った後に enabled にする。
///
/// @ingroup core_objects
class TOKOTACHI_API Timer : public Object
{
public:
    explicit Timer(Object* owner_object = nullptr);
    ~Timer();

    /// 間隔(ミリ秒、既定 1000)。動いている間に変えると、その時点から数え直す。
    Property<int> interval;
    /// 繰り返すか(既定 true)。false なら 1 回呼んだ後に enabled が false になる。
    Property<bool> repeat;
    /// 動いているか(既定 false)。true を代入すると動き始め、false を代入すると止まる。
    Property<bool> enabled;

    Property<NotifyHandler> on_timeout;

protected:
    /// 間隔が経った。on_timeout を呼ぶ。
    virtual void timeout();

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
