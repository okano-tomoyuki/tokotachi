// Window の中身(内部)。SDL3 のウィンドウ・描画のバッファ・入力の状態を持つ。
#ifndef GUI_SRC_CORE_WINDOW_IMPL_HPP
#define GUI_SRC_CORE_WINDOW_IMPL_HPP

#include <SDL3/SDL.h>

#include <cstdint>
#include <vector>

#include <gui/popup.hpp>
#include <gui/timer.hpp>
#include <gui/window.hpp>

#include "../gfx/raster_canvas.hpp"

namespace gui
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
    void update_text_input();
    void update_input_method_area();
    void widget_destroyed(Widget* w);
    // w が隠された・無効にされた(w とその子孫からフォーカス・ホバー・捕捉を外す)。
    void widget_hidden(Widget* w);

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
    Widget* focus;
    bool text_input_active;
    bool ime_dirty;
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
    // ショートカットが一致する Action を実行する。実行したら true。
    bool trigger_shortcut(const KeyEvent& ev);
};

}  // namespace detail
}  // namespace gui

#endif
