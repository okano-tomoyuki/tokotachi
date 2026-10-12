#include <tokotachi/core/application.hpp>

#include <SDL3/SDL.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <mutex>

#include <tokotachi/core/thread.hpp>

#include "application_impl.hpp"
#include "locale.hpp"
#include "../windows/window_impl.hpp"

namespace tokotachi
{

namespace
{

Application* g_instance = nullptr;
Uint32 g_timer_event = 0;
Uint32 g_call_event = 0;  // tokotachi::post

// post を受け付けているか(Application が生きている間)。別のスレッドから読むので、mutex で守る。
std::mutex& post_mutex()
{
    static std::mutex m;
    return m;
}
bool g_accepting_posts = false;
std::atomic<SDL_ThreadID> g_main_thread(0);

// タイマーのスレッドで呼ばれる。イベントを積むだけにし、関数はメインスレッドで呼ぶ。
// userdata には 番号 * 2 + 繰り返すか を入れる(マップをこのスレッドから読まないため)。
Uint32 SDLCALL timer_callback(void* userdata, SDL_TimerID, Uint32 interval)
{
    const std::intptr_t v = reinterpret_cast<std::intptr_t>(userdata);
    SDL_Event ev;
    SDL_zero(ev);
    ev.type = g_timer_event;
    ev.user.code = static_cast<Sint32>(v / 2);
    SDL_PushEvent(&ev);
    return (v & 1) ? interval : 0;
}

// Windows のウィンドウの枠のドラッグ中は OS のモーダルループに入り、exec() のループに戻ってこない。
// その間も SDL_EVENT_WINDOW_EXPOSED(data1 == 1)はメインスレッドで届くので、ここで描く(docs/adr/0010)。
bool SDLCALL event_watch(void* userdata, SDL_Event* e)
{
    detail::ApplicationImpl* d = static_cast<detail::ApplicationImpl*>(userdata);
    if (e->type == SDL_EVENT_WINDOW_EXPOSED && e->window.data1 == 1)
    {
        if (detail::WindowImpl* w = d->find_window(SDL_GetWindowFromID(e->window.windowID)))
        {
            w->update_scale();
            w->sync_size();
            w->render();
        }
    }
    return true;
}

}  // namespace

using namespace detail;

WindowImpl* Application::Impl::find_window(SDL_Window* w) const
{
    if (!w)
    {
        return nullptr;
    }
    for (std::size_t i = 0; i < windows.size(); ++i)
    {
        if (windows[i]->window == w)
        {
            return windows[i];
        }
    }
    return nullptr;
}

void Application::Impl::quit_if_no_window()
{
    for (std::size_t i = 0; i < windows.size(); ++i)
    {
        if (windows[i]->shown && !windows[i]->popup)
        {
            return;
        }
    }
    running = false;
}

void Application::Impl::update_theme()
{
    const bool next_dark =
        color_scheme == ColorScheme::DARK || (color_scheme == ColorScheme::SYSTEM && system_dark);
    const Theme& next = next_dark ? dark_theme : light_theme;
    const bool dark_changed = next_dark != dark;
    if (!dark_changed && next == current_theme)
    {
        return;
    }
    dark = next_dark;
    current_theme = next;
    if (dark_changed)
    {
        property_changed(q, &q->dark);
    }
    property_changed(q, &q->theme);
    for (std::size_t i = 0; i < windows.size(); ++i)
    {
        windows[i]->theme_changed();
    }
    Object* sender = q;
    fire(on_theme_change_, sender);
}

void Application::Impl::close_menus()
{
    while (!open_menus.empty())
    {
        open_menus.front()->q->close();  // 開いた順の先頭を閉じると、それから開いたものも閉じる
    }
}

namespace detail
{

bool trace_enabled()
{
    static const int enabled = []()
    {
        const char* v = std::getenv("TOKOTACHI_TRACE");
        return v && v[0] == '1' ? 1 : 0;
    }();
    return enabled != 0;
}

bool is_wayland()
{
    const char* driver = SDL_GetCurrentVideoDriver();
    return driver && std::strcmp(driver, "wayland") == 0;
}

const char* trace_name(const WindowImpl* w)
{
    // 1 つの printf で 2 回呼ばれても上書きしないよう、2 つのバッファを交互に使う。
    static char bufs[2][32];
    static int next = 0;
    char* buf = bufs[next];
    next ^= 1;
    if (!w)
    {
        return "(none)";
    }
    if (!w->popup)
    {
        return "window";
    }
    std::snprintf(buf, sizeof(bufs[0]), "popup#%u", static_cast<unsigned>(w->window ? SDL_GetWindowID(w->window) : 0));
    return buf;
}

namespace
{

void trace_event(const SDL_Event& e, const WindowImpl* w, std::size_t open_menus)
{
    const char* name = nullptr;
    float x = 0, y = 0;
    int extra = 0;
    switch (e.type)
    {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        name = "button_down";
        x = e.button.x;
        y = e.button.y;
        extra = e.button.button;
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        name = "button_up";
        x = e.button.x;
        y = e.button.y;
        extra = e.button.button;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        name = "motion";
        x = e.motion.x;
        y = e.motion.y;
        break;
    case SDL_EVENT_KEY_DOWN:
        name = "key_down";
        extra = static_cast<int>(e.key.key);
        break;
    case SDL_EVENT_WINDOW_SHOWN:
        name = "shown";
        break;
    case SDL_EVENT_WINDOW_HIDDEN:
        name = "hidden";
        break;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        name = "close_requested";
        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        name = "focus_gained";
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        name = "focus_lost";
        break;
    case SDL_EVENT_WINDOW_MOUSE_ENTER:
        name = "mouse_enter";
        break;
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        name = "mouse_leave";
        break;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        name = "pixel_size";
        x = static_cast<float>(e.window.data1);
        y = static_cast<float>(e.window.data2);
        break;
    case SDL_EVENT_WINDOW_MOVED:
        name = "moved";
        x = static_cast<float>(e.window.data1);
        y = static_cast<float>(e.window.data2);
        break;
    default:
        return;
    }
    std::fprintf(stderr, "[tokotachi] %-15s %-10s (%.1f, %.1f) %d  open_menus=%u\n", name, trace_name(w), x, y, extra,
                 static_cast<unsigned>(open_menus));
}

}  // namespace

}  // namespace detail

void Application::Impl::post(const std::function<void()>& fn) { tokotachi::post(fn); }

void Application::Impl::flush_doomed()
{
    while (!doomed.empty())
    {
        Object* o = doomed.back();
        doomed.pop_back();
        delete o;  // 破棄の中で delete_later が呼ばれてもよいよう、1 つずつ
    }
}

WindowImpl* Application::Impl::modal_blocker(const WindowImpl* w) const
{
    // 後に開いたものから: w がダイアログの親(またはその祖先)なら止める。
    for (std::size_t i = modal_dialogs.size(); i-- > 0;)
    {
        for (const WindowImpl* p = modal_dialogs[i]->modal_parent; p; p = p->modal_parent)
        {
            if (p == w)
            {
                return modal_dialogs[i];
            }
        }
    }
    return nullptr;
}

void Application::Impl::dispatch(const SDL_Event& e)
{
    dispatch_event(e);
    flush_doomed();
}

void Application::Impl::dispatch_event(const SDL_Event& e)
{
    if (e.type == g_call_event)
    {
        std::function<void()>* fn = static_cast<std::function<void()>*>(e.user.data1);
        const std::function<void()> copy = *fn;
        delete fn;
        copy();
        return;
    }
    if (e.type == g_timer_event)
    {
        std::map<int, Timer>::iterator it = timers.find(e.user.code);
        if (it == timers.end())
        {
            return;  // 止めた後に届いたもの
        }
        const std::function<void()> fn = it->second.fn;
        if (!it->second.repeat)
        {
            timers.erase(it);
        }
        fn();
        return;
    }
    if (e.type == SDL_EVENT_SYSTEM_THEME_CHANGED)
    {
        system_dark = SDL_GetSystemTheme() == SDL_SYSTEM_THEME_DARK;
        update_theme();
        return;
    }
    if (e.type == SDL_EVENT_QUIT)
    {
        running = false;
        return;
    }
    WindowImpl* w = find_window(SDL_GetWindowFromEvent(&e));
    if (trace_enabled() && e.type != SDL_EVENT_MOUSE_MOTION)
    {
        trace_event(e, w, open_menus.size());
    }
    if (!w)
    {
        return;
    }
    // モーダルのダイアログが開いていれば、その親への入力は捨てる(押したらダイアログを前に出す)。
    if (WindowImpl* blocker = modal_blocker(w))
    {
        switch (e.type)
        {
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            SDL_RaiseWindow(blocker->window);
            return;
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_WHEEL:
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_TEXT_INPUT:
        case SDL_EVENT_TEXT_EDITING:
            return;
        default:
            break;
        }
    }
    if (!open_menus.empty())
    {
        // メニューの外側でボタンを押したら、メニューをすべて閉じ、押したことは捨てる(Windows・X11。Wayland はコンポジタが閉じる)。
        if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            std::find(open_menus.begin(), open_menus.end(), w) == open_menus.end())
        {
            close_menus();
            return;
        }
        // キー・文字の入力は、一番上のメニューへ送る(OS がどのウィンドウにフォーカスを与えたかによらないように)。
        if (e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP || e.type == SDL_EVENT_TEXT_INPUT ||
            e.type == SDL_EVENT_TEXT_EDITING)
        {
            open_menus.back()->handle(e);
            return;
        }
    }
    w->handle(e);
}

int Application::Impl::start_timer(int interval_ms, const std::function<void()>& fn, bool repeat)
{
    const int id = ++next_timer_id;
    Timer t;
    t.fn = fn;
    t.repeat = repeat;
    const std::intptr_t v = static_cast<std::intptr_t>(id) * 2 + (repeat ? 1 : 0);
    t.sdl_id = SDL_AddTimer(static_cast<Uint32>(interval_ms > 0 ? interval_ms : 1), &timer_callback,
                            reinterpret_cast<void*>(v));
    timers[id] = t;
    return id;
}

void Application::Impl::stop_timer(int id)
{
    std::map<int, Timer>::iterator it = timers.find(id);
    if (it == timers.end())
    {
        return;
    }
    SDL_RemoveTimer(it->second.sdl_id);
    timers.erase(it);
}

Application::Application(int, char**)
    : TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, ColorScheme, color_scheme),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, Theme, light_theme),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, Theme, dark_theme),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, bool, dark),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Theme, theme),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, std::string, language),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_theme_change),
      d_(new Impl(this))
{
    g_instance = this;
#if defined(__linux__)
    // Wayland を優先し、使えなければ X11 にする(docs/adr/0002)。環境変数 SDL_VIDEO_DRIVER の指定があればそちらを使う。
    if (!std::getenv("SDL_VIDEO_DRIVER"))
    {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "wayland,x11");
    }
#endif
    // IME の変換中の文字列はウィジェットが描く(docs/adr/0009)。
    SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI, "composition");
    // 最後のウィンドウを閉じたときの終了は自分で判断する(Window::close)。
    SDL_SetHint(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE, "0");
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        std::exit(1);
    }
    d_->language = detail::system_language();
    g_timer_event = SDL_RegisterEvents(1);
    g_call_event = SDL_RegisterEvents(1);
    g_main_thread = SDL_GetCurrentThreadID();
    {
        std::lock_guard<std::mutex> lock(post_mutex());
        g_accepting_posts = true;
    }
    SDL_AddEventWatch(&event_watch, d_);
    d_->system_dark = SDL_GetSystemTheme() == SDL_SYSTEM_THEME_DARK;
    d_->update_theme();
}

Application::~Application()
{
    for (std::map<int, Impl::Timer>::iterator it = d_->timers.begin(); it != d_->timers.end();
         ++it)
    {
        SDL_RemoveTimer(it->second.sdl_id);
    }
    // これからの post は受け付けず、届いていない依頼は呼ばずに捨てる。
    {
        std::lock_guard<std::mutex> lock(post_mutex());
        g_accepting_posts = false;
    }
    SDL_Event pending[64];
    int n = 0;
    while ((n = SDL_PeepEvents(pending, 64, SDL_GETEVENT, g_call_event, g_call_event)) > 0)
    {
        for (int i = 0; i < n; ++i)
        {
            delete static_cast<std::function<void()>*>(pending[i].user.data1);
        }
    }
    SDL_RemoveEventWatch(&event_watch, d_);
    delete d_;
    g_instance = nullptr;
    SDL_Quit();
}

Application* Application::instance() { return g_instance; }

void Application::Impl::set_color_scheme(const ColorScheme& v)
{
    if (v == color_scheme)
    {
        return;
    }
    color_scheme = v;
    detail::property_changed(q, &q->color_scheme);
    update_theme();
}

void Application::Impl::set_light_theme(const Theme& v)
{
    if (v == light_theme)
    {
        return;
    }
    light_theme = v;
    detail::property_changed(q, &q->light_theme);
    update_theme();
}

void Application::Impl::set_dark_theme(const Theme& v)
{
    if (v == dark_theme)
    {
        return;
    }
    dark_theme = v;
    detail::property_changed(q, &q->dark_theme);
    update_theme();
}

int Application::exec()
{
    // 待機型のループ: イベントが来るまで眠り、溜まったイベントを処理してから、無効になった領域を描く(docs/adr/0010)。
    d_->running = true;
    d_->quit_if_no_window();
    while (d_->running)
    {
        SDL_Event e;
        if (!SDL_WaitEvent(&e))
        {
            break;
        }
        d_->dispatch(e);
        while (d_->running && SDL_PollEvent(&e))
        {
            d_->dispatch(e);
        }
        for (std::size_t i = 0; i < d_->windows.size(); ++i)
        {
            d_->windows[i]->render();
        }
    }
    return d_->exit_code;
}

void Application::quit(int code)
{
    d_->exit_code = code;
    d_->running = false;
}

bool is_main_thread() { return g_main_thread != 0 && SDL_GetCurrentThreadID() == g_main_thread; }

bool post(const std::function<void()>& fn)
{
    if (!fn)
    {
        return false;
    }
    std::lock_guard<std::mutex> lock(post_mutex());
    if (!g_accepting_posts)
    {
        return false;
    }
    SDL_Event ev;
    SDL_zero(ev);
    ev.type = g_call_event;
    ev.user.data1 = new std::function<void()>(fn);
    if (!SDL_PushEvent(&ev))
    {
        delete static_cast<std::function<void()>*>(ev.user.data1);  // キューが一杯など
        return false;
    }
    return true;
}

}  // namespace tokotachi
