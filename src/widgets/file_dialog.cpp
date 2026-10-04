#include <gui/file_dialog.hpp>

#include <SDL3/SDL.h>

#include <map>

#include <gui/window.hpp>

#include "../core/application_impl.hpp"
#include "../core/property_impl.hpp"
#include "../core/window_impl.hpp"

namespace gui
{

struct FileDialogAccess
{
    static void finished(FileDialog* d, const std::vector<std::string>& result, int filter, const std::string& failure)
    {
        d->finished(result, filter, failure);
    }
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
            FileDialogAccess::finished(d, result, filter, failure);
        });
}

}  // namespace

FileDialog::FileDialog(Window* parent, FileDialogMode initial_mode)
    : Object(parent),
      GUI_PROPERTY_MEMBER(FileDialog, FileDialogMode, mode),
      GUI_PROPERTY_MEMBER(FileDialog, std::string, title),
      GUI_PROPERTY_MEMBER(FileDialog, std::vector<FileFilter>, filters),
      GUI_PROPERTY_MEMBER(FileDialog, std::string, location),
      GUI_PROPERTY_MEMBER(FileDialog, bool, allow_many),
      GUI_READONLY_PROPERTY(FileDialog, std::vector<std::string>, paths),
      GUI_READONLY_PROPERTY(FileDialog, std::string, error),
      GUI_READONLY_PROPERTY(FileDialog, int, selected_filter),
      GUI_READONLY_PROPERTY(FileDialog, bool, running),
      GUI_HANDLER_PROPERTY(FileDialog, NotifyHandler, on_finish),
      parent_(parent),
      mode_(initial_mode),
      allow_many_(false),
      selected_filter_(-1),
      request_(0)
{
}

FileDialog::~FileDialog()
{
    if (request_ != 0)
    {
        live_requests().erase(request_);  // 結果が届いても捨てる
    }
}

void FileDialog::open()
{
    if (request_ != 0)
    {
        return;
    }
    Request* request = new Request();
    request->id = ++g_next_request;
    for (std::size_t i = 0; i < filters_.size(); ++i)
    {
        request->names.push_back(filters_[i].name);
        request->patterns.push_back(filters_[i].pattern);
    }
    for (std::size_t i = 0; i < filters_.size(); ++i)
    {
        SDL_DialogFileFilter f;
        f.name = request->names[i].c_str();
        f.pattern = request->patterns[i].c_str();
        request->filters.push_back(f);
    }
    request->props = SDL_CreateProperties();
    const SDL_PropertiesID props = request->props;
    if (!request->filters.empty() && mode_ != FileDialogMode::FOLDER)
    {
        SDL_SetPointerProperty(props, SDL_PROP_FILE_DIALOG_FILTERS_POINTER, &request->filters[0]);
        SDL_SetNumberProperty(props, SDL_PROP_FILE_DIALOG_NFILTERS_NUMBER, static_cast<Sint64>(request->filters.size()));
    }
    if (parent_ && parent_->impl() && parent_->impl()->window)
    {
        SDL_SetPointerProperty(props, SDL_PROP_FILE_DIALOG_WINDOW_POINTER, parent_->impl()->window);
    }
    if (!location_.empty())
    {
        SDL_SetStringProperty(props, SDL_PROP_FILE_DIALOG_LOCATION_STRING, location_.c_str());
    }
    if (!title_.empty())
    {
        SDL_SetStringProperty(props, SDL_PROP_FILE_DIALOG_TITLE_STRING, title_.c_str());
    }
    SDL_SetBooleanProperty(props, SDL_PROP_FILE_DIALOG_MANY_BOOLEAN, allow_many_ && mode_ != FileDialogMode::SAVE);

    request_ = request->id;
    live_requests()[request_] = this;
    paths_.clear();
    selected_filter_ = -1;
    error_.clear();
    SDL_ClearError();
    notify_changed(running);
    const SDL_FileDialogType type = mode_ == FileDialogMode::SAVE     ? SDL_FILEDIALOG_SAVEFILE
                                    : mode_ == FileDialogMode::FOLDER ? SDL_FILEDIALOG_OPENFOLDER
                                                                      : SDL_FILEDIALOG_OPENFILE;
    // コールバックは、この中で(失敗したときなど)呼ばれることもある。結果はメインスレッドへ送るので、後で届く。
    SDL_ShowFileDialogWithProperties(type, &dialog_callback, request, props);
}

void FileDialog::finished(const std::vector<std::string>& result, int filter, const std::string& failure)
{
    request_ = 0;
    paths_ = result;
    selected_filter_ = filter;
    error_ = failure;
    notify_changed(paths);
    notify_changed(error);
    notify_changed(selected_filter);
    notify_changed(running);
    Object* self = this;
    detail::fire(on_finish_, self);
}

}  // namespace gui
