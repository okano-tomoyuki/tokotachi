#include <gui/window.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>

#include <gui/application.hpp>
#include <gui/box_layout.hpp>
#include <gui/canvas.hpp>
#include <gui/label.hpp>
#include <gui/popup.hpp>

#include "../layout/layout_access.hpp"
#include "application_impl.hpp"
#include "property_impl.hpp"
#include "widget_access.hpp"
#include "window_impl.hpp"

namespace gui
{

namespace detail
{

namespace
{

Modifiers to_modifiers(SDL_Keymod mod)
{
    Modifiers m = MODIFIER_NONE;
    if (mod & SDL_KMOD_SHIFT)
    {
        m |= MODIFIER_SHIFT;
    }
    if (mod & SDL_KMOD_CTRL)
    {
        m |= MODIFIER_CONTROL;
    }
    if (mod & SDL_KMOD_ALT)
    {
        m |= MODIFIER_ALT;
    }
    if (mod & SDL_KMOD_GUI)
    {
        m |= MODIFIER_META;
    }
    return m;
}

Key to_key(SDL_Keycode k)
{
    if (k >= SDLK_A && k <= SDLK_Z)
    {
        return static_cast<Key>(static_cast<int>(Key::A) + static_cast<int>(k - SDLK_A));
    }
    if (k >= SDLK_0 && k <= SDLK_9)
    {
        return static_cast<Key>(static_cast<int>(Key::DIGIT_0) + static_cast<int>(k - SDLK_0));
    }
    if (k >= SDLK_F1 && k <= SDLK_F12)
    {
        return static_cast<Key>(static_cast<int>(Key::F1) + static_cast<int>(k - SDLK_F1));
    }
    switch (k)
    {
    case SDLK_BACKSPACE:
        return Key::BACKSPACE;
    case SDLK_TAB:
        return Key::TAB;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        return Key::RETURN;
    case SDLK_ESCAPE:
        return Key::ESCAPE;
    case SDLK_SPACE:
        return Key::SPACE;
    case SDLK_DELETE:
        return Key::DEL;
    case SDLK_INSERT:
        return Key::INSERT;
    case SDLK_LEFT:
        return Key::LEFT;
    case SDLK_RIGHT:
        return Key::RIGHT;
    case SDLK_UP:
        return Key::UP;
    case SDLK_DOWN:
        return Key::DOWN;
    case SDLK_HOME:
        return Key::HOME;
    case SDLK_END:
        return Key::END;
    case SDLK_PAGEUP:
        return Key::PAGE_UP;
    case SDLK_PAGEDOWN:
        return Key::PAGE_DOWN;
    default:
        return Key::UNKNOWN;
    }
}

MouseButton to_button(Uint8 b)
{
    switch (b)
    {
    case SDL_BUTTON_LEFT:
        return MouseButton::LEFT;
    case SDL_BUTTON_MIDDLE:
        return MouseButton::MIDDLE;
    case SDL_BUTTON_RIGHT:
        return MouseButton::RIGHT;
    default:
        return MouseButton::NONE;
    }
}

bool takes_tab_focus(const Widget* w)
{
    const FocusPolicy p = w->focus_policy;
    return (p == FocusPolicy::TAB || p == FocusPolicy::TAB_AND_CLICK) && w->showing && w->effectively_enabled;
}

bool takes_click_focus(const Widget* w)
{
    const FocusPolicy p = w->focus_policy;
    return p == FocusPolicy::CLICK || p == FocusPolicy::TAB_AND_CLICK;
}

void collect_focusable(Widget* w, std::vector<Widget*>& out)
{
    if (!WidgetAccess::visible_flag(w))
    {
        return;
    }
    if (takes_tab_focus(w))
    {
        out.push_back(w);
    }
    const std::vector<Widget*>& children = WidgetAccess::children(w);
    for (std::size_t i = 0; i < children.size(); ++i)
    {
        collect_focusable(children[i], out);
    }
}

}  // namespace

WindowImpl::WindowImpl(Window* q_)
    : q(q_), window(nullptr), renderer(nullptr), texture(nullptr), pixel_width(0), pixel_height(0), scale(1),
      density(1), shown(false), popup(false), popup_parent(nullptr), tooltip_timer(nullptr), tooltip(nullptr), layout_pending(true),
      hover(nullptr), grab(nullptr), focus(nullptr), text_input_active(false),
      ime_dirty(false)
{
    dirty.x0 = dirty.y0 = dirty.x1 = dirty.y1 = 0;
    static const SDL_SystemCursor ids[7] = {SDL_SYSTEM_CURSOR_DEFAULT,   SDL_SYSTEM_CURSOR_TEXT,
                                            SDL_SYSTEM_CURSOR_POINTER,   SDL_SYSTEM_CURSOR_WAIT,
                                            SDL_SYSTEM_CURSOR_CROSSHAIR, SDL_SYSTEM_CURSOR_EW_RESIZE,
                                            SDL_SYSTEM_CURSOR_NS_RESIZE};
    for (int i = 0; i < 7; ++i)
    {
        cursors[i] = SDL_CreateSystemCursor(ids[i]);
    }
}

WindowImpl::~WindowImpl()
{
    delete tooltip_timer;
    if (text_input_active)
    {
        SDL_StopTextInput(window);
    }
    for (int i = 0; i < 7; ++i)
    {
        if (cursors[i])
        {
            SDL_DestroyCursor(cursors[i]);
        }
    }
    if (texture)
    {
        SDL_DestroyTexture(texture);
    }
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }
    if (window)
    {
        SDL_DestroyWindow(window);
    }
}

void WindowImpl::invalidate(const Rect& r)
{
    if (r.is_empty())
    {
        return;
    }
    // アンチエイリアスのはみ出しを含めるため、1 ピクセル広げる。
    const PixelRect p = {static_cast<int>(std::floor(r.x * scale)) - 1, static_cast<int>(std::floor(r.y * scale)) - 1,
                         static_cast<int>(std::ceil(r.right() * scale)) + 1,
                         static_cast<int>(std::ceil(r.bottom() * scale)) + 1};
    if (dirty.empty())
    {
        dirty = p;
    }
    else
    {
        dirty.x0 = std::min(dirty.x0, p.x0);
        dirty.y0 = std::min(dirty.y0, p.y0);
        dirty.x1 = std::max(dirty.x1, p.x1);
        dirty.y1 = std::max(dirty.y1, p.y1);
    }
}

void WindowImpl::invalidate_all()
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    const PixelRect all = {0, 0, std::max(std::max(pixel_width, w), 1), std::max(std::max(pixel_height, h), 1)};
    dirty = all;
}

void WindowImpl::ensure_target()
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    w = std::max(w, 1);
    h = std::max(h, 1);
    if (w == pixel_width && h == pixel_height && texture)
    {
        return;
    }
    pixel_width = w;
    pixel_height = h;
    buffer.assign(static_cast<std::size_t>(w) * static_cast<std::size_t>(h), 0);
    canvas.set_target(&buffer[0], w, h);
    if (texture)
    {
        SDL_DestroyTexture(texture);
    }
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    const PixelRect all = {0, 0, w, h};
    dirty = all;
}

void WindowImpl::paint_tree(Widget* w)
{
    if (!WidgetAccess::visible_flag(w))
    {
        return;
    }
    canvas.save();
    if (w != q)
    {
        const Rect& b = WidgetAccess::bounds(w);
        canvas.translate(b.x, b.y);
    }
    canvas.clip_rect(w->local_rect);
    if (!canvas.clip_bounds().is_empty())
    {
        WidgetAccess::paint(w, canvas);
        const std::vector<Widget*>& children = WidgetAccess::children(w);
        for (std::size_t i = 0; i < children.size(); ++i)
        {
            paint_tree(children[i]);
        }
    }
    canvas.restore();
}

void WindowImpl::layout_tree(Widget* w)
{
    // 隠れている子は、表示されたときに配置する(印は残る)。ウィンドウ自身は表示の前でも配置する
    // (表示したときに配置が済んでいるように・save_png や apply_layout で結果を読めるように)。
    if (w != q && !WidgetAccess::visible_flag(w))
    {
        return;
    }
    if (WidgetAccess::layout_dirty(w))
    {
        WidgetAccess::clear_layout_dirty(w);
        WidgetAccess::arrange_children(w);
    }
    const std::vector<Widget*> children = WidgetAccess::children(w);
    for (std::size_t i = 0; i < children.size(); ++i)
    {
        layout_tree(children[i]);
    }
}

void WindowImpl::run_layout()
{
    // 配置の結果を受けたハンドラが、さらに配置を依頼することがあるので、落ち着くまで(上限あり)繰り返す。
    for (int i = 0; layout_pending && i < 8; ++i)
    {
        layout_pending = false;
        layout_tree(q);
    }
    // レイアウトを持つウィンドウは、その最小の大きさより小さくできないようにする。
    const Size m = WidgetAccess::layout_of(q) ? q->minimum_hint.get() : Size();
    if (m != minimum_size)
    {
        minimum_size = m;
        SDL_SetWindowMinimumSize(window, static_cast<int>(std::ceil(m.width * scale / density)),
                                 static_cast<int>(std::ceil(m.height * scale / density)));
    }
}

void WindowImpl::render(bool present)
{
    run_layout();
    if (!window || dirty.empty())
    {
        return;
    }
    ensure_target();
    const PixelRect clip = {std::max(dirty.x0, 0), std::max(dirty.y0, 0), std::min(dirty.x1, pixel_width),
                            std::min(dirty.y1, pixel_height)};
    dirty.x0 = dirty.y0 = dirty.x1 = dirty.y1 = 0;
    if (clip.empty())
    {
        return;
    }

    canvas.set_scale(scale);
    canvas.begin(clip);
    paint_tree(q);
    canvas.end();

    const SDL_Rect r = {clip.x0, clip.y0, clip.x1 - clip.x0, clip.y1 - clip.y0};
    SDL_UpdateTexture(texture, &r,
                      &buffer[static_cast<std::size_t>(clip.y0) * static_cast<std::size_t>(pixel_width) +
                              static_cast<std::size_t>(clip.x0)],
                      pixel_width * 4);
    if (present && shown)
    {
        if (trace_enabled())
        {
            std::fprintf(stderr, "[gui] present         %s\n", trace_name(this));
        }
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
    update_input_method_area();
}

void WindowImpl::set_shown(bool show)
{
    if (show == shown)
    {
        return;
    }
    shown = show;
    WidgetAccess::set_visible_flag(q, show);
    WidgetAccess::notify_visible(q);
    if (show)
    {
        SDL_ShowWindow(window);
        invalidate_all();
        // 表示の前にフォーカスを移したウィジェットは、表示されていなかったので文字入力を有効にしていない。ここで判断し直す。
        update_text_input();
    }
    else
    {
        widget_hidden(q);
        if (popup && is_wayland())
        {
            // Wayland(WSLg で確かめた)では、隠したポップアップが画面に残ることがある。破棄すれば確実に消える。
            destroy_native();
        }
        else
        {
            SDL_HideWindow(window);
        }
    }
}

void WindowImpl::destroy_native()
{
    if (text_input_active)
    {
        SDL_StopTextInput(window);
        text_input_active = false;
    }
    if (texture)
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    pixel_width = pixel_height = 0;
    dirty.x0 = dirty.y0 = dirty.x1 = dirty.y1 = 0;
}

bool WindowImpl::create_popup(SDL_Window* parent, bool is_tooltip, int x, int y, int w, int h)
{
    if (!window)
    {
        const SDL_WindowFlags flags = (is_tooltip ? SDL_WINDOW_TOOLTIP : SDL_WINDOW_POPUP_MENU) |
                                      SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN;
        window = SDL_CreatePopupWindow(parent, x, y, std::max(1, w), std::max(1, h), flags);
        if (!window)
        {
            // ポップアップに対応しないビデオドライバ(offscreen 等)では、枠の無い通常のウィンドウを、親の位置 + 相対の位置に置く。
            // Wayland はポップアップに対応するので、ここは使わない(Wayland では絶対の位置を指定できない。docs/adr/0002)。
            window = SDL_CreateWindow("", std::max(1, w), std::max(1, h),
                                      SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN |
                                          SDL_WINDOW_ALWAYS_ON_TOP | (is_tooltip ? SDL_WINDOW_NOT_FOCUSABLE : 0));
            popup_parent = parent;
        }
        if (window && popup_parent)
        {
            int px = 0, py = 0;
            SDL_GetWindowPosition(popup_parent, &px, &py);
            SDL_SetWindowPosition(window, px + x, py + y);
        }
        if (!window)
        {
            std::fprintf(stderr, "SDL_CreatePopupWindow: %s\n", SDL_GetError());
            return false;
        }
        renderer = SDL_CreateRenderer(window, nullptr);
    }
    else
    {
        int px = 0, py = 0;
        if (popup_parent)
        {
            SDL_GetWindowPosition(popup_parent, &px, &py);  // 代わりの通常のウィンドウは絶対の位置
        }
        SDL_SetWindowPosition(window, px + x, py + y);  // ポップアップの位置は親からの相対
        SDL_SetWindowSize(window, std::max(1, w), std::max(1, h));
    }
    update_scale();
    sync_size();
    return true;
}

void WindowImpl::schedule_tooltip()
{
    hide_tooltip();
    if (popup || !hover || hover->tooltip.get().empty())
    {
        return;
    }
    if (!tooltip_timer)
    {
        tooltip_timer = new Timer();
        tooltip_timer->interval = 600;
        tooltip_timer->repeat = false;
        tooltip_timer->on_timer = [this](Object*)
        {
            if (!hover || hover->tooltip.get().empty() || !shown)
            {
                return;
            }
            if (!tooltip)
            {
                tooltip = new Popup(q, PopupKind::TOOLTIP);
                tooltip->background = Color::rgb(0x1f2937);
                Label* text = new Label(tooltip);
                text->color = Color::rgb(0xffffff);
                text->font = Font(12);
                BoxLayout* box = new BoxLayout(tooltip);
                box->margins = Margins(6, 4, 6, 4);
                box->add(text);
            }
            Label* text = static_cast<Label*>(tooltip->children.get().front());
            text->text = hover->tooltip.get();
            const Size size = tooltip->preferred_size;
            // ポインタの少し下に出す。
            tooltip->open(Rect(pointer.x, pointer.y + 20, 0, 0), size, PopupPlacement::AT);
        };
    }
    tooltip_timer->enabled = true;
}

void WindowImpl::hide_tooltip()
{
    if (tooltip_timer)
    {
        tooltip_timer->enabled = false;
    }
    if (tooltip && tooltip->visible)
    {
        tooltip->close();
    }
}

void WindowImpl::request_size(const Size& size)
{
    SDL_SetWindowSize(window, static_cast<int>(std::floor(size.width * scale / density + 0.5f)),
                      static_cast<int>(std::floor(size.height * scale / density + 0.5f)));
    sync_size();
}

void WindowImpl::update_scale()
{
    float s = SDL_GetWindowDisplayScale(window);
    float d = SDL_GetWindowPixelDensity(window);
    if (s <= 0)
    {
        s = 1;
    }
    if (d <= 0)
    {
        d = 1;
    }
    const bool changed = s != scale;
    scale = s;
    density = d;
    if (changed)
    {
        WidgetAccess::scale_changed(q);
        invalidate_all();
        ime_dirty = true;
    }
}

void WindowImpl::sync_size()
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    const Rect r(0, 0, w / scale, h / scale);
    const Rect old = WidgetAccess::bounds(q);
    WidgetAccess::set_bounds_raw(q, r);
    invalidate_all();
    WidgetAccess::notify_bounds(q, old);
    if (r.size() != old.size())
    {
        WidgetAccess::mark_layout_dirty(q);
        WidgetAccess::resized(q);
    }
}

Widget* WindowImpl::widget_at(const Point& p) const
{
    Widget* w = q->child_at(p);
    return w ? w : q;
}

void WindowImpl::set_hover(Widget* w)
{
    if (w == hover)
    {
        return;
    }
    Widget* old = hover;
    hover = w;
    schedule_tooltip();
    if (old)
    {
        WidgetAccess::mouse_leave(old);
    }
    if (w)
    {
        WidgetAccess::mouse_enter(w);
    }
    update_cursor();
}

void WindowImpl::update_cursor()
{
    Widget* w = grab ? grab : hover;
    const int shape = w && w->effectively_enabled ? static_cast<int>(w->cursor.get()) : 0;
    if (cursors[shape])
    {
        SDL_SetCursor(cursors[shape]);
    }
}

void WindowImpl::set_focus(Widget* w)
{
    if (w == focus)
    {
        return;
    }
    Widget* old = focus;
    focus = w;
    update_text_input();
    // ハンドラより先に、結びつけた相手へ伝える(docs/adr/0016)。
    if (old)
    {
        WidgetAccess::notify_focused(old);
    }
    if (w)
    {
        WidgetAccess::notify_focused(w);
    }
    if (old)
    {
        WidgetAccess::focus_out(old);
    }
    if (w)
    {
        WidgetAccess::focus_in(w);
    }
}

void WindowImpl::focus_next(bool forward)
{
    std::vector<Widget*> list;
    collect_focusable(q, list);
    if (list.empty())
    {
        return;
    }
    std::size_t index = 0;
    const std::vector<Widget*>::iterator it = std::find(list.begin(), list.end(), focus);
    if (it == list.end())
    {
        index = forward ? 0 : list.size() - 1;
    }
    else
    {
        const std::size_t cur = static_cast<std::size_t>(it - list.begin());
        index = forward ? (cur + 1) % list.size() : (cur + list.size() - 1) % list.size();
    }
    set_focus(list[index]);
}

void WindowImpl::update_text_input()
{
    const bool want = focus && focus->accepts_text_input && focus->showing && focus->effectively_enabled;
    if (want != text_input_active)
    {
        text_input_active = want;
        if (want)
        {
            SDL_StartTextInput(window);
        }
        else
        {
            SDL_StopTextInput(window);
        }
    }
    ime_dirty = true;
}

void WindowImpl::update_input_method_area()
{
    if (!ime_dirty || !text_input_active || !focus)
    {
        return;
    }
    ime_dirty = false;
    // IME の候補窓の位置は window の座標で伝える(docs/adr/0008、0009)。
    const Rect local = WidgetAccess::input_method_rect(focus);
    const Point origin = focus->map_to_window(local.position());
    const float to_window = scale / density;
    SDL_Rect r;
    r.x = static_cast<int>(std::floor(origin.x * to_window));
    r.y = static_cast<int>(std::floor(origin.y * to_window));
    r.w = std::max(1, static_cast<int>(std::ceil(local.width * to_window)));
    r.h = std::max(1, static_cast<int>(std::ceil(local.height * to_window)));
    SDL_SetTextInputArea(window, &r, 0);
}

void WindowImpl::widget_destroyed(Widget* w)
{
    if (hover == w)
    {
        hover = nullptr;
    }
    if (grab == w)
    {
        grab = nullptr;
    }
    if (focus == w)
    {
        focus = nullptr;
        update_text_input();
    }
}

void WindowImpl::widget_hidden(Widget* w)
{
    if (focus && is_ancestor_or_self(w, focus))
    {
        set_focus(nullptr);
    }
    if (grab && is_ancestor_or_self(w, grab))
    {
        grab = nullptr;
    }
    if (hover && is_ancestor_or_self(w, hover))
    {
        set_hover(nullptr);
    }
}

void WindowImpl::mouse_down(const Point& p, MouseButton button, int clicks)
{
    hide_tooltip();
    Widget* target = widget_at(p);
    if (!target->effectively_enabled)
    {
        return;
    }
    // クリックでフォーカスを得るウィジェットを、押した位置から親へ向かって探す。
    for (Widget* w = target; w; w = WidgetAccess::parent_of(w))
    {
        if (takes_click_focus(w))
        {
            set_focus(w);
            break;
        }
    }
    grab = nullptr;
    const Modifiers mods = to_modifiers(SDL_GetModState());
    for (Widget* w = target; w; w = WidgetAccess::parent_of(w))
    {
        MouseEvent ev(w->map_from_window(p), button, mods, clicks);
        WidgetAccess::mouse_down(w, ev);
        if (ev.accepted)
        {
            // 押したボタンを離すまで、移動と離したことをこのウィジェットに送る(暗黙の捕捉)。
            grab = w;
            break;
        }
    }
}

void WindowImpl::mouse_up(const Point& p, MouseButton button, int clicks)
{
    if (Widget* w = grab)
    {
        grab = nullptr;
        MouseEvent ev(w->map_from_window(p), button, to_modifiers(SDL_GetModState()), clicks);
        WidgetAccess::mouse_up(w, ev);
    }
    set_hover(widget_at(p));
    update_cursor();
}

void WindowImpl::mouse_move(const Point& p)
{
    pointer = p;
    const Modifiers mods = to_modifiers(SDL_GetModState());
    if (grab)
    {
        MouseEvent ev(grab->map_from_window(p), MouseButton::NONE, mods, 0);
        WidgetAccess::mouse_move(grab, ev);
        return;
    }
    Widget* target = widget_at(p);
    set_hover(target);
    if (target->effectively_enabled)
    {
        MouseEvent ev(target->map_from_window(p), MouseButton::NONE, mods, 0);
        WidgetAccess::mouse_move(target, ev);
    }
}

void WindowImpl::key(const SDL_KeyboardEvent& e)
{
    hide_tooltip();
    const Modifiers mods = to_modifiers(e.mod);
    const Key k = to_key(e.key);
    if (e.down && k == Key::TAB && !(mods & (MODIFIER_CONTROL | MODIFIER_ALT)))
    {
        focus_next(!(mods & MODIFIER_SHIFT));
        return;
    }
    for (Widget* w = focus ? focus : q; w; w = WidgetAccess::parent_of(w))
    {
        KeyEvent ev(k, mods, e.repeat, static_cast<std::uint32_t>(e.key));
        if (e.down)
        {
            WidgetAccess::key_down(w, ev);
        }
        else
        {
            WidgetAccess::key_up(w, ev);
        }
        if (ev.accepted)
        {
            break;
        }
    }
}

void WindowImpl::handle(const SDL_Event& e)
{
    switch (e.type)
    {
    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
    case SDL_EVENT_WINDOW_RESIZED:
        update_scale();
        sync_size();
        break;
    case SDL_EVENT_WINDOW_EXPOSED:
        invalidate_all();
        break;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
    {
        Object* self = q;
        bool can_close = true;
        fire(q->on_close.get(), self, can_close);
        if (can_close)
        {
            q->close();
        }
        break;
    }
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        if (!grab)
        {
            set_hover(nullptr);
        }
        hide_tooltip();
        break;
    case SDL_EVENT_MOUSE_MOTION:
        mouse_move(to_dp(e.motion.x, e.motion.y));
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        mouse_down(to_dp(e.button.x, e.button.y), to_button(e.button.button), e.button.clicks);
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        mouse_up(to_dp(e.button.x, e.button.y), to_button(e.button.button), e.button.clicks);
        break;
    case SDL_EVENT_MOUSE_WHEEL:
    {
        // ポインタの下のウィジェットへ送り、受け取らなければ親へ(docs/adr/0021)。
        const float flip = e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0f : 1.0f;
        const Point p = to_dp(e.wheel.mouse_x, e.wheel.mouse_y);
        const Modifiers mods = to_modifiers(SDL_GetModState());
        for (Widget* w = widget_at(p); w; w = WidgetAccess::parent_of(w))
        {
            if (!w->effectively_enabled)
            {
                continue;
            }
            WheelEvent ev(w->map_from_window(p), Point(e.wheel.x * flip, e.wheel.y * flip), mods);
            WidgetAccess::mouse_wheel(w, ev);
            if (ev.accepted)
            {
                break;
            }
        }
        break;
    }
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        key(e.key);
        break;
    case SDL_EVENT_TEXT_INPUT:
        if (focus && focus->accepts_text_input)
        {
            TextInputEvent ev(e.text.text ? e.text.text : "");
            WidgetAccess::text_input(focus, ev);
        }
        break;
    case SDL_EVENT_TEXT_EDITING:
        if (focus && focus->accepts_text_input)
        {
            TextEditingEvent ev(e.edit.text ? e.edit.text : "", e.edit.start, e.edit.length);
            WidgetAccess::text_editing(focus, ev);
        }
        break;
    default:
        break;
    }
}

}  // namespace detail

Window::Window(const std::string& title_text, float width_dp, float height_dp)
    : GUI_PROPERTY_GETTER(Window, std::string, title),
      GUI_PROPERTY_GETTER(Window, Color, background),
      GUI_READONLY_PROPERTY(Window, Widget*, focused_widget),
      GUI_HANDLER_PROPERTY(Window, CloseHandler, on_close),
      d_(nullptr),
      background_(Color::rgb(0xf5f6f8))
{
    Application* app = Application::instance();
    if (!app)
    {
        std::fprintf(stderr, "gui::Window: create gui::Application first\n");
        return;
    }
    detail::WidgetAccess::set_is_window(this, true);
    detail::WidgetAccess::set_visible_flag(this, false);
    d_ = new detail::WindowImpl(this);
    d_->window = SDL_CreateWindow(title_text.c_str(), static_cast<int>(width_dp), static_cast<int>(height_dp),
                                  SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
    if (!d_->window)
    {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return;
    }
    d_->renderer = SDL_CreateRenderer(d_->window, nullptr);
    if (!d_->renderer)
    {
        std::fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
    }
    d_->update_scale();
    d_->request_size(Size(width_dp, height_dp));
    app->impl()->windows.push_back(d_);
}

Window::~Window()
{
    // 子のデストラクタは window を通じてこのウィンドウに触るので、Window のうちに破棄する(タイマー・変数なども含む)。
    delete_owned();
    if (Application* app = Application::instance())
    {
        std::vector<detail::WindowImpl*>& ws = app->impl()->windows;
        ws.erase(std::remove(ws.begin(), ws.end(), d_), ws.end());
    }
    detail::WidgetAccess::set_is_window(this, false);
    delete d_;
}

std::string Window::get_title() const
{
    const char* t = SDL_GetWindowTitle(d_->window);
    return t ? t : "";
}

void Window::set_title(const std::string& v) { SDL_SetWindowTitle(d_->window, v.c_str()); }

void Window::set_background(const Color& v)
{
    background_ = v;
    invalidate();
}

Widget* Window::get_focused_widget() const { return d_->focus; }

Window::Window(Object* owner_object)
    : GUI_PROPERTY_GETTER(Window, std::string, title),
      GUI_PROPERTY_GETTER(Window, Color, background),
      GUI_READONLY_PROPERTY(Window, Widget*, focused_widget),
      GUI_HANDLER_PROPERTY(Window, CloseHandler, on_close),
      d_(nullptr),
      background_(Color::rgb(0xffffff))
{
    detail::WidgetAccess::set_is_window(this, true);
    detail::WidgetAccess::set_visible_flag(this, false);
    change_owner(owner_object);
    d_ = new detail::WindowImpl(this);
    d_->popup = true;
    if (Application* app = Application::instance())
    {
        app->impl()->windows.push_back(d_);
    }
}

void Window::close()
{
    visible = false;
    if (Application* app = Application::instance())
    {
        app->impl()->quit_if_no_window();
    }
}

bool Window::save_png(const std::string& path)
{
    d_->invalidate_all();
    d_->render(false);
    SDL_Surface* s = SDL_CreateSurfaceFrom(d_->pixel_width, d_->pixel_height, SDL_PIXELFORMAT_ARGB8888,
                                           &d_->buffer[0], d_->pixel_width * 4);
    const bool ok = s && SDL_SavePNG(s, path.c_str());
    if (!ok)
    {
        std::fprintf(stderr, "save_png: %s\n", SDL_GetError());
    }
    SDL_DestroySurface(s);
    return ok;
}

void Window::paint(Canvas& canvas)
{
    canvas.fill_rect(local_rect, background_);
    Widget::paint(canvas);
}

}  // namespace gui
