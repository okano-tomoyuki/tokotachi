#include <gui/timer.hpp>

#include <gui/application.hpp>

#include "application_impl.hpp"
#include "property_impl.hpp"

namespace gui
{

Timer::Timer(Object* owner_object)
    : Object(owner_object),
      GUI_PROPERTY_MEMBER(Timer, int, interval),
      GUI_PROPERTY_MEMBER(Timer, bool, repeat),
      GUI_PROPERTY_MEMBER(Timer, bool, enabled),
      GUI_HANDLER_PROPERTY(Timer, NotifyHandler, on_timer),
      interval_(1000),
      repeat_(true),
      enabled_(false),
      id_(0)
{
}

Timer::~Timer() { stop(); }

void Timer::set_interval(const int& v)
{
    interval_ = v;
    if (enabled_)
    {
        stop();
        start();
    }
}

void Timer::set_repeat(const bool& v)
{
    repeat_ = v;
    if (enabled_)
    {
        stop();
        start();
    }
}

void Timer::set_enabled(const bool& v)
{
    if (v == enabled_)
    {
        return;
    }
    enabled_ = v;
    if (v)
    {
        start();
    }
    else
    {
        stop();
    }
}

void Timer::start()
{
    Application* app = Application::instance();
    if (!app)
    {
        enabled_ = false;  // 代入した側(call_setter_and_notify)が、enabled が設定されたことを知らせる
        return;
    }
    id_ = app->impl()->start_timer(interval_, [this]() { fired(); }, repeat_);
}

void Timer::stop()
{
    Application* app = Application::instance();
    if (id_ && app)
    {
        app->impl()->stop_timer(id_);
    }
    id_ = 0;
}

void Timer::fired()
{
    if (!repeat_)
    {
        // 1 回だけのタイマーは、内部のタイマーがすでに消えている。
        id_ = 0;
        enabled_ = false;
        notify_changed(enabled);
    }
    // ハンドラの中でこのタイマーが破棄されてもよいよう、最後に呼ぶ。
    timeout();
}

void Timer::timeout()
{
    Object* self = this;
    detail::fire(on_timer_, self);
}

}  // namespace gui
