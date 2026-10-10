#include <tokotachi/actions/menu.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/windows/menu_bar.hpp>
#include <tokotachi/windows/popup.hpp>
#include <tokotachi/graphics/text_layout.hpp>
#include <tokotachi/widgets/widget.hpp>

#include "action_access.hpp"
#include "../core/property_impl.hpp"
#include "menu_access.hpp"
#include "mnemonic.hpp"
#include "../widgets/theme.hpp"

namespace tokotachi
{

namespace detail
{

// メニューの項目を並べて描き、操作を受ける(内部)。Menu のポップアップの中に 1 つ置く。
class MenuView : public Widget
{
public:
    MenuView(Widget* parent_widget, Menu* owner_menu)
        : Widget(parent_widget), menu(owner_menu), highlighted(-1), open_child(nullptr)
    {
        focus_policy = FocusPolicy::CLICK;
    }

    Menu* menu;
    int highlighted;
    Menu* open_child;  // 開いているサブメニュー

    static const float PADDING;
    static const float GAP;
    static const float ARROW;
    static const float CHECK;
    static const float SEPARATOR;

    float item_height() const { return std::ceil(TextLayout("", Font(), display_scale).height()) + 10; }

    bool is_separator(std::size_t i) const { return !menu->entries_[i].action && !menu->entries_[i].submenu; }

    float height_of(std::size_t i) const { return is_separator(i) ? SEPARATOR : item_height(); }

    // 項目の表示の文字(& を含む)。
    std::string label_of(std::size_t i) const
    {
        const Menu::Entry& e = menu->entries_[i];
        return e.submenu ? e.submenu->title_ : e.action ? ActionAccess::text(e.action) : std::string();
    }

    std::string shortcut_of(std::size_t i) const
    {
        const Menu::Entry& e = menu->entries_[i];
        return e.action ? ActionAccess::shortcut(e.action).to_string() : std::string();
    }

    bool has_checks() const
    {
        for (std::size_t i = 0; i < menu->entries_.size(); ++i)
        {
            if (menu->entries_[i].action && ActionAccess::checkable(menu->entries_[i].action))
            {
                return true;
            }
        }
        return false;
    }

    Rect row_rect(std::size_t i) const
    {
        float y = 4;
        for (std::size_t k = 0; k < i; ++k)
        {
            y += height_of(k);
        }
        return Rect(0, y, width, height_of(i));
    }

    int row_at(float y) const
    {
        float top = 4;
        for (std::size_t i = 0; i < menu->entries_.size(); ++i)
        {
            const float h = height_of(i);
            if (y >= top && y < top + h)
            {
                return static_cast<int>(i);
            }
            top += h;
        }
        return -1;
    }

    bool selectable(int i) const
    {
        if (i < 0 || i >= static_cast<int>(menu->entries_.size()))
        {
            return false;
        }
        const Menu::Entry& e = menu->entries_[static_cast<std::size_t>(i)];
        return e.submenu || (e.action && ActionAccess::enabled(e.action));
    }

    void highlight(int i)
    {
        if (i != highlighted)
        {
            highlighted = i;
            invalidate();
        }
    }

    // 区切りと無効な項目を飛ばして、step の向きに次の項目へ。
    void step_highlight(int step)
    {
        const int n = static_cast<int>(menu->entries_.size());
        // 何も強調していなければ、↓ は最初から、↑ は最後から探す。
        int i = highlighted >= 0 ? highlighted : (step > 0 ? -1 : n);
        for (int k = 0; k < n; ++k)
        {
            i = (i + step + n) % n;
            if (selectable(i))
            {
                highlight(i);
                return;
            }
        }
    }

    void close_child()
    {
        if (open_child)
        {
            open_child->close();
            open_child = nullptr;
        }
    }

    // 項目 i のサブメニューを開く(keyboard なら最初の項目を強調し、キーボードの操作をサブメニューへ移す)。
    void open_submenu(int i, bool keyboard)
    {
        Menu* sub = menu->entries_[static_cast<std::size_t>(i)].submenu;
        if (!sub || sub == open_child)
        {
            return;
        }
        close_child();
        open_child = sub;
        sub->open_beside(this, row_rect(static_cast<std::size_t>(i)));
        if (keyboard && sub->view_)
        {
            sub->view_->highlighted = -1;
            sub->view_->step_highlight(1);
        }
    }

    void choose(int i)
    {
        if (!selectable(i))
        {
            return;
        }
        const Menu::Entry& e = menu->entries_[static_cast<std::size_t>(i)];
        if (e.submenu)
        {
            open_submenu(i, true);
        }
        else
        {
            menu->activate(e.action);  // ハンドラの中でメニューが破棄されてもよいよう、何もせずに戻る
        }
    }

    // ニーモニックのキーの項目を選ぶ(同じキーの項目が複数あれば、順に強調するだけ)。使ったら true。
    bool choose_mnemonic(Key key)
    {
        std::vector<int> matches;
        for (std::size_t i = 0; i < menu->entries_.size(); ++i)
        {
            if (selectable(static_cast<int>(i)) && parse_mnemonic(label_of(i)).key == key)
            {
                matches.push_back(static_cast<int>(i));
            }
        }
        if (matches.empty())
        {
            return false;
        }
        if (matches.size() == 1)
        {
            highlight(matches[0]);
            choose(matches[0]);
            return true;
        }
        int next = matches[0];
        for (std::size_t k = 0; k < matches.size(); ++k)
        {
            if (matches[k] > highlighted)
            {
                next = matches[k];
                break;
            }
        }
        highlight(next);
        return true;
    }

protected:
    Size compute_preferred_size() const override
    {
        const float scale = display_scale;
        float text_w = 0, shortcut_w = 0, h = 8;
        bool has_submenu = false;
        for (std::size_t i = 0; i < menu->entries_.size(); ++i)
        {
            h += height_of(i);
            if (is_separator(i))
            {
                continue;
            }
            text_w = std::max(text_w, TextLayout(parse_mnemonic(label_of(i)).text, Font(), scale).width());
            shortcut_w = std::max(shortcut_w, TextLayout(shortcut_of(i), Font(), scale).width());
            has_submenu = has_submenu || menu->entries_[i].submenu;
        }
        const float w = PADDING * 2 + (has_checks() ? CHECK : 0) + text_w + (shortcut_w > 0 ? GAP + shortcut_w : 0) +
                        (has_submenu ? ARROW : 0);
        return Size(std::ceil(std::max(w, 120.0f)), std::ceil(h));
    }

    void paint(Canvas& canvas) override
    {
        const float scale = display_scale;
        const float w = width;
        const float text_x = PADDING + (has_checks() ? CHECK : 0);
        const bool cues = menu->root()->cues_;
        for (std::size_t i = 0; i < menu->entries_.size(); ++i)
        {
            const Rect row = row_rect(i);
            if (is_separator(i))
            {
                canvas.fill_rect(Rect(PADDING / 2, row.y + row.height / 2, w - PADDING, 1), theme::border());
                continue;
            }
            const Menu::Entry& e = menu->entries_[i];
            const bool enabled_now = selectable(static_cast<int>(i));
            const bool hot = static_cast<int>(i) == highlighted && enabled_now;
            if (hot)
            {
                canvas.fill_round_rect(row.adjusted(4, 0, 4, 0), 4, theme::selection());
            }
            const Color fg = !enabled_now ? theme::text_disabled() : hot ? theme::on_selection() : theme::text();
            if (e.action && ActionAccess::checkable(e.action) && ActionAccess::checked(e.action))
            {
                // チェックの印
                const float cx = PADDING + CHECK / 2 - 2, cy = row.y + row.height / 2;
                Path mark;
                mark.move_to(cx - 5, cy);
                mark.line_to(cx - 1.5f, cy + 3.5f);
                mark.line_to(cx + 5, cy - 4);
                canvas.stroke_path(mark, 1.8f, fg, LineCap::ROUND);
            }
            const Mnemonic m = parse_mnemonic(label_of(i));
            const TextLayout text(m.text, Font(), scale);
            const Point at(text_x, row.y + (row.height - text.height()) / 2);
            canvas.draw_text(text, at, fg);
            if (cues)
            {
                paint_mnemonic_underline(canvas, text, at, m.offset, fg);
            }
            const std::string sc_text = shortcut_of(i);
            if (!sc_text.empty())
            {
                const TextLayout sc(sc_text, Font(), scale);
                const float right = w - PADDING - (e.submenu ? ARROW : 0);
                canvas.draw_text(sc, Point(right - sc.width(), row.y + (row.height - sc.height()) / 2),
                                 hot ? theme::on_selection() : theme::placeholder());
            }
            if (e.submenu)
            {
                // 右向きの矢印
                const float cx = w - PADDING - ARROW / 2 + 4, cy = row.y + row.height / 2;
                Path arrow;
                arrow.move_to(cx - 2, cy - 4);
                arrow.line_to(cx + 2, cy);
                arrow.line_to(cx - 2, cy + 4);
                canvas.stroke_path(arrow, 1.5f, fg, LineCap::ROUND);
            }
        }
        Widget::paint(canvas);
    }

    void mouse_move(MouseEvent& e) override
    {
        Widget::mouse_move(e);
        const int i = row_at(e.pos.y);
        if (!selectable(i))
        {
            return;
        }
        highlight(i);
        // サブメニューの項目に乗ったら開き、他の項目に乗ったら閉じる。
        if (menu->entries_[static_cast<std::size_t>(i)].submenu)
        {
            open_submenu(i, false);
        }
        else
        {
            close_child();
        }
    }

    void mouse_down(MouseEvent& e) override
    {
        Widget::mouse_down(e);
        e.accepted = true;  // 離したことを受け取るため
    }

    void mouse_up(MouseEvent& e) override
    {
        Widget::mouse_up(e);
        e.accepted = true;
        if (e.button == MouseButton::LEFT)
        {
            choose(row_at(e.pos.y));
        }
    }

    void key_down(KeyEvent& e) override
    {
        Widget::key_down(e);
        e.accepted = true;
        Menu* root = menu->root();
        switch (e.key)
        {
        case Key::UP:
            step_highlight(-1);
            break;
        case Key::DOWN:
            step_highlight(1);
            break;
        case Key::RIGHT:
            if (selectable(highlighted) && menu->entries_[static_cast<std::size_t>(highlighted)].submenu)
            {
                open_submenu(highlighted, true);
            }
            else if (root->bar_)
            {
                root->bar_->open_adjacent(root, 1);  // メニューバーの次のメニューへ
            }
            break;
        case Key::LEFT:
            if (menu->parent_menu_)
            {
                menu->close();  // サブメニューを閉じ、親のメニューへ戻る
            }
            else if (menu->bar_)
            {
                menu->bar_->open_adjacent(menu, -1);
            }
            break;
        case Key::RETURN:
        case Key::SPACE:
            choose(highlighted);
            break;
        case Key::ESCAPE:
            if (!menu->parent_menu_ && menu->bar_)
            {
                menu->bar_->menu_escaped(menu);  // メニューバーへ戻る
            }
            else
            {
                menu->close();
            }
            break;
        default:
            if (e.has_modifier(MODIFIER_CONTROL) || !choose_mnemonic(e.key))
            {
                e.accepted = false;
            }
            break;
        }
    }
};

const float MenuView::PADDING = 14;
const float MenuView::GAP = 32;
const float MenuView::ARROW = 16;
const float MenuView::CHECK = 20;
const float MenuView::SEPARATOR = 9;

void MenuAccess::forget_action(Menu* m, Action* a)
{
    for (std::size_t i = 0; i < m->entries_.size(); ++i)
    {
        if (m->entries_[i].action == a)
        {
            m->entries_.erase(m->entries_.begin() + static_cast<std::ptrdiff_t>(i));
            break;
        }
    }
    ActionAccess::detach_menu(a, m);
    m->notify_changed(m->actions);
    m->changed();
}

}  // namespace detail

Menu::Menu(Object* owner, const std::string& initial_title)
    : Object(owner),
      TOKOTACHI_PROPERTY_MEMBER(Menu, std::string, title),
      TOKOTACHI_READONLY_PROPERTY(Menu, std::vector<Action*>, actions),
      title_(initial_title),
      parent_menu_(nullptr),
      bar_(nullptr),
      popup_(nullptr),
      view_(nullptr),
      anchor_(nullptr),
      cues_(false),
      destroying_(false)
{
}

Menu::~Menu()
{
    destroying_ = true;
    // Action から自分を外す(このメニューが作った Action は、この後 Object として破棄される)。
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        if (entries_[i].action)
        {
            detail::ActionAccess::detach_menu(entries_[i].action, this);
        }
    }
    // サブメニューを先に破棄する(親のポップアップの OS のウィンドウを先に消すと、OS が子のポップアップも消すため)。
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        if (entries_[i].submenu)
        {
            delete entries_[i].submenu;
            entries_[i].submenu = nullptr;
        }
    }
    delete popup_;
}

void Menu::set_title(const std::string& v)
{
    title_ = v;
    if (parent_menu_)
    {
        parent_menu_->changed();
    }
    if (bar_)
    {
        bar_->request_layout();
        bar_->invalidate();
    }
}

std::vector<Action*> Menu::get_actions() const
{
    std::vector<Action*> out;
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        if (entries_[i].action)
        {
            out.push_back(entries_[i].action);
        }
    }
    return out;
}

void Menu::changed()
{
    if (destroying_)
    {
        return;
    }
    if (view_)
    {
        view_->invalidate();
        view_->request_layout();
    }
}

Action* Menu::add_item(const std::string& text, const NotifyHandler& on_trigger)
{
    Action* action = new Action(this, text);
    action->on_trigger = on_trigger;
    const Entry e = {action, nullptr, true};
    entries_.push_back(e);
    detail::ActionAccess::attach_menu(action, this);
    notify_changed(actions);
    changed();
    return action;
}

void Menu::add_action(Action* action)
{
    if (!action)
    {
        return;
    }
    const Entry e = {action, nullptr, false};
    entries_.push_back(e);
    detail::ActionAccess::attach_menu(action, this);
    notify_changed(actions);
    changed();
}

void Menu::remove_action(Action* action)
{
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        if (entries_[i].action == action)
        {
            const bool owned = entries_[i].owned;
            detail::MenuAccess::forget_action(this, action);
            if (owned)
            {
                delete action;
            }
            return;
        }
    }
}

void Menu::add_separator()
{
    const Entry e = {nullptr, nullptr, false};
    entries_.push_back(e);
    changed();
}

Menu* Menu::add_submenu(const std::string& text)
{
    Menu* sub = new Menu(this, text);
    sub->parent_menu_ = this;
    const Entry e = {nullptr, sub, false};
    entries_.push_back(e);
    changed();
    return sub;
}

Menu* Menu::root()
{
    Menu* m = this;
    while (m->parent_menu_)
    {
        m = m->parent_menu_;
    }
    return m;
}

void Menu::ensure_popup(Widget* anchor)
{
    if (popup_ && anchor == anchor_)
    {
        return;
    }
    // 基準のウィジェットが変わったら作り直す(OS のポップアップは、基準のウィジェットのウィンドウの子になるため)。
    delete popup_;
    anchor_ = anchor;
    popup_ = new Popup(anchor, PopupKind::MENU, this);
    view_ = new detail::MenuView(popup_, this);
    popup_->on_closed = [this](Object*)
    {
        if (view_)
        {
            view_->open_child = nullptr;
            view_->highlighted = -1;
        }
        if (parent_menu_ && parent_menu_->view_ && parent_menu_->view_->open_child == this)
        {
            parent_menu_->view_->open_child = nullptr;
        }
        if (bar_)
        {
            bar_->menu_closed(this);
        }
    };
}

void Menu::popup(Widget* anchor_widget, const Point& at)
{
    if (!anchor_widget)
    {
        return;
    }
    ensure_popup(anchor_widget);
    cues_ = false;
    view_->highlighted = -1;
    const Size size = view_->preferred_size;
    view_->bounds = Rect(Point(), size);
    popup_->open(Rect(at, Size()), size, PopupPlacement::AT);
    view_->focused = true;
}

void Menu::open_below(Widget* anchor, const Rect& r, bool keyboard)
{
    ensure_popup(anchor);
    cues_ = keyboard;
    view_->highlighted = -1;
    const Size size = view_->preferred_size;
    view_->bounds = Rect(Point(), size);
    popup_->open(r, size, PopupPlacement::BELOW);
    view_->focused = true;
    if (keyboard)
    {
        view_->step_highlight(1);
    }
}

void Menu::open_beside(detail::MenuView* parent_view, const Rect& row)
{
    ensure_popup(parent_view);
    view_->highlighted = -1;
    const Size size = view_->preferred_size;
    view_->bounds = Rect(Point(), size);
    popup_->open(row, size, PopupPlacement::RIGHT);
    view_->focused = true;
}

void Menu::close()
{
    if (view_)
    {
        view_->close_child();
    }
    if (popup_ && popup_->visible)
    {
        popup_->close();
    }
}

bool Menu::is_open() const { return popup_ && popup_->visible; }

void Menu::activate(Action* action)
{
    Menu* r = root();
    r->close();
    if (r->bar_)
    {
        r->bar_->menu_activated();
    }
    if (action)
    {
        action->trigger();  // ハンドラの中でメニューが破棄されてもよいよう、最後に呼ぶ
    }
}

}  // namespace tokotachi
