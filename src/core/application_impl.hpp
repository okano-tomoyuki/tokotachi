// Application の中身(内部)。
#ifndef GUI_SRC_CORE_APPLICATION_IMPL_HPP
#define GUI_SRC_CORE_APPLICATION_IMPL_HPP

#include <SDL3/SDL.h>

#include <functional>
#include <map>
#include <vector>

namespace gui
{
namespace detail
{

struct WindowImpl;

// 環境変数 GUI_TRACE が 1 なら、イベントの流れを標準エラーに出す(不具合の調べ用)。
bool trace_enabled();
// ビデオドライバが Wayland か。
bool is_wayland();
// ウィンドウの名前(メインのウィンドウは "window"、ポップアップは "popup#番号")。
const char* trace_name(const WindowImpl* w);

struct ApplicationImpl
{
    struct Timer
    {
        std::function<void()> fn;
        bool repeat;
        SDL_TimerID sdl_id;
    };

    ApplicationImpl() : running(false), exit_code(0), next_timer_id(0) {}

    void dispatch(const SDL_Event& e);
    // interval_ms ごとに fn をメインスレッドで呼ぶ(repeat が false なら 1 回だけ)。戻り値は stop_timer() に渡す番号。
    // 利用者には Timer として見せる(docs/adr/0014)。
    int start_timer(int interval_ms, const std::function<void()>& fn, bool repeat);
    void stop_timer(int id);
    WindowImpl* find_window(SDL_Window* w) const;
    // 表示されているウィンドウ(ポップアップを除く)が無ければ終える。
    void quit_if_no_window();
    // 開いているメニューの種類のポップアップをすべて閉じる。
    void close_menus();

    std::vector<WindowImpl*> windows;
    // 開いているメニューの種類のポップアップ(開いた順。docs/adr/0022)。
    std::vector<WindowImpl*> open_menus;
    std::map<int, Timer> timers;
    bool running;
    int exit_code;
    int next_timer_id;
};

}  // namespace detail
}  // namespace gui

#endif
