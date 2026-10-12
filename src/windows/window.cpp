#include <tokotachi/windows/window.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>

#include <tokotachi/core/application.hpp>
#include <tokotachi/layout/box_layout.hpp>
#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/graphics/text_layout.hpp>
#include <tokotachi/widgets/label.hpp>
#include <tokotachi/windows/menu_bar.hpp>
#include <tokotachi/windows/status_bar.hpp>
#include <tokotachi/windows/tool_bar.hpp>
#include <tokotachi/windows/popup.hpp>

#include "../core/object_impl.hpp"
#include "../layout/layout_access.hpp"
#include "../actions/menu_access.hpp"
#include "../actions/action_access.hpp"
#include "../core/application_impl.hpp"
#include "../core/impl.hpp"
#include "../widgets/widget_access.hpp"
#include "window_impl.hpp"

namespace tokotachi
{

/// DragData の内部の値を書く(ウィンドウだけが使う)。
struct DragAccess
{
    static void set_source(DragData& d, Widget* w) { d.source_ = w; }
    static void set_outside(DragData& d, bool v) { d.outside_ = v; }
};

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
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
        return Key::SHIFT;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
        return Key::CONTROL;
    case SDLK_LALT:
    case SDLK_RALT:
        return Key::ALT;
    case SDLK_LGUI:
    case SDLK_RGUI:
        return Key::META;
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

// w が root か、その子孫か。
bool within(const Widget* w, const Widget* root)
{
    for (; w; w = WidgetAccess::parent_of(w))
    {
        if (w == root)
        {
            return true;
        }
    }
    return false;
}

// 表示されている部品を木の前順に並べる。
void collect_displayed(Widget* w, std::vector<Widget*>& out)
{
    if (!WidgetAccess::displayed(w))
    {
        return;
    }
    out.push_back(w);
    const std::vector<Widget*>& children = WidgetAccess::children(w);
    for (std::size_t i = 0; i < children.size(); ++i)
    {
        collect_displayed(children[i], out);
    }
}

void collect_focusable(Widget* w, std::vector<Widget*>& out)
{
    if (!WidgetAccess::displayed(w))
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
    : q(q_), window(nullptr), modal_parent(nullptr), renderer(nullptr), texture(nullptr), pixel_width(0), pixel_height(0), scale(1),
      density(1), shown(false), popup(false), popup_parent(nullptr), tooltip_timer(nullptr), tooltip(nullptr), layout_pending(true),
      hover(nullptr), grab(nullptr), focus(nullptr), text_input_active(false),
      ime_dirty(false),
      cues(false)
{
    dirty.x0 = dirty.y0 = dirty.x1 = dirty.y1 = 0;
    drag.active = false;
    drag.outside = false;
    drag.source = nullptr;
    drag.target = nullptr;
    drag.action = DropAction::NONE;
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
    if (Application* app = Application::instance())
    {
        std::vector<WindowImpl*>& modals = detail::impl_of(app)->modal_dialogs;
        modals.erase(std::remove(modals.begin(), modals.end(), this), modals.end());
        for (std::size_t i = 0; i < modals.size(); ++i)
        {
            if (modals[i]->modal_parent == this)
            {
                modals[i]->modal_parent = nullptr;
            }
        }
    }
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
    if (!WidgetAccess::displayed(w))
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
        WidgetAccess::paint(w, canvas.api());
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
    if (w != q && !WidgetAccess::displayed(w))
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
    for (int round = 0; round < 2; ++round)
    {
        // 配置の結果を受けたハンドラが、さらに配置を依頼することがあるので、落ち着くまで(上限あり)繰り返す。
        for (int i = 0; layout_pending && i < 8; ++i)
        {
            layout_pending = false;
            layout_tree(q);
        }
        // レイアウトを持つウィンドウは、その最小の大きさより小さくできないようにする。
        const Size m = WidgetAccess::layout_of(q) ? q->minimum_hint.get() : Size();
        if (m == minimum_size || !window)
        {
            return;
        }
        minimum_size = m;
        SDL_SetWindowMinimumSize(window, static_cast<int>(std::ceil(m.width * scale / density)),
                                 static_cast<int>(std::ceil(m.height * scale / density)));
        // OS がその場で大きくしたら、知らせ(イベント)を待たずに大きさを合わせて配置し直す
        // (イベントを処理しない capture・デザイナーのキャンバスでも、広がった部分まで描くため)
        const Size before = WidgetAccess::bounds(q).size();
        sync_size();
        if (WidgetAccess::bounds(q).size() == before)
        {
            return;
        }
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
    paint_drag_ghost();
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
            std::fprintf(stderr, "[tokotachi] present         %s\n", trace_name(this));
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
        // 所有者のウィンドウの上に出す(docs/adr/0062)。所有者が隠れていれば親子にしない(隠れた親の子は出ない環境がある)。
        Window* owner = ImplAccess<Window>::of(q)->owner_window_;
        WindowImpl* owner_impl = owner ? window_impl_of(owner) : nullptr;
        if (owner_impl && owner_impl->window && owner_impl->shown)
        {
            SDL_SetWindowParent(window, owner_impl->window);
        }
        SDL_ShowWindow(window);
        invalidate_all();
        // 表示の前にフォーカスを移したウィジェットは、表示されていなかったので文字入力を有効にしていない。ここで判断し直す。
        update_text_input();
    }
    else
    {
        // 所有するウィンドウも隠す(docs/adr/0062)。隠すと一覧が変わることはないが、念のため写してから回す。
        if (Application* app = Application::instance())
        {
            const std::vector<WindowImpl*> ws = impl_of(app)->windows;
            for (std::size_t i = 0; i < ws.size(); ++i)
            {
                if (ws[i] != this && ws[i]->shown && ImplAccess<Window>::of(ws[i]->q)->owner_window_ == q)
                {
                    ws[i]->q->visible = false;
                }
            }
        }
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

bool WindowImpl::recreate_toplevel(const Size& size)
{
    if (shown || popup)
    {
        return false;
    }
    const std::string title_text = window ? SDL_GetWindowTitle(window) : "";
    const float k = scale / density;
    destroy_native();
    window = SDL_CreateWindow(title_text.c_str(), std::max(1, static_cast<int>(std::floor(size.width * k + 0.5f))),
                              std::max(1, static_cast<int>(std::floor(size.height * k + 0.5f))),
                              SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
    if (!window)
    {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return false;
    }
    renderer = SDL_CreateRenderer(window, nullptr);
    update_scale();
    sync_size();
    minimum_size = Size();  // 最小の大きさは次の配置で設定し直す
    layout_pending = true;
    return true;
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
        tooltip_timer->on_timeout = [this](Object*)
        {
            if (!hover || hover->tooltip.get().empty() || !shown)
            {
                return;
            }
            if (!tooltip)
            {
                tooltip = new Popup(q, PopupKind::TOOLTIP);
                tooltip->background = ColorRole::TOOLTIP;
                Label* text = new Label(tooltip);
                text->text_color = ColorRole::ON_TOOLTIP;
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

void WindowImpl::theme_changed()
{
    WidgetAccess::theme_changed(q);
    layout_pending = true;
    invalidate_all();
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

void WindowImpl::cursor_changed(Widget* w)
{
    if (w == (grab ? grab : hover))
    {
        update_cursor();
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

bool WindowImpl::focus_after(Widget* w)
{
    // 表示されている部品をタブの順(木の前順)に並べ、w の子孫の後ろから、フォーカスを受ける部品を探す
    std::vector<Widget*> all;
    collect_displayed(q, all);
    std::vector<Widget*>::iterator it = std::find(all.begin(), all.end(), w);
    if (it == all.end())
    {
        return false;
    }
    for (++it; it != all.end(); ++it)
    {
        if (!within(w, *it) && !within(*it, w) && takes_tab_focus(*it))
        {
            set_focus(*it);
            return true;
        }
    }
    return false;
}

bool WindowImpl::focus_first_in(Widget* w)
{
    std::vector<Widget*> list;
    collect_focusable(w, list);
    if (list.empty())
    {
        return false;
    }
    set_focus(list[0]);
    return true;
}

bool WindowImpl::activate_mnemonic(Key key)
{
    if (key == Key::UNKNOWN)
    {
        return false;
    }
    std::vector<Widget*> all, hits;
    collect_displayed(q, all);
    for (std::size_t i = 0; i < all.size(); ++i)
    {
        if (all[i]->effectively_enabled && WidgetAccess::mnemonic_key(all[i]) == key)
        {
            hits.push_back(all[i]);
        }
    }
    if (hits.empty())
    {
        return false;
    }
    // 同じキーの部品が複数あれば、フォーカスのある部品(またはその祖先)の次の部品へ
    Widget* target = hits[0];
    if (focus && hits.size() > 1)
    {
        const std::vector<Widget*>::iterator at = std::find(all.begin(), all.end(), focus);
        for (std::size_t i = 0; i < hits.size(); ++i)
        {
            if (std::find(all.begin(), at, hits[i]) == at && !within(focus, hits[i]))
            {
                target = hits[i];
                break;
            }
        }
    }
    WidgetAccess::mnemonic_activated(target);
    return true;
}

void WindowImpl::set_cues(bool on)
{
    if (on != cues)
    {
        cues = on;
        q->invalidate();
    }
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
    if (drag.source == w)
    {
        drag.source = nullptr;
        DragAccess::set_source(drag.data, nullptr);
    }
    if (drag.target == w)
    {
        drag.target = nullptr;
    }
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
    set_cues(false);
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
    // ドラッグの途中なら落とす(その後、始めたウィジェットにも離したことを送る)
    if (drag.active && !drag.outside)
    {
        drag_update(p, to_modifiers(SDL_GetModState()));
        drag_end(true, to_modifiers(SDL_GetModState()));
    }
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
    if (drag.active && !drag.outside)
    {
        drag_update(p, mods);
        return;
    }
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
    // ドラッグの途中: Esc でやめる。Ctrl で操作(移す・写す)が変わるので、受け取る側に聞き直す
    if (drag.active && !drag.outside)
    {
        if (e.down && k == Key::ESCAPE)
        {
            drag_end(false, mods);
        }
        else if (k == Key::CONTROL)
        {
            drag_update(drag.pos, mods);
        }
        return;
    }
    // Alt を押している間は、部品のニーモニックの下線を出す(docs/adr/0049)
    if (k == Key::ALT)
    {
        set_cues(e.down);
    }
    // メニューバーのキーボードの操作(Alt・F10・Alt+ニーモニック)を先に(docs/adr/0027)。
    if (MenuBar* bar = q->menu_bar)
    {
        const KeyEvent ev(k, mods, e.repeat, static_cast<std::uint32_t>(e.key));
        if (MenuBarAccess::handle_key(bar, ev, e.down))
        {
            return;
        }
    }
    // 部品のニーモニック(Alt+文字)。メニューバーのものが優先
    if (e.down && k != Key::ALT && (mods & MODIFIER_ALT) && !(mods & (MODIFIER_CONTROL | MODIFIER_SHIFT | MODIFIER_META)) &&
        activate_mnemonic(k))
    {
        return;
    }
    const bool tab = e.down && k == Key::TAB && !(mods & (MODIFIER_CONTROL | MODIFIER_ALT));
    bool tab_to_widget = false;  // フォーカスのある部品か祖先が Tab を自分で使う(docs/adr/0050)
    for (Widget* w = focus; tab && w && !tab_to_widget; w = WidgetAccess::parent_of(w))
    {
        tab_to_widget = WidgetAccess::handles_tab_key(w);
    }
    if (tab && !tab_to_widget)
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
            return;
        }
    }
    if (tab)
    {
        focus_next(!(mods & MODIFIER_SHIFT));  // 部品が受け取らなかった Tab
        return;
    }
    if (e.down)
    {
        const KeyEvent ev(k, mods, e.repeat, static_cast<std::uint32_t>(e.key));
        trigger_shortcut(ev);
    }
}

// ---- ドラッグ&ドロップ(docs/adr/0047) ----

bool WindowImpl::start_drag(Widget* source, const DragData& data, const std::string& label)
{
    if (drag.active || !grab)
    {
        return false;
    }
    hide_tooltip();
    drag.active = true;
    drag.outside = false;
    drag.data = data;
    DragAccess::set_source(drag.data, source);
    DragAccess::set_outside(drag.data, false);
    drag.source = source;
    drag.label = label;
    drag.target = nullptr;
    drag.action = DropAction::NONE;
    drag.ghost = Rect();
    drag_update(pointer, to_modifiers(SDL_GetModState()));
    return true;
}

void WindowImpl::drag_update(const Point& p, Modifiers mods)
{
    drag.pos = p;
    // ウィンドウの中は移す(Ctrl なら写す)、OS からは写すを提案する
    const DropAction proposed =
        drag.outside ? DropAction::COPY : (mods & MODIFIER_CONTROL) ? DropAction::COPY : DropAction::MOVE;
    Widget* found = nullptr;
    DropAction action = DropAction::NONE;
    for (Widget* w = widget_at(p); w; w = WidgetAccess::parent_of(w))
    {
        if (!WidgetAccess::accepts_drops(w) || !w->effectively_enabled)
        {
            continue;
        }
        DragEvent ev(w->map_from_window(p), drag.data, mods, proposed);
        WidgetAccess::drag_move(w, ev);
        if (!drag.active)
        {
            return;  // ハンドラがドラッグを終わらせた
        }
        if (ev.accepted && ev.action != DropAction::NONE)
        {
            found = w;
            action = ev.action;
            break;
        }
    }
    if (drag.target && drag.target != found)
    {
        Widget* old = drag.target;
        drag.target = nullptr;
        WidgetAccess::drag_leave(old);
    }
    drag.target = found;
    drag.action = action;
    // 像を描き直す
    invalidate(drag.ghost);
    drag.ghost = drag_ghost_rect();
    invalidate(drag.ghost);
}

void WindowImpl::drag_end(bool do_drop, Modifiers mods)
{
    if (!drag.active)
    {
        return;
    }
    DropAction result = DropAction::NONE;
    if (Widget* target = drag.target)
    {
        drag.target = nullptr;
        if (do_drop)
        {
            DragEvent ev(target->map_from_window(drag.pos), drag.data, mods, drag.action);
            WidgetAccess::drop(target, ev);
            result = ev.accepted ? ev.action : DropAction::NONE;
        }
        else
        {
            WidgetAccess::drag_leave(target);
        }
    }
    Widget* source = drag.source;
    drag.active = false;
    drag.source = nullptr;
    drag.data = DragData();
    invalidate(drag.ghost);
    drag.ghost = Rect();
    if (source)
    {
        WidgetAccess::drag_finished(source, result);
    }
}

void WindowImpl::os_drop(const SDL_DropEvent& e)
{
    const Modifiers mods = to_modifiers(SDL_GetModState());
    switch (e.type)
    {
    case SDL_EVENT_DROP_BEGIN:
        if (drag.active)
        {
            return;  // ウィンドウの中のドラッグの途中
        }
        drag.active = true;
        drag.outside = true;
        drag.data = DragData();
        DragAccess::set_outside(drag.data, true);
        drag.source = nullptr;
        drag.label.clear();
        drag.target = nullptr;
        drag.action = DropAction::NONE;
        drag.ghost = Rect();
        break;
    case SDL_EVENT_DROP_POSITION:
        if (drag.active && drag.outside)
        {
            drag_update(to_dp(e.x, e.y), mods);
        }
        break;
    case SDL_EVENT_DROP_FILE:
    case SDL_EVENT_DROP_TEXT:
        if (drag.active && drag.outside && e.data)
        {
            if (e.type == SDL_EVENT_DROP_FILE)
            {
                drag.data.files.push_back(e.data);
            }
            else
            {
                drag.data.text += e.data;
            }
            drag.pos = to_dp(e.x, e.y);
        }
        break;
    case SDL_EVENT_DROP_COMPLETE:
        if (drag.active && drag.outside)
        {
            // 中身が無ければ、落とさずに離れた
            const bool dropped = !drag.data.files.empty() || !drag.data.text.empty();
            if (dropped)
            {
                // 中身が分かったので、受け取るかを聞き直してから落とす
                drag_update(drag.pos, mods);
            }
            drag_end(dropped, mods);
        }
        break;
    default:
        break;
    }
}

Rect WindowImpl::drag_ghost_rect() const
{
    if (!drag.active || drag.outside || drag.label.empty())
    {
        return Rect();
    }
    const TextLayout l(drag.label, Font(), scale);
    return Rect(drag.pos.x + 12, drag.pos.y + 14, std::ceil(l.width()) + 16, std::ceil(l.height()) + 8);
}

void WindowImpl::paint_drag_ghost()
{
    const Rect r = drag_ghost_rect();
    if (r.is_empty())
    {
        return;
    }
    // 受け取る先が無いときは薄く描く
    const float alpha = drag.target ? 1.0f : 0.5f;
    const Color bg = theme::selection();
    canvas.fill_round_rect(r, 4, bg.with_alpha(static_cast<std::uint8_t>(220 * alpha)));
    const TextLayout l(drag.label, Font(), scale);
    const Color fg = theme::on_selection();
    canvas.draw_text(l, Point(r.x + 8, r.y + 4), fg.with_alpha(static_cast<std::uint8_t>(255 * alpha)));
}

bool WindowImpl::trigger_shortcut(const KeyEvent& ev)
{
    // フォーカスのあるウィジェットから親へ、それぞれに加えた Action を探し、最後にメニューバーのメニューを探す。
    for (Widget* w = focus ? focus : q; w; w = WidgetAccess::parent_of(w))
    {
        const std::vector<Action*> list = WidgetAccess::actions_of(w);
        for (std::size_t i = 0; i < list.size(); ++i)
        {
            if (ActionAccess::enabled(list[i]) && ActionAccess::shortcut(list[i]).matches(ev))
            {
                list[i]->trigger();
                return true;
            }
        }
    }
    // ツールバーの Action はウィンドウ全体で効く。
    if (ToolBar* tools = q->tool_bar)
    {
        const std::vector<Action*> list = WidgetAccess::actions_of(tools);
        for (std::size_t i = 0; i < list.size(); ++i)
        {
            if (ActionAccess::enabled(list[i]) && ActionAccess::shortcut(list[i]).matches(ev))
            {
                list[i]->trigger();
                return true;
            }
        }
    }
    if (MenuBar* bar = q->menu_bar)
    {
        return MenuBarAccess::trigger_shortcut(bar, ev);
    }
    return false;
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
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        set_cues(false);
        if (MenuBar* bar = q->menu_bar)
        {
            MenuBarAccess::cancel(bar);
        }
        break;
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
        if (MenuBar* bar = q->menu_bar)
        {
            MenuBarAccess::cancel(bar);
        }
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
    case SDL_EVENT_DROP_BEGIN:
    case SDL_EVENT_DROP_POSITION:
    case SDL_EVENT_DROP_FILE:
    case SDL_EVENT_DROP_TEXT:
    case SDL_EVENT_DROP_COMPLETE:
        os_drop(e.drop);
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

void Window::Impl::init_toplevel(Window* owner, const std::string& title_text, float width_dp, float height_dp)
{
    native_ = nullptr;
    background_ = ColorRole::WINDOW;
    menu_bar_ = nullptr;
    tool_bar_ = nullptr;
    status_bar_ = nullptr;
    Application* app = Application::instance();
    if (!app)
    {
        std::fprintf(stderr, "tokotachi::Window: create tokotachi::Application first\n");
        return;
    }
    // 所有者のウィンドウが無ければ Application が所有する(docs/adr/0062)。
    owner_window_ = owner;
    detail::change_owner(q, owner ? static_cast<Object*>(owner) : static_cast<Object*>(app));
    detail::WidgetAccess::set_is_window(q, true);
    detail::WidgetAccess::set_visible_flag(q, false);
    native_ = new detail::WindowImpl(q);
    native_->window = SDL_CreateWindow(title_text.c_str(), static_cast<int>(width_dp), static_cast<int>(height_dp),
                                       SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
    if (!native_->window)
    {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return;
    }
    native_->renderer = SDL_CreateRenderer(native_->window, nullptr);
    if (!native_->renderer)
    {
        std::fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
    }
    native_->update_scale();
    native_->request_size(Size(width_dp, height_dp));
    detail::impl_of(app)->windows.push_back(native_);
}

Window::Window(const std::string& title_text, float width_dp, float height_dp)
    : TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, std::string, title),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, ThemeColor, background),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Widget*, focused_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, MenuBar*, menu_bar),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, ToolBar*, tool_bar),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, StatusBar*, status_bar),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, CloseHandler, on_close),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, destroy_on_close),
      d_(new Impl(this))
{
    d_->install(this);
    d_->init_toplevel(nullptr, title_text, width_dp, height_dp);
}

Window::Window(Window* owner, const std::string& title_text, float width_dp, float height_dp)
    : TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, std::string, title),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, ThemeColor, background),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Widget*, focused_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, MenuBar*, menu_bar),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, ToolBar*, tool_bar),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, StatusBar*, status_bar),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, CloseHandler, on_close),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, destroy_on_close),
      d_(new Impl(this))
{
    d_->install(this);
    d_->init_toplevel(owner, title_text, width_dp, height_dp);
}

Window::~Window()
{
    // 子のデストラクタは window を通じてこのウィンドウに触るので、Window のうちに破棄する(タイマー・変数なども含む)。
    // 所有するウィンドウ(docs/adr/0062)も、SDL のウィンドウの親子が残らないよう先に破棄する。
    delete_owned();
    if (Application* app = Application::instance())
    {
        detail::impl_of(app)->window_destroyed(this);
        std::vector<detail::WindowImpl*>& ws = detail::impl_of(app)->windows;
        ws.erase(std::remove(ws.begin(), ws.end(), d_->native_), ws.end());
    }
    detail::WidgetAccess::set_is_window(this, false);
    delete d_->native_;
    delete d_;
}

std::string Window::Impl::get_title() const
{
    const char* t = SDL_GetWindowTitle(native_->window);
    return t ? t : "";
}

void Window::Impl::set_title(const std::string& v) { SDL_SetWindowTitle(native_->window, v.c_str()); }

void Window::Impl::set_background(const ThemeColor& v)
{
    background_ = v;
    q->invalidate();
}

Widget* Window::Impl::get_focused_widget() const { return native_->focus; }

Window::Window(Object* owner_object)
    : TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, std::string, title),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, ThemeColor, background),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Widget*, focused_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, MenuBar*, menu_bar),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, ToolBar*, tool_bar),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, StatusBar*, status_bar),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, CloseHandler, on_close),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, destroy_on_close),
      d_(new Impl(this))
{
    d_->install(this);
    d_->native_ = nullptr;
    d_->background_ = ColorRole::POPUP;
    d_->menu_bar_ = nullptr;
    d_->tool_bar_ = nullptr;
    d_->status_bar_ = nullptr;
    detail::WidgetAccess::set_is_window(this, true);
    detail::WidgetAccess::set_visible_flag(this, false);
    detail::change_owner(this, owner_object);
    d_->native_ = new detail::WindowImpl(this);
    d_->native_->popup = true;
    if (Application* app = Application::instance())
    {
        detail::impl_of(app)->windows.push_back(d_->native_);
    }
}

void Window::close()
{
    visible = false;
    if (Application* app = Application::instance())
    {
        detail::impl_of(app)->quit_if_done();
    }
    if (d_->destroy_on_close_)
    {
        // ハンドラ(Dialog の on_finish など)を呼び終えてから破棄する。
        delete_later();
    }
}

bool Window::save_png(const std::string& path)
{
    d_->native_->invalidate_all();
    d_->native_->render(false);
    SDL_Surface* s = SDL_CreateSurfaceFrom(d_->native_->pixel_width, d_->native_->pixel_height, SDL_PIXELFORMAT_ARGB8888,
                                           &d_->native_->buffer[0], d_->native_->pixel_width * 4);
    const bool ok = s && SDL_SavePNG(s, path.c_str());
    if (!ok)
    {
        std::fprintf(stderr, "save_png: %s\n", SDL_GetError());
    }
    SDL_DestroySurface(s);
    return ok;
}

Bitmap Window::capture()
{
    d_->native_->invalidate_all();
    d_->native_->render(false);
    Bitmap b(d_->native_->pixel_width, d_->native_->pixel_height, d_->native_->scale);
    std::uint32_t* out = b.pixels();
    // ウィンドウの描画先は不透明(ARGB8888)。Bitmap はアルファ乗算済みなので、アルファを 255 にして写す
    for (std::size_t i = 0; i < d_->native_->buffer.size(); ++i)
    {
        out[i] = d_->native_->buffer[i] | 0xff000000u;
    }
    return b;
}

namespace
{

// 上端・下端の専用の場所の部品を差し替える。前のものは破棄する。
template <typename T>
void replace_bar(Window* self, T*& slot, T* v)
{
    if (v == slot)
    {
        return;
    }
    T* old = slot;
    slot = v;
    if (v)
    {
        v->parent = self;
        if (v->parent.get() != self)
        {
            slot = nullptr;  // 子にできなかった
        }
    }
    delete old;  // child_removed で slot を変えないよう、付け替えた後に破棄する
    detail::WidgetAccess::mark_layout_dirty(self);
    self->request_layout();
}

}  // namespace

void Window::Impl::set_menu_bar(MenuBar* const& v) { replace_bar(q, menu_bar_, v); }

void Window::Impl::set_tool_bar(ToolBar* const& v) { replace_bar(q, tool_bar_, v); }

void Window::Impl::set_status_bar(StatusBar* const& v) { replace_bar(q, status_bar_, v); }

void Window::Impl::child_removed(Widget* child)
{
    bool removed = true;
    if (child == menu_bar_)
    {
        menu_bar_ = nullptr;
        q->notify_changed(q->menu_bar);
    }
    else if (child == tool_bar_)
    {
        tool_bar_ = nullptr;
        q->notify_changed(q->tool_bar);
    }
    else if (child == status_bar_)
    {
        status_bar_ = nullptr;
        q->notify_changed(q->status_bar);
    }
    else
    {
        removed = false;
    }
    if (removed)
    {
        detail::WidgetAccess::mark_layout_dirty(q);
        q->request_layout();
    }
}

void Window::arrange_children()
{
    // 上端にメニューバー・ツールバー、下端にステータスバーを置き、レイアウトはその間に配置する。
    Rect area = local_rect;
    Widget* const top[] = {d_->menu_bar_, d_->tool_bar_};
    for (int i = 0; i < 2; ++i)
    {
        if (top[i])
        {
            const float h = top[i]->preferred_size.get().height;
            top[i]->bounds = Rect(0, area.y, area.width, h);
            area = area.adjusted(0, h, 0, 0);
        }
    }
    if (d_->status_bar_)
    {
        const float h = d_->status_bar_->preferred_size.get().height;
        d_->status_bar_->bounds = Rect(0, area.bottom() - h, area.width, h);
        area = area.adjusted(0, 0, 0, h);
    }
    if (Layout* l = layout)
    {
        detail::LayoutAccess::arrange(l, area);
    }
}

Size Window::compute_preferred_size() const
{
    Size s = Widget::compute_preferred_size();
    Widget* const bars[] = {d_->menu_bar_, d_->tool_bar_, d_->status_bar_};
    for (int i = 0; i < 3; ++i)
    {
        if (bars[i])
        {
            const Size b = bars[i]->preferred_size;
            s = Size(std::max(s.width, bars[i] == d_->menu_bar_ ? b.width : 0.0f), s.height + b.height);
        }
    }
    return s;
}

Size Window::compute_minimum_size() const
{
    Size s = Widget::compute_minimum_size();
    Widget* const bars[] = {d_->menu_bar_, d_->tool_bar_, d_->status_bar_};
    for (int i = 0; i < 3; ++i)
    {
        if (bars[i])
        {
            const Size b = bars[i]->minimum_hint;
            s = Size(std::max(s.width, b.width), s.height + b.height);
        }
    }
    return s;
}

void Window::paint(Canvas& canvas)
{
    canvas.fill_rect(local_rect, d_->background_.resolve());
    Widget::paint(canvas);
}

}  // namespace tokotachi
