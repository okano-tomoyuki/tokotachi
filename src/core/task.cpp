#include <gui/core/task.hpp>

#include <atomic>
#include <exception>
#include <mutex>
#include <system_error>
#include <thread>

#include <gui/core/thread.hpp>

#include "property_impl.hpp"

namespace gui
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

struct TaskAccess
{
    static void progress_arrived(TaskBase* t, const std::shared_ptr<detail::TaskShared>& run)
    {
        t->progress_arrived(run);
    }

    static void finished(TaskBase* t, const std::shared_ptr<detail::TaskShared>& run,
                         const std::function<void()>& deliver, const std::string& failure)
    {
        t->finished(run, deliver, failure);
    }
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
        post(run->owner, [run](TaskBase* t) { TaskAccess::progress_arrived(t, run); });
    }
}

TaskBase::TaskBase(Object* owner_object)
    : Object(owner_object),
      GUI_READONLY_PROPERTY(TaskBase, bool, running),
      GUI_READONLY_PROPERTY(TaskBase, double, progress),
      GUI_READONLY_PROPERTY(TaskBase, bool, cancelled),
      GUI_READONLY_PROPERTY(TaskBase, std::string, error),
      GUI_HANDLER_PROPERTY(TaskBase, NotifyHandler, on_progress),
      GUI_HANDLER_PROPERTY(TaskBase, NotifyHandler, on_finish),
      progress_(0),
      cancelled_(false)
{
}

TaskBase::~TaskBase()
{
    // 結果は、この Object が破棄されたので届けない(WeakRef が止める)。仕事には戻るよう頼む。
    if (shared_)
    {
        shared_->cancel = true;
    }
}

void TaskBase::cancel()
{
    if (shared_)
    {
        shared_->cancel = true;
    }
}

bool TaskBase::launch(const std::function<void(TaskContext&)>& body, const std::function<void()>& deliver)
{
    if (shared_ || !body)
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
                post(run->owner, [run, deliver, failure](TaskBase* t) { TaskAccess::finished(t, run, deliver, failure); });
            })
            .detach();
    }
    catch (const std::system_error&)
    {
        return false;  // スレッドを作れなかった
    }
    shared_ = run;
    const bool had_progress = progress_ != 0;
    const bool had_cancelled = cancelled_;
    const bool had_error = !error_.empty();
    progress_ = 0;
    cancelled_ = false;
    error_.clear();
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

void TaskBase::progress_arrived(const std::shared_ptr<detail::TaskShared>& run)
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
    notify_changed(progress);
    Object* self = this;
    detail::fire(on_progress_, self);
}

void TaskBase::finished(const std::shared_ptr<detail::TaskShared>& run, const std::function<void()>& deliver,
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
        notify_changed(progress);
    }
    notify_changed(running);
    if (cancelled_)
    {
        notify_changed(cancelled);
    }
    if (!error_.empty())
    {
        notify_changed(error);
    }
    // ハンドラの中でこの Task が破棄されてもよいよう、最後に呼ぶ。
    Object* self = this;
    detail::fire(on_finish_, self);
}

}  // namespace gui
