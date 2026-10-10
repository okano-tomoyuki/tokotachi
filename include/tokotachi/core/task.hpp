/// @file
/// 別のスレッドで行う、終わりのある仕事(docs/adr/0029)。結果・進み具合・取り消し・例外をメインスレッドで受け取る。
///
/// @code
/// tokotachi::Task<std::size_t>* count = new tokotachi::Task<std::size_t>(&window);   // window が破棄されたら取り消し、結果は捨てる
/// count->on_progress = [=](tokotachi::Object*) { progress_bar_count->value = count->progress; };
/// count->on_finish = [=](tokotachi::Object*)
/// {
///     if (!count->error.get().empty()) { ... }       // 仕事が例外を投げた
///     else if (count->cancelled) { ... }             // 取り消した
///     else { label_result->text = std::to_string(count->result.get()); }
/// };
/// count->start([](tokotachi::TaskContext& ctx) -> std::size_t
/// {
///     std::size_t n = 0;
///     for (int i = 0; i < 100 && !ctx.cancel_requested(); ++i)   // 別のスレッドで動く。ウィジェットには触らない
///     {
///         n += heavy_work(i);
///         ctx.report_progress(i + 1);
///     }
///     return n;
/// });
/// count->cancel();   // 取り消しを頼む(仕事が cancel_requested を見て戻るまで待たない)
/// @endcode
///
/// on_progress・on_finish はメインスレッドで、イベントの処理の中で呼ぶ。結果の型 T はコピーと既定の構築ができること。
#ifndef TOKOTACHI_CORE_TASK_HPP
#define TOKOTACHI_CORE_TASK_HPP

#include <functional>
#include <memory>
#include <string>

#include "event.hpp"
#include "property.hpp"

namespace tokotachi
{

namespace detail
{
struct TaskShared;
}

/// 仕事の関数に渡す。どのスレッドから呼んでもよい。
class TaskContext
{
public:
    /// cancel が頼まれたか(仕事はなるべく早く戻る)。
    bool cancel_requested() const;
    /// 進み具合を知らせる(値の範囲は利用者が決める)。頻繁に呼んでよい(メインスレッドへは間引いて届ける)。
    void report_progress(double value);

    explicit TaskContext(const std::shared_ptr<detail::TaskShared>& shared) : shared_(shared) {}

private:
    std::shared_ptr<detail::TaskShared> shared_;
};

/// Task の型によらない部分。
class TaskBase : public Object
{
public:
    /// 破棄すると取り消しを頼み、仕事の結果は捨てる(仕事の関数が戻るまで待たない)。
    ~TaskBase();

    /// 動いているか(start から on_finish の直前まで)。
    ReadOnlyProperty<bool> running;
    /// 最後に知らせた進み具合(start で 0 に戻す)。
    ReadOnlyProperty<double> progress;
    /// 取り消しを頼んだ後に終わったか。
    ReadOnlyProperty<bool> cancelled;
    /// 仕事の関数が投げた例外のメッセージ(投げなければ空)。
    ReadOnlyProperty<std::string> error;

    Property<NotifyHandler> on_progress;
    /// 終わった(結果が出た・取り消した・例外を投げた)。
    Property<NotifyHandler> on_finish;

    /// 取り消しを頼む。動いていなければ何もしない。
    void cancel();

protected:
    explicit TaskBase(Object* owner);

    /// body を別のスレッドで呼ぶ。body が例外を投げずに戻ったら、メインスレッドで deliver を呼んでから on_finish を呼ぶ。
    /// 動いていれば何もせず false。
    bool launch(const std::function<void(TaskContext&)>& body, const std::function<void()>& deliver);

private:
    friend struct TaskAccess;

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

template <typename T>
class Task : public TaskBase
{
public:
    explicit Task(Object* owner = nullptr) : TaskBase(owner), result(this, &Task::read_result), result_() {}

    /// 仕事の結果(on_finish で、error が空で cancelled でなければ読む)。
    ReadOnlyProperty<T> result;

    /// work を別のスレッドで始める。動いていれば何もせず false。
    bool start(const std::function<T(TaskContext&)>& work)
    {
        std::shared_ptr<T> value = std::make_shared<T>();
        return launch([work, value](TaskContext& ctx) { *value = work(ctx); },
                      [this, value]()
                      {
                          result_ = *value;
                          notify_changed(result);
                      });
    }

private:
    static T read_result(Object* self) { return static_cast<Task*>(self)->result_; }

    T result_;
};

template <>
class Task<void> : public TaskBase
{
public:
    explicit Task(Object* owner = nullptr) : TaskBase(owner) {}

    bool start(const std::function<void(TaskContext&)>& work) { return launch(work, std::function<void()>()); }
};

}  // namespace tokotachi

#endif
