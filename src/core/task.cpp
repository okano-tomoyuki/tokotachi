#include <tokotachi/core/task.hpp>

#include <atomic>
#include <exception>
#include <mutex>
#include <system_error>
#include <thread>

#include <tokotachi/core/thread.hpp>

#include "impl.hpp"

namespace tokotachi
{

namespace detail
{

// 1 回の実行の、スレッドの間で共有する状態。
struct TaskShared
{
    std::atomic<bool> cancel;
    std::mutex mutex;
    double progress;        // 最後に知らせた値(mutex で守る)
    bool progress_pending;  // メインスレッドへ届けている途中か(mutex で守る)
    WeakRef<TaskBase> owner;

    TaskShared() : cancel(false), progress(0), progress_pending(false) {}
};

}  // namespace detail

class TaskBase::Impl
{
    TOKOTACHI_IMPL(TaskBase);

    explicit Impl(TaskBase* task) : q(task), progress_(0), cancelled_(false) {}

    bool get_running() const { return shared_ != nullptr; }
    double get_progress() const { return progress_; }
    bool get_cancelled() const { return cancelled_; }
    std::string get_error() const { return error_; }

    /// メインスレッドで呼ぶ。
    void progress_arrived(const std::shared_ptr<detail::TaskShared>& run);
    void finished(const std::shared_ptr<detail::TaskShared>& run, const std::function<void()>& deliver,
                  const std::string& failure);

    std::shared_ptr<detail::TaskShared> shared_;  ///< 動いている実行(無ければ nullptr)
    double progress_;
    bool cancelled_;
    std::string error_;
    NotifyHandler on_progress_;
    NotifyHandler on_finish_;
};

bool TaskContext::cancel_requested() const { return shared_->cancel.load(); }

void TaskContext::report_progress(double value)
{
    bool need_post = false;
    {
        std::lock_guard<std::mutex> lock(shared_->mutex);
        shared_->progress = value;
        if (!shared_->progress_pending)
        {
            shared_->progress_pending = true;
            need_post = true;
        }
    }
    if (need_post)
    {
        // 届くまでに知らせた値は、届いたときに最後の値だけを読む(間引く)。
        const std::shared_ptr<detail::TaskShared> run = shared_;
        post(run->owner, [run](TaskBase* t) { detail::impl_of(t)->progress_arrived(run); });
    }
}

TaskBase::TaskBase(Object* owner_object)
    : Object(owner_object),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, bool, running),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, double, progress),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, bool, cancelled),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::string, error),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_progress),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_finish),
      d_(new Impl(this))
{
}

TaskBase::~TaskBase()
{
    // 結果は、この Object が破棄されたので届けない(WeakRef が止める)。仕事には戻るよう頼む。
    if (d_->shared_)
    {
        d_->shared_->cancel = true;
    }
    delete d_;
}

void TaskBase::cancel()
{
    if (d_->shared_)
    {
        d_->shared_->cancel = true;
    }
}

bool TaskBase::launch(const std::function<void(TaskContext&)>& body, const std::function<void()>& deliver)
{
    if (d_->shared_ || !body)
    {
        return false;
    }
    std::shared_ptr<detail::TaskShared> run = std::make_shared<detail::TaskShared>();
    run->owner = WeakRef<TaskBase>(this);
    try
    {
        std::thread(
            [run, body, deliver]()
            {
                std::string failure;
                TaskContext ctx(run);
                try
                {
                    body(ctx);
                }
                catch (const std::exception& e)
                {
                    failure = e.what();
                    if (failure.empty())
                    {
                        failure = "exception";
                    }
                }
                catch (...)
                {
                    failure = "unknown exception";
                }
                post(run->owner, [run, deliver, failure](TaskBase* t) { detail::impl_of(t)->finished(run, deliver, failure); });
            })
            .detach();
    }
    catch (const std::system_error&)
    {
        return false;  // スレッドを作れなかった
    }
    d_->shared_ = run;
    const bool had_progress = d_->progress_ != 0;
    const bool had_cancelled = d_->cancelled_;
    const bool had_error = !d_->error_.empty();
    d_->progress_ = 0;
    d_->cancelled_ = false;
    d_->error_.clear();
    notify_changed(running);
    if (had_progress)
    {
        notify_changed(progress);
    }
    if (had_cancelled)
    {
        notify_changed(cancelled);
    }
    if (had_error)
    {
        notify_changed(error);
    }
    return true;
}

void TaskBase::Impl::progress_arrived(const std::shared_ptr<detail::TaskShared>& run)
{
    if (run != shared_)
    {
        return;  // 前の実行のもの・終わった後に届いたもの
    }
    double value = 0;
    {
        std::lock_guard<std::mutex> lock(run->mutex);
        value = run->progress;
        run->progress_pending = false;
    }
    if (value == progress_)
    {
        return;
    }
    progress_ = value;
    q->notify_changed(q->progress);
    Object* self = q;
    detail::fire(on_progress_, self);
}

void TaskBase::Impl::finished(const std::shared_ptr<detail::TaskShared>& run, const std::function<void()>& deliver,
                              const std::string& failure)
{
    if (run != shared_)
    {
        return;
    }
    // 最後に知らせた進み具合が届いていなければ、ここで反映する。
    double last = 0;
    {
        std::lock_guard<std::mutex> lock(run->mutex);
        last = run->progress;
        run->progress_pending = false;
    }
    const bool progress_changed = last != progress_;
    progress_ = last;
    shared_.reset();
    cancelled_ = run->cancel.load();
    error_ = failure;
    if (failure.empty() && deliver)
    {
        deliver();
    }
    if (progress_changed)
    {
        q->notify_changed(q->progress);
    }
    q->notify_changed(q->running);
    if (cancelled_)
    {
        q->notify_changed(q->cancelled);
    }
    if (!error_.empty())
    {
        q->notify_changed(q->error);
    }
    // ハンドラの中でこの Task が破棄されてもよいよう、最後に呼ぶ。
    Object* self = q;
    detail::fire(on_finish_, self);
}

}  // namespace tokotachi
