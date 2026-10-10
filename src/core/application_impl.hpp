// Application の中身(内部)。
#ifndef TOKOTACHI_SRC_CORE_APPLICATION_IMPL_HPP
#define TOKOTACHI_SRC_CORE_APPLICATION_IMPL_HPP

#include <SDL3/SDL.h>

#include <tokotachi/graphics/theme.hpp>

#include <functional>
#include <map>
#include <vector>

namespace tokotachi
{

class Application;
class Object;

namespace detail
{

struct WindowImpl;

// 環境変数 TOKOTACHI_TRACE が 1 なら、イベントの流れを標準エラーに出す(不具合の調べ用)。
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

    explicit ApplicationImpl(Application* q_)
        : q(q_), running(false), exit_code(0), next_timer_id(0), color_scheme(ColorScheme::SYSTEM),
          light_theme(Theme::light()), dark_theme(Theme::dark()), system_dark(false), dark(false)
    {
    }

    // イベントを 1 つ処理し、delete_later で頼まれたオブジェクトを破棄する。
    void dispatch(const SDL_Event& e);
    void dispatch_event(const SDL_Event& e);
    // interval_ms ごとに fn をメインスレッドで呼ぶ(repeat が false なら 1 回だけ)。戻り値は stop_timer() に渡す番号。
    // 利用者には Timer として見せる(docs/adr/0014)。
    int start_timer(int interval_ms, const std::function<void()>& fn, bool repeat);
    void stop_timer(int id);
    WindowImpl* find_window(SDL_Window* w) const;
    // 表示されているウィンドウ(ポップアップを除く)が無ければ終える。
    void quit_if_no_window();
    // 開いているメニューの種類のポップアップをすべて閉じる。
    void close_menus();
    // 他のスレッドからも呼べる: fn を、メインスレッドのイベントの処理の中で呼ぶ。
    static void post(const std::function<void()>& fn);
    // delete_later で頼まれたオブジェクトを破棄する(イベントを 1 つ処理するごとに呼ぶ)。
    void flush_doomed();
    // w への入力を、開いているモーダルのダイアログが止めるか(docs/adr/0028)。止めるならそのダイアログ。
    WindowImpl* modal_blocker(const WindowImpl* w) const;
    // color_scheme・OS の設定・light_theme・dark_theme から使うテーマを決め直す。変わっていれば、
    // プロパティ(dark・theme)の変更を知らせ、すべてのウィンドウを描き直し、on_theme_change を呼ぶ(docs/adr/0023)。
    void update_theme();

    Application* q;
    std::vector<WindowImpl*> windows;
    // 開いているメニューの種類のポップアップ(開いた順。docs/adr/0022)。
    std::vector<WindowImpl*> open_menus;
    // 開いているモーダルのダイアログ(開いた順)。
    std::vector<WindowImpl*> modal_dialogs;
    // delete_later で頼まれたオブジェクト。
    std::vector<Object*> doomed;
    std::map<int, Timer> timers;
    bool running;
    int exit_code;
    int next_timer_id;

    ColorScheme color_scheme;
    Theme light_theme;
    Theme dark_theme;
    bool system_dark;     // OS の設定がダークか
    bool dark;            // 今ダークのテーマを使っているか
    Theme current_theme;  // 今使っているテーマ
};

}  // namespace detail
}  // namespace tokotachi

#endif
