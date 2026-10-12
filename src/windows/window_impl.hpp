// Window の中身(内部)。SDL3 のウィンドウ・描画のバッファ・入力の状態を持つ。
#ifndef TOKOTACHI_SRC_WINDOWS_WINDOW_IMPL_HPP
#define TOKOTACHI_SRC_WINDOWS_WINDOW_IMPL_HPP

#include <SDL3/SDL.h>

#include <cstdint>
#include <vector>

#include <tokotachi/windows/popup.hpp>
#include <tokotachi/core/timer.hpp>
#include <tokotachi/windows/window.hpp>

#include "../widgets/widget_access.hpp"
#include "../core/impl.hpp"
#include "../graphics/raster_canvas.hpp"

namespace tokotachi
{
namespace detail
{

struct WindowImpl
{
    explicit WindowImpl(Window* q);
    ~WindowImpl();

    // このウィンドウ宛ての SDL のイベント。
    void handle(const SDL_Event& e);
    // 無効になった領域があれば描く。present が false なら画面には出さない(バッファにだけ描く)。
    void render(bool present = true);

    // dp(ウィンドウの座標系)の領域を無効にする。
    void invalidate(const Rect& r);
    void invalidate_all();

    void set_shown(bool shown);
    // ポップアップの SDL3 のウィンドウを作る(作ってあれば位置と大きさを変える)。x・y・w・h は親の window の座標。
    bool create_popup(SDL_Window* parent, bool tooltip, int x, int y, int w, int h);
    // SDL3 のウィンドウ・レンダラ・テクスチャを破棄する(Wayland でポップアップを閉じるとき。次に開くとき作り直す)。
    void destroy_native();
    // 隠れているトップレベルのウィンドウを、size(dp)の大きさで作り直す(題名・状態は引き継ぐ)。
    // WSLg(Wayland)では、作ってから見せる前に大きさを変えると、最初の表示が崩れるため(docs/adr/0028)。
    bool recreate_toplevel(const Size& size);
    // ツールチップを出す・消す(docs/adr/0022)。
    void schedule_tooltip();
    void hide_tooltip();
    // 依頼されている配置をまとめて行う(docs/adr/0017)。render() の始めにも呼ぶ。
    void run_layout();
    // dp の大きさを SDL3 のウィンドウの大きさにする(docs/adr/0008)。
    void request_size(const Size& size);
    // 倍率を取り直す。変わっていればウィジェットに知らせる。
    void update_scale();
    // テーマが変わった: ウィジェットに知らせ、配置をやり直し、全体を描き直す。
    void theme_changed();
    // SDL3 のウィンドウの大きさを、ウィンドウのウィジェットの大きさ(dp)に反映し、変わったプロパティを知らせる。
    void sync_size();

    void set_focus(Widget* w);
    void focus_next(bool forward);
    /// タブの順で w の後ろ(子孫を除く)・w か子孫の最初の部品へフォーカスを移す(docs/adr/0049)。
    bool focus_after(Widget* w);
    bool focus_first_in(Widget* w);
    /// Alt+key のニーモニックを持つ部品を探して知らせる。見つかれば true。
    bool activate_mnemonic(Key key);
    /// ニーモニックの下線を出すか(Alt を押している間)。変われば全体を描き直す。
    void set_cues(bool on);
    void update_text_input();
    void update_input_method_area();
    void widget_destroyed(Widget* w);
    // w が隠された・無効にされた(w とその子孫からフォーカス・ホバー・捕捉を外す)。
    void widget_hidden(Widget* w);
    // ドラッグを始める(docs/adr/0047)。ボタンを押していない・ほかのドラッグの途中なら false。
    bool start_drag(Widget* source, const DragData& data, const std::string& label);

    Window* q;
    SDL_Window* window;
    // モーダルのダイアログとして開いているときの親(docs/adr/0028)。
    WindowImpl* modal_parent;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    std::vector<std::uint32_t> buffer;
    int pixel_width, pixel_height;
    RasterCanvas canvas;
    float scale;    // dp -> pixel(SDL_GetWindowDisplayScale)
    float density;  // window -> pixel(SDL_GetWindowPixelDensity)
    PixelRect dirty;
    bool shown;
    bool popup;  // ポップアップか(Popup)
    SDL_Window* popup_parent;  // ポップアップに対応しないドライバで、代わりに通常のウィンドウを使っているときの親
    Point pointer;  // 最後のポインタの位置(dp)
    Timer* tooltip_timer;
    Popup* tooltip;
    bool layout_pending;
    Size minimum_size;  // SDL3 に伝えたウィンドウの最小の大きさ(dp)

    Widget* hover;
    Widget* grab;
    // ドラッグ&ドロップの途中の状態(docs/adr/0047)
    struct Drag
    {
        bool active;
        bool outside;         // OS から
        DragData data;
        Widget* source;
        std::string label;
        Widget* target;       // 受け取ると答えたウィジェット
        DropAction action;    // target が答えた操作
        Point pos;            // ポインタの位置(dp)
        Rect ghost;           // 最後に描いた像の範囲
    } drag;
    Widget* focus;
    bool text_input_active;
    bool ime_dirty;
    bool cues;  ///< ニーモニックの下線を出す(Alt を押している間)
    SDL_Cursor* cursors[7];

    // w の cursor が変わった(w がポインタの下・捕捉中なら、OS のカーソルを変える)。
    void cursor_changed(Widget* w);

private:
    void ensure_target();
    void layout_tree(Widget* w);
    void paint_tree(Widget* w);
    Point to_dp(float wx, float wy) const { return Point(wx * density / scale, wy * density / scale); }
    Widget* widget_at(const Point& p) const;
    void set_hover(Widget* w);
    void update_cursor();
    void mouse_down(const Point& p, MouseButton button, int clicks);
    void mouse_up(const Point& p, MouseButton button, int clicks);
    void mouse_move(const Point& p);
    void key(const SDL_KeyboardEvent& e);
    // ドラッグの位置を p にし、受け取るウィジェットを探し直す。
    void drag_update(const Point& p, Modifiers mods);
    // ドラッグを終える(drop が true なら、受け取るウィジェットへ落とす)。
    void drag_end(bool drop, Modifiers mods);
    // OS からのドロップ(SDL_EVENT_DROP_*)。
    void os_drop(const SDL_DropEvent& e);
    // ドラッグ中の像(label)を描く・その範囲。
    Rect drag_ghost_rect() const;
    void paint_drag_ghost();
    // ショートカットが一致する Action を実行する。実行したら true。
    bool trigger_shortcut(const KeyEvent& ev);
};

}  // namespace detail

class Window::Impl : public detail::WidgetHooks
{
    TOKOTACHI_IMPL(Window);

    explicit Impl(Window* object) : q(object), owner_window_(nullptr), destroy_on_close_(false) {}

    // トップレベルのウィンドウを作る(owner が無ければ Application が所有する。docs/adr/0062)。
    void init_toplevel(Window* owner, const std::string& title_text, float width_dp, float height_dp);
    void set_destroy_on_close(const bool& v) { destroy_on_close_ = v; }

    // 内部のフック(docs/adr/0058)
    void child_removed(Widget* child) override;

    std::string get_title() const;
    void set_title(const std::string& v);
    ThemeColor get_background() const { return background_; }
    void set_background(const ThemeColor& v);
    Widget* get_focused_widget() const;
    MenuBar* get_menu_bar() const { return menu_bar_; }
    void set_menu_bar(MenuBar* const& v);
    ToolBar* get_tool_bar() const { return tool_bar_; }
    void set_tool_bar(ToolBar* const& v);
    StatusBar* get_status_bar() const { return status_bar_; }
    void set_status_bar(StatusBar* const& v);

    detail::WindowImpl* native_;
    ThemeColor background_;
    CloseHandler on_close_;
    MenuBar* menu_bar_;
    ToolBar* tool_bar_;
    StatusBar* status_bar_;
    // 所有者のウィンドウ(Window(owner, ...) で作ったとき。表示するとその上に出し、その所有者を隠すと一緒に隠す)。
    Window* owner_window_;
    bool destroy_on_close_;
};

namespace detail
{

// w の SDL3 のウィンドウなどの中身。
inline WindowImpl* window_impl_of(const Window* w) { return ImplAccess<Window>::of(w)->native_; }

}  // namespace detail
}  // namespace tokotachi

#endif
