#include <tokotachi/core/timer.hpp>

#include <tokotachi/core/application.hpp>

#include "application_impl.hpp"
#include "impl.hpp"

namespace tokotachi
{

class Timer::Impl
{
    TOKOTACHI_IMPL(Timer);

    explicit Impl(Timer* timer) : q(timer), interval_(1000), repeat_(true), enabled_(false), id_(0) {}

    void set_interval(const int& v)
    {
        interval_ = v;
        if (enabled_)
        {
            stop();
            start();
        }
    }

    void set_repeat(const bool& v)
    {
        repeat_ = v;
        if (enabled_)
        {
            stop();
            start();
        }
    }

    void set_enabled(const bool& v)
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

    void start()
    {
        Application* app = Application::instance();
        if (!app)
        {
            enabled_ = false;  // 代入した側(call_impl_setter_and_notify)が、enabled が設定されたことを知らせる
            return;
        }
        id_ = detail::impl_of(app)->start_timer(interval_, [this]() { fired(); }, repeat_);
    }

    void stop()
    {
        Application* app = Application::instance();
        if (id_ && app)
        {
            detail::impl_of(app)->stop_timer(id_);
        }
        id_ = 0;
    }

    void fired()
    {
        if (!repeat_)
        {
            // 1 回だけのタイマーは、内部のタイマーがすでに消えている。
            id_ = 0;
            enabled_ = false;
            q->notify_changed(q->enabled);
        }
        // ハンドラの中でこのタイマーが破棄されてもよいよう、最後に呼ぶ。
        q->timeout();
    }

    int interval_;
    bool repeat_;
    bool enabled_;
    NotifyHandler on_timeout_;
    int id_;  ///< Application の内部のタイマーの番号(止まっていれば 0)
};

Timer::Timer(Object* owner_object)
    : Object(owner_object),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, int, interval),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, repeat),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, enabled),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_timeout),
      d_(new Impl(this))
{
}

Timer::~Timer()
{
    d_->stop();
    delete d_;
}

void Timer::timeout()
{
    Object* self = this;
    detail::fire(d_->on_timeout_, self);
}

}  // namespace tokotachi
