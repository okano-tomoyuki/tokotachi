#include <tokotachi/actions/menu.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/windows/menu_bar.hpp>
#include <tokotachi/windows/popup.hpp>
#include <tokotachi/graphics/text_layout.hpp>
#include <tokotachi/widgets/widget.hpp>

#include "action_access.hpp"
#include "../core/impl.hpp"
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
    static const float ICON;
    static const float SEPARATOR;

    float item_height() const { return std::ceil(TextLayout("", Font(), display_scale).height()) + 10; }

    bool is_separator(std::size_t i) const { return !detail::ImplAccess<Menu>::of(menu)->entries_[i].action && !detail::ImplAccess<Menu>::of(menu)->entries_[i].submenu; }

    float height_of(std::size_t i) const { return is_separator(i) ? SEPARATOR : item_height(); }

    // 項目の表示の文字(& を含む)。
    std::string label_of(std::size_t i) const
    {
        const detail::ImplAccess<Menu>::Impl::Entry& e = detail::ImplAccess<Menu>::of(menu)->entries_[i];
        return e.submenu ? detail::ImplAccess<Menu>::of(e.submenu)->title_ : e.action ? ActionAccess::text(e.action) : std::string();
    }

    std::string shortcut_of(std::size_t i) const
    {
        const detail::ImplAccess<Menu>::Impl::Entry& e = detail::ImplAccess<Menu>::of(menu)->entries_[i];
        return e.action ? ActionAccess::shortcut(e.action).to_string() : std::string();
    }

    // 文字の前の、チェックの印・画像の列の幅(どの項目にも無ければ 0)。
    float mark_width() const
    {
        float w = 0;
        for (std::size_t i = 0; i < detail::ImplAccess<Menu>::of(menu)->entries_.size(); ++i)
        {
            const Action* a = detail::ImplAccess<Menu>::of(menu)->entries_[i].action;
            if (a && !ActionAccess::icon(a).is_null())
            {
                return ICON + 8;
            }
            if (a && ActionAccess::checkable(a))
            {
                w = CHECK;
            }
        }
        return w;
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
        for (std::size_t i = 0; i < detail::ImplAccess<Menu>::of(menu)->entries_.size(); ++i)
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
        if (i < 0 || i >= static_cast<int>(detail::ImplAccess<Menu>::of(menu)->entries_.size()))
        {
            return false;
        }
        const detail::ImplAccess<Menu>::Impl::Entry& e = detail::ImplAccess<Menu>::of(menu)->entries_[static_cast<std::size_t>(i)];
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
        const int n = static_cast<int>(detail::ImplAccess<Menu>::of(menu)->entries_.size());
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
        Menu* sub = detail::ImplAccess<Menu>::of(menu)->entries_[static_cast<std::size_t>(i)].submenu;
        if (!sub || sub == open_child)
        {
            return;
        }
        close_child();
        open_child = sub;
        detail::ImplAccess<Menu>::of(sub)->open_beside(this, row_rect(static_cast<std::size_t>(i)));
        if (keyboard && detail::ImplAccess<Menu>::of(sub)->view_)
        {
            detail::ImplAccess<Menu>::of(sub)->view_->highlighted = -1;
            detail::ImplAccess<Menu>::of(sub)->view_->step_highlight(1);
        }
    }

    void choose(int i)
    {
        if (!selectable(i))
        {
            return;
        }
        const detail::ImplAccess<Menu>::Impl::Entry& e = detail::ImplAccess<Menu>::of(menu)->entries_[static_cast<std::size_t>(i)];
        if (e.submenu)
        {
            open_submenu(i, true);
        }
        else
        {
            detail::ImplAccess<Menu>::of(menu)->activate(e.action);  // ハンドラの中でメニューが破棄されてもよいよう、何もせずに戻る
        }
    }

    // ニーモニックのキーの項目を選ぶ(同じキーの項目が複数あれば、順に強調するだけ)。使ったら true。
    bool choose_mnemonic(Key key)
    {
        std::vector<int> matches;
        for (std::size_t i = 0; i < detail::ImplAccess<Menu>::of(menu)->entries_.size(); ++i)
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
        for (std::size_t i = 0; i < detail::ImplAccess<Menu>::of(menu)->entries_.size(); ++i)
        {
            h += height_of(i);
            if (is_separator(i))
            {
                continue;
            }
            text_w = std::max(text_w, TextLayout(parse_mnemonic(label_of(i)).text, Font(), scale).width());
            shortcut_w = std::max(shortcut_w, TextLayout(shortcut_of(i), Font(), scale).width());
            has_submenu = has_submenu || detail::ImplAccess<Menu>::of(menu)->entries_[i].submenu;
        }
        const float w = PADDING * 2 + mark_width() + text_w + (shortcut_w > 0 ? GAP + shortcut_w : 0) +
                        (has_submenu ? ARROW : 0);
        return Size(std::ceil(std::max(w, 120.0f)), std::ceil(h));
    }

    void paint(Canvas& canvas) override
    {
        const float scale = display_scale;
        const float w = width;
        const float text_x = PADDING + mark_width();
        const bool cues = detail::ImplAccess<Menu>::of(detail::ImplAccess<Menu>::of(menu)->root())->cues_;
        for (std::size_t i = 0; i < detail::ImplAccess<Menu>::of(menu)->entries_.size(); ++i)
        {
            const Rect row = row_rect(i);
            if (is_separator(i))
            {
                canvas.fill_rect(Rect(PADDING / 2, row.y + row.height / 2, w - PADDING, 1), theme::border());
                continue;
            }
            const detail::ImplAccess<Menu>::Impl::Entry& e = detail::ImplAccess<Menu>::of(menu)->entries_[i];
            const bool enabled_now = selectable(static_cast<int>(i));
            const bool hot = static_cast<int>(i) == highlighted && enabled_now;
            if (hot)
            {
                canvas.fill_round_rect(row.adjusted(4, 0, 4, 0), 4, theme::selection());
            }
            const Color fg = !enabled_now ? theme::text_disabled() : hot ? theme::on_selection() : theme::text();
            const bool checked = e.action && ActionAccess::checkable(e.action) && ActionAccess::checked(e.action);
            if (e.action && !ActionAccess::icon(e.action).is_null())
            {
                // 画像(チェックしているなら、画像を枠で囲む)
                const Rect icon_rect(PADDING, row.y + std::floor((row.height - ICON) / 2), ICON, ICON);
                if (checked)
                {
                    const Rect box = icon_rect.adjusted(-3, -3, -3, -3);
                    canvas.fill_round_rect(box, 3, hot ? theme::on_selection().with_alpha(48) : theme::selection_inactive());
                    canvas.stroke_round_rect(box, 3, 1, hot ? theme::on_selection() : theme::accent());
                }
                const Image& icon = ActionAccess::icon(e.action);
                canvas.draw_image(icon, icon_rect, fg, enabled_now || icon.is_icon() ? 1.0f : 0.4f);
            }
            else if (checked)
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
        if (detail::ImplAccess<Menu>::of(menu)->entries_[static_cast<std::size_t>(i)].submenu)
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
        Menu* root = detail::ImplAccess<Menu>::of(menu)->root();
        switch (e.key)
        {
        case Key::UP:
            step_highlight(-1);
            break;
        case Key::DOWN:
            step_highlight(1);
            break;
        case Key::RIGHT:
            if (selectable(highlighted) && detail::ImplAccess<Menu>::of(menu)->entries_[static_cast<std::size_t>(highlighted)].submenu)
            {
                open_submenu(highlighted, true);
            }
            else if (detail::ImplAccess<Menu>::of(root)->bar_)
            {
                detail::ImplAccess<MenuBar>::of(detail::ImplAccess<Menu>::of(root)->bar_)->open_adjacent(root, 1);  // メニューバーの次のメニューへ
            }
            break;
        case Key::LEFT:
            if (detail::ImplAccess<Menu>::of(menu)->parent_menu_)
            {
                menu->close();  // サブメニューを閉じ、親のメニューへ戻る
            }
            else if (detail::ImplAccess<Menu>::of(menu)->bar_)
            {
                detail::ImplAccess<MenuBar>::of(detail::ImplAccess<Menu>::of(menu)->bar_)->open_adjacent(menu, -1);
            }
            break;
        case Key::RETURN:
        case Key::SPACE:
            choose(highlighted);
            break;
        case Key::ESCAPE:
            if (!detail::ImplAccess<Menu>::of(menu)->parent_menu_ && detail::ImplAccess<Menu>::of(menu)->bar_)
            {
                detail::ImplAccess<MenuBar>::of(detail::ImplAccess<Menu>::of(menu)->bar_)->menu_escaped(menu);  // メニューバーへ戻る
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
const float MenuView::ICON = 16;
const float MenuView::SEPARATOR = 9;

void MenuAccess::forget_action(Menu* m, Action* a)
{
    for (std::size_t i = 0; i < detail::ImplAccess<Menu>::of(m)->entries_.size(); ++i)
    {
        if (detail::ImplAccess<Menu>::of(m)->entries_[i].action == a)
        {
            detail::ImplAccess<Menu>::of(m)->entries_.erase(detail::ImplAccess<Menu>::of(m)->entries_.begin() + static_cast<std::ptrdiff_t>(i));
            break;
        }
    }
    ActionAccess::detach_menu(a, m);
    m->notify_changed(m->actions);
    detail::ImplAccess<Menu>::of(m)->changed();
}

}  // namespace detail

Menu::Menu(Object* owner, const std::string& initial_title)
    : Object(owner),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, title),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::vector<Action*>, actions),
      d_(new Impl(this))
{
    d_->title_ = initial_title;
    d_->parent_menu_ = nullptr;
    d_->bar_ = nullptr;
    d_->popup_ = nullptr;
    d_->view_ = nullptr;
    d_->anchor_ = nullptr;
    d_->cues_ = false;
    d_->destroying_ = false;
}

Menu::~Menu()
{
    d_->destroying_ = true;
    // Action から自分を外す(このメニューが作った Action は、この後 Object として破棄される)。
    for (std::size_t i = 0; i < d_->entries_.size(); ++i)
    {
        if (d_->entries_[i].action)
        {
            detail::ActionAccess::detach_menu(d_->entries_[i].action, this);
        }
    }
    // サブメニューを先に破棄する(親のポップアップの OS のウィンドウを先に消すと、OS が子のポップアップも消すため)。
    for (std::size_t i = 0; i < d_->entries_.size(); ++i)
    {
        if (d_->entries_[i].submenu)
        {
            delete d_->entries_[i].submenu;
            d_->entries_[i].submenu = nullptr;
        }
    }
    delete d_->popup_;
    delete d_;
}

void Menu::Impl::set_title(const std::string& v)
{
    title_ = v;
    if (parent_menu_)
    {
        detail::ImplAccess<Menu>::of(parent_menu_)->changed();
    }
    if (bar_)
    {
        bar_->request_layout();
        bar_->invalidate();
    }
}

std::vector<Action*> Menu::Impl::get_actions() const
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

void Menu::Impl::changed()
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
    const Impl::Entry e = {action, nullptr, true};
    d_->entries_.push_back(e);
    detail::ActionAccess::attach_menu(action, this);
    notify_changed(actions);
    d_->changed();
    return action;
}

void Menu::add_action(Action* action)
{
    if (!action)
    {
        return;
    }
    const Impl::Entry e = {action, nullptr, false};
    d_->entries_.push_back(e);
    detail::ActionAccess::attach_menu(action, this);
    notify_changed(actions);
    d_->changed();
}

void Menu::remove_action(Action* action)
{
    for (std::size_t i = 0; i < d_->entries_.size(); ++i)
    {
        if (d_->entries_[i].action == action)
        {
            const bool owned = d_->entries_[i].owned;
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
    const Impl::Entry e = {nullptr, nullptr, false};
    d_->entries_.push_back(e);
    d_->changed();
}

Menu* Menu::add_submenu(const std::string& text)
{
    Menu* sub = new Menu(this, text);
    detail::ImplAccess<Menu>::of(sub)->parent_menu_ = this;
    const Impl::Entry e = {nullptr, sub, false};
    d_->entries_.push_back(e);
    d_->changed();
    return sub;
}

Menu* Menu::Impl::root()
{
    Menu* m = q;
    while (detail::ImplAccess<Menu>::of(m)->parent_menu_)
    {
        m = detail::ImplAccess<Menu>::of(m)->parent_menu_;
    }
    return m;
}

void Menu::Impl::ensure_popup(Widget* anchor)
{
    if (popup_ && anchor == anchor_)
    {
        return;
    }
    // 基準のウィジェットが変わったら作り直す(OS のポップアップは、基準のウィジェットのウィンドウの子になるため)。
    delete popup_;
    anchor_ = anchor;
    popup_ = new Popup(anchor, PopupKind::MENU, q);
    view_ = new detail::MenuView(popup_, q);
    popup_->on_closed = [this](Object*)
    {
        if (view_)
        {
            view_->open_child = nullptr;
            view_->highlighted = -1;
        }
        if (parent_menu_ && detail::ImplAccess<Menu>::of(parent_menu_)->view_ && detail::ImplAccess<Menu>::of(parent_menu_)->view_->open_child == q)
        {
            detail::ImplAccess<Menu>::of(parent_menu_)->view_->open_child = nullptr;
        }
        if (bar_)
        {
            detail::ImplAccess<MenuBar>::of(bar_)->menu_closed(q);
        }
    };
}

void Menu::popup(Widget* anchor_widget, const Point& at)
{
    if (!anchor_widget)
    {
        return;
    }
    d_->ensure_popup(anchor_widget);
    d_->cues_ = false;
    d_->view_->highlighted = -1;
    const Size size = d_->view_->preferred_size;
    d_->view_->bounds = Rect(Point(), size);
    d_->popup_->open(Rect(at, Size()), size, PopupPlacement::AT);
    d_->view_->focused = true;
}

void Menu::Impl::open_below(Widget* anchor, const Rect& r, bool keyboard)
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

void Menu::Impl::open_beside(detail::MenuView* parent_view, const Rect& row)
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
    if (d_->view_)
    {
        d_->view_->close_child();
    }
    if (d_->popup_ && d_->popup_->visible)
    {
        d_->popup_->close();
    }
}

bool Menu::is_open() const { return d_->popup_ && d_->popup_->visible; }

void Menu::Impl::activate(Action* action)
{
    Menu* r = root();
    r->close();
    if (detail::ImplAccess<Menu>::of(r)->bar_)
    {
        detail::ImplAccess<MenuBar>::of(detail::ImplAccess<Menu>::of(r)->bar_)->menu_activated();
    }
    if (action)
    {
        action->trigger();  // ハンドラの中でメニューが破棄されてもよいよう、最後に呼ぶ
    }
}

}  // namespace tokotachi
