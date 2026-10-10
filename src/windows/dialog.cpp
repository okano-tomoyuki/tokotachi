#include <tokotachi/windows/dialog.hpp>

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include <tokotachi/core/application.hpp>
#include <tokotachi/layout/layout.hpp>

#include "../core/application_impl.hpp"
#include "../core/property_impl.hpp"
#include "../widgets/widget_access.hpp"
#include "window_impl.hpp"
#include "../layout/layout_access.hpp"

namespace tokotachi
{

namespace
{
const float MARGIN = 16;          // ボタンの行の下と左右の余白(dp)
const float BUTTON_SPACING = 8;   // ボタンの間
const float BUTTON_MIN_WIDTH = 88;
const float MIN_WIDTH = 320;      // 開くときの幅の範囲
const float MAX_WIDTH = 560;

enum class Role
{
    ACCEPT,
    DESTRUCTIVE,
    REJECT,
    NONE
};

Role role_of(DialogButton b)
{
    switch (b)
    {
    case DialogButton::OK:
    case DialogButton::YES:
    case DialogButton::SAVE:
        return Role::ACCEPT;
    case DialogButton::NO:
    case DialogButton::DISCARD:
        return Role::DESTRUCTIVE;
    case DialogButton::CANCEL:
    case DialogButton::CLOSE:
        return Role::REJECT;
    default:
        return Role::NONE;
    }
}

// 決定のボタンを先に並べるか(Windows・KDE)。GNOME などは最後に並べる。
bool accept_first()
{
#ifdef _WIN32
    return true;
#else
    static const bool kde = []()
    {
        const char* desktop = std::getenv("XDG_CURRENT_DESKTOP");
        return desktop && std::strstr(desktop, "KDE") != nullptr;
    }();
    return kde;
#endif
}

int rank(DialogButton b)
{
    const Role r = role_of(b);
    if (accept_first())
    {
        return r == Role::ACCEPT ? 0 : r == Role::DESTRUCTIVE ? 1 : 2;
    }
    return r == Role::DESTRUCTIVE ? 0 : r == Role::REJECT ? 1 : 2;
}

// 利用者の言語が日本語か(標準のボタンの文字を選ぶ)。
bool japanese()
{
    static const bool ja = []()
    {
        int count = 0;
        SDL_Locale** locales = SDL_GetPreferredLocales(&count);
        bool result = false;
        if (locales && count > 0 && locales[0] && locales[0]->language)
        {
            result = std::strcmp(locales[0]->language, "ja") == 0;
        }
        SDL_free(locales);
        return result;
    }();
    return ja;
}

std::string standard_text(DialogButton b)
{
    const bool ja = japanese();
    switch (b)
    {
    case DialogButton::OK:
        return "OK";
    case DialogButton::YES:
        return ja ? "はい" : "Yes";
    case DialogButton::SAVE:
        return ja ? "保存" : "Save";
    case DialogButton::NO:
        return ja ? "いいえ" : "No";
    case DialogButton::DISCARD:
        return ja ? "保存しない" : "Don't Save";
    case DialogButton::CANCEL:
        return ja ? "キャンセル" : "Cancel";
    case DialogButton::CLOSE:
        return ja ? "閉じる" : "Close";
    default:
        return std::string();
    }
}

}  // namespace

Dialog::Dialog(Window* parent, const std::string& title_text)
    : Window(title_text, 400, 200),
      TOKOTACHI_READONLY_PROPERTY(Dialog, DialogButton, result),
      TOKOTACHI_HANDLER_PROPERTY(Dialog, NotifyHandler, on_finish),
      parent_window_(parent),
      result_(DialogButton::NONE),
      running_(false)
{
    change_owner(parent);
}

Dialog::~Dialog()
{
    if (running_)
    {
        // 開いたまま破棄された: 親への入力を止めるのをやめる(WindowImpl の破棄でも外れる)。
        running_ = false;
        if (Application* app = Application::instance())
        {
            std::vector<detail::WindowImpl*>& modals = app->impl()->modal_dialogs;
            modals.erase(std::remove(modals.begin(), modals.end(), impl()), modals.end());
        }
    }
    buttons_.clear();
}

Button* Dialog::add_button(DialogButton button, const std::string& text_value)
{
    Button* b = new Button(this, text_value.empty() ? standard_text(button) : text_value);
    b->style = role_of(button) == Role::ACCEPT ? ButtonStyle::PRIMARY : ButtonStyle::SECONDARY;
    b->focus_policy = FocusPolicy::TAB_AND_CLICK;
    b->on_click = [this, button](Object*) { finish(button); };
    buttons_.push_back(std::make_pair(button, b));
    request_layout();
    return b;
}

void Dialog::child_removed(Widget* child)
{
    for (std::size_t i = 0; i < buttons_.size(); ++i)
    {
        if (buttons_[i].second == child)
        {
            buttons_.erase(buttons_.begin() + static_cast<std::ptrdiff_t>(i));
            request_layout();
            break;
        }
    }
    Window::child_removed(child);
}

std::vector<std::pair<DialogButton, Button*> > Dialog::ordered_buttons() const
{
    std::vector<std::pair<DialogButton, Button*> > out = buttons_;
    std::stable_sort(out.begin(), out.end(),
                     [](const std::pair<DialogButton, Button*>& a, const std::pair<DialogButton, Button*>& b)
                     { return rank(a.first) < rank(b.first); });
    return out;
}

Button* Dialog::button_for(DialogButton role) const
{
    for (std::size_t i = 0; i < buttons_.size(); ++i)
    {
        if (buttons_[i].first == role)
        {
            return buttons_[i].second;
        }
    }
    return nullptr;
}

DialogButton Dialog::cancel_result() const
{
    for (std::size_t i = 0; i < buttons_.size(); ++i)
    {
        if (role_of(buttons_[i].first) == Role::REJECT)
        {
            return buttons_[i].first;
        }
    }
    // ボタンが 1 つだけ(「OK」だけのお知らせなど)なら、それで閉じる。
    return buttons_.size() == 1 ? buttons_[0].first : DialogButton::NONE;
}

// ---- 配置 ----

float Dialog::button_row_height() const
{
    float h = 0;
    for (std::size_t i = 0; i < buttons_.size(); ++i)
    {
        if (buttons_[i].second->visible)
        {
            h = std::max(h, buttons_[i].second->preferred_size.get().height);
        }
    }
    return h;
}

float Dialog::button_row_width() const
{
    float w = 0;
    int n = 0;
    for (std::size_t i = 0; i < buttons_.size(); ++i)
    {
        if (buttons_[i].second->visible)
        {
            w += std::max(BUTTON_MIN_WIDTH, buttons_[i].second->preferred_size.get().width);
            ++n;
        }
    }
    return n > 0 ? w + BUTTON_SPACING * static_cast<float>(n - 1) + MARGIN * 2 : 0;
}

void Dialog::arrange_children()
{
    // ボタンを下端に右寄せで並べ、レイアウト(中身)はその上に配置する。
    const Rect area = local_rect;
    const float row_h = button_row_height();
    Rect content = area;
    if (row_h > 0)
    {
        const float y = area.height - MARGIN - row_h;
        const std::vector<std::pair<DialogButton, Button*> > ordered = ordered_buttons();
        float x = area.width - MARGIN;
        for (std::size_t i = ordered.size(); i-- > 0;)
        {
            Button* b = ordered[i].second;
            if (!b->visible)
            {
                continue;
            }
            const float w = std::max(BUTTON_MIN_WIDTH, b->preferred_size.get().width);
            x -= w;
            b->bounds = Rect(x, y, w, row_h);
            x -= BUTTON_SPACING;
        }
        content = Rect(0, 0, area.width, std::max(0.0f, y));
    }
    if (Layout* l = layout)
    {
        detail::LayoutAccess::arrange(l, content);
    }
}

Size Dialog::compute_preferred_size() const
{
    const Size content = Window::compute_preferred_size();
    const float row_h = button_row_height();
    return Size(std::max(content.width, button_row_width()), content.height + (row_h > 0 ? row_h + MARGIN : 0));
}

Size Dialog::compute_minimum_size() const
{
    const Size content = Window::compute_minimum_size();
    const float row_h = button_row_height();
    return Size(std::max(content.width, button_row_width()), content.height + (row_h > 0 ? row_h + MARGIN : 0));
}

// ---- 開閉 ----

void Dialog::open()
{
    detail::WindowImpl* d = impl();
    if (running_ || !d || !d->window)
    {
        return;
    }
    Application* app = Application::instance();
    detail::WindowImpl* parent_impl = parent_window_ ? parent_window_->impl() : nullptr;
    if (app)
    {
        app->impl()->close_menus();
    }
    // 大きさを中身に合わせる: 幅を決めてから配置し(折り返す文字の高さが決まる)、高さを決める。
    // OS のウィンドウの大きさは最後に 1 回だけ変える。
    const Size wanted = preferred_size;
    const float w = std::max(minimum_size.get().width, std::max(MIN_WIDTH, std::min(MAX_WIDTH, wanted.width)));
    float h = std::max(height.get(), 1.0f);
    for (int i = 0; i < 3; ++i)
    {
        detail::WidgetAccess::set_bounds_raw(this, Rect(0, 0, w, h));
        detail::WidgetAccess::mark_layout_dirty(this);
        apply_layout();
        h = std::max(minimum_hint.get().height, preferred_size.get().height);
    }
    int current_w = 0, current_h = 0;
    SDL_GetWindowSize(d->window, &current_w, &current_h);
    const float k = d->scale / d->density;
    const bool size_changes = current_w != static_cast<int>(std::floor(w * k + 0.5f)) ||
                              current_h != static_cast<int>(std::floor(h * k + 0.5f));
    // WSLg(Wayland)では、隠れたウィンドウの大きさを変えてから見せると、最初の表示が崩れる(枠と中身がずれる)。
    // 大きさが変わるときは、その大きさで作り直す。
    if (!(size_changes && detail::is_wayland() && d->recreate_toplevel(Size(w, h))))
    {
        d->request_size(Size(w, h));
    }
    apply_layout();
    if (parent_impl && parent_impl->window)
    {
        SDL_SetWindowParent(d->window, parent_impl->window);
        SDL_SetWindowModal(d->window, true);
        d->modal_parent = parent_impl;
    }
    // 親の中央に置く(Wayland では OS が決める)。
    if (parent_impl && parent_impl->window)
    {
        int px = 0, py = 0, pw = 0, ph = 0, dw = 0, dh = 0;
        SDL_GetWindowPosition(parent_impl->window, &px, &py);
        SDL_GetWindowSize(parent_impl->window, &pw, &ph);
        SDL_GetWindowSize(d->window, &dw, &dh);
        SDL_SetWindowPosition(d->window, px + (pw - dw) / 2, py + (ph - dh) / 2);
    }

    result_ = DialogButton::NONE;
    running_ = true;
    if (app)
    {
        app->impl()->modal_dialogs.push_back(d);
    }
    visible = true;
    // フォーカスが無ければ、最初のフォーカスを受けるウィジェット(無ければ決定のボタン)へ。
    if (!focused_widget.get())
    {
        d->focus_next(true);
    }
}

void Dialog::finish(DialogButton r)
{
    if (!running_)
    {
        return;
    }
    running_ = false;
    result_ = r;
    detail::WindowImpl* d = impl();
    if (Application* app = Application::instance())
    {
        std::vector<detail::WindowImpl*>& modals = app->impl()->modal_dialogs;
        modals.erase(std::remove(modals.begin(), modals.end(), d), modals.end());
    }
    if (d && d->window && d->modal_parent)
    {
        SDL_SetWindowModal(d->window, false);
    }
    detail::WindowImpl* parent_impl = d ? d->modal_parent : nullptr;
    if (d)
    {
        d->modal_parent = nullptr;
    }
    Window::close();
    if (parent_impl && parent_impl->window)
    {
        SDL_RaiseWindow(parent_impl->window);
    }
    notify_changed(result);
    // ハンドラの中でダイアログが破棄されてもよいよう、最後に呼ぶ。
    Object* self = this;
    detail::fire(on_finish_, self);
}

void Dialog::close()
{
    if (running_)
    {
        finish(cancel_result());
        return;
    }
    Window::close();
}

void Dialog::key_down(KeyEvent& e)
{
    Window::key_down(e);
    if (e.modifiers & (MODIFIER_CONTROL | MODIFIER_ALT | MODIFIER_META))
    {
        return;
    }
    if (e.key == Key::ESCAPE)
    {
        const DialogButton cancel = cancel_result();
        if (cancel != DialogButton::NONE || buttons_.empty())
        {
            e.accepted = true;
            finish(cancel);
        }
        return;
    }
    if (e.key == Key::RETURN)
    {
        // 最初の決定のボタン(並べた順)を押す。
        const std::vector<std::pair<DialogButton, Button*> > ordered = ordered_buttons();
        for (std::size_t i = 0; i < ordered.size(); ++i)
        {
            if (role_of(ordered[i].first) == Role::ACCEPT && ordered[i].second->effectively_enabled.get())
            {
                e.accepted = true;
                finish(ordered[i].first);
                return;
            }
        }
    }
}

}  // namespace tokotachi
