#include <gui/windows/menu_bar.hpp>

#include <algorithm>
#include <cmath>

#include <gui/graphics/canvas.hpp>
#include <gui/graphics/text_layout.hpp>

#include "../actions/action_access.hpp"
#include "../core/property_impl.hpp"
#include "../actions/mnemonic.hpp"
#include "../widgets/theme.hpp"

namespace gui
{

namespace
{
const float TITLE_PADDING_X = 10;  // 見出しの文字の左右の余白(dp)
const float TITLE_PADDING_Y = 6;   // 上下の余白
const float BAR_PADDING_X = 4;     // バーの左端の余白

// menu とサブメニューの Action から、e に一致するものを探す。
Action* find_shortcut(const Menu* menu, const KeyEvent& e)
{
    const std::vector<Action*> actions = menu->actions;
    for (std::size_t i = 0; i < actions.size(); ++i)
    {
        if (detail::ActionAccess::enabled(actions[i]) && detail::ActionAccess::shortcut(actions[i]).matches(e))
        {
            return actions[i];
        }
    }
    return nullptr;
}
}  // namespace

MenuBar::MenuBar(Widget* parent_widget)
    : Widget(parent_widget),
      GUI_READONLY_PROPERTY(MenuBar, std::vector<Menu*>, menus),
      open_(-1),
      hover_(-1),
      active_(-1),
      alt_alone_(false),
      cues_(false)
{
}

MenuBar::~MenuBar()
{
    // メニュー(所有しているオブジェクト)は、自分がまだ MenuBar であるうちに破棄する(閉じたことを知らせてくるため)。
    std::vector<Menu*> menus_copy;
    menus_copy.swap(menus_);
    for (std::size_t i = 0; i < menus_copy.size(); ++i)
    {
        menus_copy[i]->bar_ = nullptr;
        delete menus_copy[i];
    }
}

Menu* MenuBar::add_menu(const std::string& title)
{
    Menu* menu = new Menu(this, title);
    menu->bar_ = this;
    menus_.push_back(menu);
    notify_changed(menus);
    request_layout();
    invalidate();
    return menu;
}

void MenuBar::remove_menu(Menu* menu)
{
    const int i = index_of(menu);
    if (i < 0)
    {
        return;
    }
    menu->close();
    menus_.erase(menus_.begin() + i);
    menu->bar_ = nullptr;
    delete menu;
    open_ = -1;
    hover_ = -1;
    active_ = -1;
    notify_changed(menus);
    request_layout();
    invalidate();
}

int MenuBar::index_of(const Menu* menu) const
{
    for (std::size_t i = 0; i < menus_.size(); ++i)
    {
        if (menus_[i] == menu)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int MenuBar::mnemonic_at(Key key) const
{
    if (key == Key::UNKNOWN)
    {
        return -1;
    }
    for (std::size_t i = 0; i < menus_.size(); ++i)
    {
        if (detail::parse_mnemonic(menus_[i]->title_).key == key)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// ---- 寸法 ----

float MenuBar::bar_height() const
{
    return std::ceil(TextLayout("Ag", Font(), display_scale).height()) + TITLE_PADDING_Y * 2;
}

Rect MenuBar::title_rect(int i) const
{
    float x = BAR_PADDING_X;
    const float scale = display_scale;
    for (int k = 0; k < static_cast<int>(menus_.size()); ++k)
    {
        const float w = std::ceil(TextLayout(detail::parse_mnemonic(menus_[k]->title_).text, Font(), scale).width()) +
                        TITLE_PADDING_X * 2;
        if (k == i)
        {
            return Rect(x, 0, w, height);
        }
        x += w;
    }
    return Rect();
}

int MenuBar::title_at(const Point& p) const
{
    for (int i = 0; i < static_cast<int>(menus_.size()); ++i)
    {
        if (title_rect(i).contains(p))
        {
            return i;
        }
    }
    return -1;
}

Size MenuBar::compute_preferred_size() const
{
    const Rect last = menus_.empty() ? Rect() : title_rect(static_cast<int>(menus_.size()) - 1);
    return Size(last.right() + BAR_PADDING_X, bar_height());
}

Size MenuBar::compute_minimum_size() const { return Size(0, bar_height()); }

// ---- 描画とマウス ----

void MenuBar::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    canvas.fill_rect(r, theme::window());
    canvas.fill_rect(Rect(0, r.height - 1, r.width, 1), theme::border());
    const float scale = display_scale;
    const bool enabled_now = effectively_enabled;
    for (int i = 0; i < static_cast<int>(menus_.size()); ++i)
    {
        const Rect t = title_rect(i);
        const bool hot = i == open_ || i == active_ || (open_ < 0 && active_ < 0 && i == hover_);
        if (hot && enabled_now)
        {
            canvas.fill_round_rect(t.adjusted(0, 3, 0, 4), theme::field_radius(), theme::selection_inactive());
        }
        const detail::Mnemonic m = detail::parse_mnemonic(menus_[i]->title_);
        const TextLayout l(m.text, Font(), scale);
        const Point at(t.x + TITLE_PADDING_X, (r.height - l.height()) / 2);
        const Color fg = enabled_now ? theme::text() : theme::text_disabled();
        canvas.draw_text(l, at, fg);
        if (cues_)
        {
            detail::paint_mnemonic_underline(canvas, l, at, m.offset, fg);
        }
    }
    Widget::paint(canvas);
}

void MenuBar::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    e.accepted = true;
    // メニューを開いている間にバーを押すと、Application がメニューを閉じて押したことを捨てるので、ここへは来ない。
    const int i = title_at(e.pos);
    if (i >= 0)
    {
        open(i, false);
    }
}

void MenuBar::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    const int i = title_at(e.pos);
    if (i != hover_)
    {
        hover_ = i;
        invalidate();
    }
    // 開いている間は、ポインタが乗った見出しのメニューに切り替える。
    if (open_ >= 0 && i >= 0 && i != open_)
    {
        open(i, false);
    }
}

void MenuBar::mouse_leave()
{
    if (hover_ != -1)
    {
        hover_ = -1;
        invalidate();
    }
    Widget::mouse_leave();
}

// ---- メニューの開閉 ----

void MenuBar::open(int i, bool keyboard)
{
    if (i < 0 || i >= static_cast<int>(menus_.size()))
    {
        return;
    }
    if (open_ >= 0 && open_ != i)
    {
        Menu* old = menus_[open_];
        open_ = -1;
        old->close();
    }
    active_ = -1;
    cues_ = keyboard;
    open_ = i;
    invalidate();
    menus_[i]->open_below(this, title_rect(i), keyboard);
    if (!menus_[i]->is_open())
    {
        open_ = -1;  // 開けなかった(ウィンドウが表示されていないなど)
    }
}

void MenuBar::open_adjacent(Menu* from, int step)
{
    const int n = static_cast<int>(menus_.size());
    const int i = index_of(from);
    if (i < 0 || n == 0)
    {
        return;
    }
    open((i + step + n) % n, true);
}

void MenuBar::menu_closed(Menu* menu)
{
    if (index_of(menu) == open_)
    {
        open_ = -1;
        if (active_ < 0)
        {
            cues_ = false;
        }
        invalidate();
    }
}

void MenuBar::menu_escaped(Menu* menu)
{
    const int i = index_of(menu);
    menu->close();
    enter_keyboard_mode(i);
}

void MenuBar::menu_activated() { leave_keyboard_mode(); }

void MenuBar::enter_keyboard_mode(int i)
{
    if (menus_.empty())
    {
        return;
    }
    active_ = std::max(0, std::min(i, static_cast<int>(menus_.size()) - 1));
    cues_ = true;
    invalidate();
}

void MenuBar::leave_keyboard_mode()
{
    if (active_ >= 0 || cues_)
    {
        active_ = -1;
        cues_ = false;
        invalidate();
    }
}

void MenuBar::cancel()
{
    alt_alone_ = false;
    leave_keyboard_mode();
}

// ---- キー ----

bool MenuBar::handle_key(const KeyEvent& e, bool down)
{
    if (menus_.empty() || !showing.get() || !effectively_enabled.get())
    {
        return false;
    }
    if (!down)
    {
        if (e.key == Key::ALT)
        {
            const bool toggle = alt_alone_;
            alt_alone_ = false;
            if (toggle)
            {
                // Alt を押して離した: メニューバーへ移る(移っていれば戻る)。
                if (active_ >= 0)
                {
                    leave_keyboard_mode();
                }
                else
                {
                    enter_keyboard_mode(0);
                }
                return true;
            }
            if (active_ < 0 && cues_)
            {
                cues_ = false;
                invalidate();
            }
        }
        return false;
    }

    if (e.key == Key::ALT)
    {
        if (!e.repeat)
        {
            alt_alone_ = true;
            if (!cues_)
            {
                cues_ = true;  // 押している間は下線を出す
                invalidate();
            }
        }
        return false;
    }
    alt_alone_ = false;

    const bool other_mods = (e.modifiers & (MODIFIER_CONTROL | MODIFIER_SHIFT | MODIFIER_META)) != 0;
    if (active_ >= 0)
    {
        // メニューバーに移っている: ←→ で選び、↓・Enter・Space で開く。
        const int n = static_cast<int>(menus_.size());
        switch (e.key)
        {
        case Key::LEFT:
            active_ = (active_ - 1 + n) % n;
            invalidate();
            return true;
        case Key::RIGHT:
            active_ = (active_ + 1) % n;
            invalidate();
            return true;
        case Key::DOWN:
        case Key::UP:
        case Key::RETURN:
        case Key::SPACE:
            open(active_, true);
            return true;
        case Key::ESCAPE:
        case Key::F10:
            leave_keyboard_mode();
            return true;
        default:
        {
            const int i = other_mods ? -1 : mnemonic_at(e.key);
            if (i >= 0)
            {
                open(i, true);
                return true;
            }
            leave_keyboard_mode();
            return false;  // 他のキーは普段どおりに扱う
        }
        }
    }
    if (e.has_modifier(MODIFIER_ALT) && !other_mods)
    {
        const int i = mnemonic_at(e.key);
        if (i >= 0)
        {
            open(i, true);
            return true;
        }
    }
    if (e.key == Key::F10 && e.modifiers == MODIFIER_NONE)
    {
        enter_keyboard_mode(0);
        return true;
    }
    return false;
}

bool MenuBar::trigger_shortcut(const KeyEvent& e)
{
    if (!showing.get() || !effectively_enabled.get())
    {
        return false;
    }
    std::vector<const Menu*> stack(menus_.begin(), menus_.end());
    while (!stack.empty())
    {
        const Menu* m = stack.back();
        stack.pop_back();
        if (Action* a = find_shortcut(m, e))
        {
            a->trigger();
            return true;
        }
        for (std::size_t i = 0; i < m->entries_.size(); ++i)
        {
            if (m->entries_[i].submenu)
            {
                stack.push_back(m->entries_[i].submenu);
            }
        }
    }
    return false;
}

}  // namespace gui
