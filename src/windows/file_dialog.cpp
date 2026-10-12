#include <tokotachi/windows/file_dialog.hpp>

#include <SDL3/SDL.h>

#include <map>

#include <tokotachi/windows/window.hpp>

#include "../core/application_impl.hpp"
#include "../core/impl.hpp"
#include "window_impl.hpp"

namespace tokotachi
{

class FileDialog::Impl
{
    TOKOTACHI_IMPL(FileDialog);

    explicit Impl(FileDialog* object) : q(object) {}

    std::vector<std::string> get_paths() const { return paths_; }
    int get_selected_filter() const { return selected_filter_; }
    std::string get_error() const { return error_; }
    bool get_running() const { return request_ != 0; }
    void set_mode(const FileDialogMode& v) { mode_ = v; }
    void set_title(const std::string& v) { title_ = v; }
    void set_filters(const std::vector<FileFilter>& v) { filters_ = v; }
    void set_location(const std::string& v) { location_ = v; }
    void set_multi_enabled(const bool& v) { multi_enabled_ = v; }
    /// 結果が届いた(メインスレッド)。
    void finished(const std::vector<std::string>& result, int filter, const std::string& failure);


    Window* parent_;
    FileDialogMode mode_;
    std::string title_;
    std::vector<FileFilter> filters_;
    std::string location_;
    bool multi_enabled_;
    std::vector<std::string> paths_;
    int selected_filter_;
    std::string error_;
    NotifyHandler on_finish_;
    int request_;  ///< 開いている要求の番号(0 なら開いていない)
};

namespace
{

// 開いている要求(番号から FileDialog へ)。メインスレッドだけで触る。
std::map<int, FileDialog*>& live_requests()
{
    static std::map<int, FileDialog*> requests;
    return requests;
}

int g_next_request = 0;

// SDL に渡す、コールバックが呼ばれるまで残すデータ。
struct Request
{
    int id;
    std::vector<std::string> names;
    std::vector<std::string> patterns;
    std::vector<SDL_DialogFileFilter> filters;
    SDL_PropertiesID props;
};

// OS によっては別のスレッドで呼ばれる。結果を写し、メインスレッドへ送る。
void SDLCALL dialog_callback(void* userdata, const char* const* filelist, int filter)
{
    Request* request = static_cast<Request*>(userdata);
    const int id = request->id;
    std::vector<std::string> result;
    std::string failure;
    if (!filelist)
    {
        // SDL_GetError はスレッドごとなので、失敗を知らせたこのスレッドで読む。
        const char* message = SDL_GetError();
        failure = message && *message ? message : "file dialog failed";
    }
    else
    {
        for (const char* const* p = filelist; *p; ++p)
        {
            result.push_back(*p);
        }
    }
    SDL_DestroyProperties(request->props);
    delete request;
    detail::ApplicationImpl::post(
        [id, result, filter, failure]()
        {
            std::map<int, FileDialog*>& live = live_requests();
            const std::map<int, FileDialog*>::iterator it = live.find(id);
            if (it == live.end())
            {
                return;  // 待っている間に破棄された
            }
            FileDialog* d = it->second;
            live.erase(it);
            detail::impl_of(d)->finished(result, filter, failure);
        });
}

}  // namespace

FileDialog::FileDialog(Window* parent, FileDialogMode initial_mode)
    : Object(parent),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, FileDialogMode, mode),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, title),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::vector<FileFilter>, filters),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, location),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, multi_enabled),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::vector<std::string>, paths),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::string, error),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, int, selected_filter),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, bool, running),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_finish),
      d_(new Impl(this))
{
    d_->parent_ = parent;
    d_->mode_ = initial_mode;
    d_->multi_enabled_ = false;
    d_->selected_filter_ = -1;
    d_->request_ = 0;
}

FileDialog::~FileDialog()
{
    if (d_->request_ != 0)
    {
        live_requests().erase(d_->request_);  // 結果が届いても捨てる
    }
    delete d_;
}

void FileDialog::open()
{
    if (d_->request_ != 0)
    {
        return;
    }
    Request* request = new Request();
    request->id = ++g_next_request;
    for (std::size_t i = 0; i < d_->filters_.size(); ++i)
    {
        request->names.push_back(d_->filters_[i].name);
        request->patterns.push_back(d_->filters_[i].pattern);
    }
    for (std::size_t i = 0; i < d_->filters_.size(); ++i)
    {
        SDL_DialogFileFilter f;
        f.name = request->names[i].c_str();
        f.pattern = request->patterns[i].c_str();
        request->filters.push_back(f);
    }
    request->props = SDL_CreateProperties();
    const SDL_PropertiesID props = request->props;
    if (!request->filters.empty() && d_->mode_ != FileDialogMode::FOLDER)
    {
        SDL_SetPointerProperty(props, SDL_PROP_FILE_DIALOG_FILTERS_POINTER, &request->filters[0]);
        SDL_SetNumberProperty(props, SDL_PROP_FILE_DIALOG_NFILTERS_NUMBER, static_cast<Sint64>(request->filters.size()));
    }
    if (d_->parent_ && detail::window_impl_of(d_->parent_) && detail::window_impl_of(d_->parent_)->window)
    {
        SDL_SetPointerProperty(props, SDL_PROP_FILE_DIALOG_WINDOW_POINTER, detail::window_impl_of(d_->parent_)->window);
    }
    if (!d_->location_.empty())
    {
        SDL_SetStringProperty(props, SDL_PROP_FILE_DIALOG_LOCATION_STRING, d_->location_.c_str());
    }
    if (!d_->title_.empty())
    {
        SDL_SetStringProperty(props, SDL_PROP_FILE_DIALOG_TITLE_STRING, d_->title_.c_str());
    }
    SDL_SetBooleanProperty(props, SDL_PROP_FILE_DIALOG_MANY_BOOLEAN, d_->multi_enabled_ && d_->mode_ != FileDialogMode::SAVE);

    d_->request_ = request->id;
    live_requests()[d_->request_] = this;
    d_->paths_.clear();
    d_->selected_filter_ = -1;
    d_->error_.clear();
    SDL_ClearError();
    notify_changed(running);
    const SDL_FileDialogType type = d_->mode_ == FileDialogMode::SAVE     ? SDL_FILEDIALOG_SAVEFILE
                                    : d_->mode_ == FileDialogMode::FOLDER ? SDL_FILEDIALOG_OPENFOLDER
                                                                      : SDL_FILEDIALOG_OPENFILE;
    // コールバックは、この中で(失敗したときなど)呼ばれることもある。結果はメインスレッドへ送るので、後で届く。
    SDL_ShowFileDialogWithProperties(type, &dialog_callback, request, props);
}

void FileDialog::Impl::finished(const std::vector<std::string>& result, int filter, const std::string& failure)
{
    request_ = 0;
    paths_ = result;
    selected_filter_ = filter;
    error_ = failure;
    q->notify_changed(q->paths);
    q->notify_changed(q->error);
    q->notify_changed(q->selected_filter);
    q->notify_changed(q->running);
    Object* self = q;
    detail::fire(on_finish_, self);
}

}  // namespace tokotachi
