// タイマー: 一定の間隔で on_timer をメインスレッドで呼ぶ(docs/adr/0014)。
//
//   gui::Timer timer;
//   timer.interval = 500;                          // ミリ秒
//   timer.on_timer = [&](gui::Object* sender) { ... };
//   timer.enabled = true;                          // 動かす(false で止める)
//
// 所有者を渡すと所有者と一緒に破棄される(new gui::Timer(&window)。docs/adr/0015)。所有者が無ければ利用者が寿命を
// 管理する(スタック・メンバ変数にも置ける)。破棄すると止まる。Application を作った後に enabled にする。
#ifndef GUI_TIMER_HPP
#define GUI_TIMER_HPP

#include "event.hpp"
#include "property.hpp"

namespace gui
{

class Timer : public Object
{
public:
    explicit Timer(Object* owner_object = nullptr);
    ~Timer();

    // 間隔(ミリ秒、既定 1000)。動いている間に変えると、その時点から数え直す。
    Property<int> interval;
    // 繰り返すか(既定 true)。false なら 1 回呼んだ後に enabled が false になる。
    Property<bool> repeat;
    // 動いているか(既定 false)。true を代入すると動き始め、false を代入すると止まる。
    Property<bool> enabled;

    Property<NotifyHandler> on_timer;

protected:
    // 間隔が経った。on_timer を呼ぶ。
    virtual void timeout();

private:
    void set_interval(const int& v);
    void set_repeat(const bool& v);
    void set_enabled(const bool& v);
    void start();
    void stop();
    void fired();

    int interval_;
    bool repeat_;
    bool enabled_;
    NotifyHandler on_timer_;
    int id_;  // Application の内部のタイマーの番号(止まっていれば 0)
};

}  // namespace gui

#endif
