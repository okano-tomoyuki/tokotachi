#include <gui/menu.hpp>

#include <algorithm>
#include <cmath>

#include <gui/canvas.hpp>
#include <gui/popup.hpp>
#include <gui/text_layout.hpp>
#include <gui/widget.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace gui
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
    static const float SEPARATOR;

    float item_height() const { return std::ceil(TextLayout("", Font(), display_scale).height()) + 10; }

    float height_of(std::size_t i) const { return menu->entries_[i] ? item_height() : SEPARATOR; }

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
        MenuItem* item = menu->entries_[static_cast<std::size_t>(i)];
        return item && item->enabled_;
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
        MenuItem* item = menu->entries_[static_cast<std::size_t>(i)];
        Menu* sub = item ? item->submenu_ : nullptr;
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
        MenuItem* item = menu->entries_[static_cast<std::size_t>(i)];
        if (item->submenu_)
        {
            open_submenu(i, true);
        }
        else
        {
            menu->activate(item);  // ハンドラの中でメニューが破棄されてもよいよう、何もせずに戻る
        }
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
            MenuItem* item = menu->entries_[i];
            if (!item)
            {
                continue;
            }
            text_w = std::max(text_w, TextLayout(item->text_, Font(), scale).width());
            shortcut_w = std::max(shortcut_w, TextLayout(item->shortcut_text_, Font(), scale).width());
            has_submenu = has_submenu || item->submenu_;
        }
        const float w = PADDING * 2 + text_w + (shortcut_w > 0 ? GAP + shortcut_w : 0) + (has_submenu ? ARROW : 0);
        return Size(std::ceil(std::max(w, 120.0f)), std::ceil(h));
    }

    void paint(Canvas& canvas) override
    {
        const float scale = display_scale;
        const float w = width;
        for (std::size_t i = 0; i < menu->entries_.size(); ++i)
        {
            const Rect row = row_rect(i);
            MenuItem* item = menu->entries_[i];
            if (!item)
            {
                canvas.fill_rect(Rect(PADDING / 2, row.y + row.height / 2, w - PADDING, 1), theme::border());
                continue;
            }
            const bool hot = static_cast<int>(i) == highlighted && item->enabled_;
            if (hot)
            {
                canvas.fill_round_rect(row.adjusted(4, 0, 4, 0), 4, theme::selection());
            }
            const Color fg = !item->enabled_ ? theme::text_disabled() : hot ? theme::on_selection() : theme::text();
            const TextLayout text(item->text_, Font(), scale);
            canvas.draw_text(text, Point(PADDING, row.y + (row.height - text.height()) / 2), fg);
            if (!item->shortcut_text_.empty())
            {
                const TextLayout sc(item->shortcut_text_, Font(), scale);
                const float right = w - PADDING - (item->submenu_ ? ARROW : 0);
                canvas.draw_text(sc, Point(right - sc.width(), row.y + (row.height - sc.height()) / 2),
                                 hot ? theme::on_selection() : theme::placeholder());
            }
            if (item->submenu_)
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
        MenuItem* item = menu->entries_[static_cast<std::size_t>(i)];
        if (item->submenu_)
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
        switch (e.key)
        {
        case Key::UP:
            step_highlight(-1);
            break;
        case Key::DOWN:
            step_highlight(1);
            break;
        case Key::RIGHT:
            if (selectable(highlighted))
            {
                open_submenu(highlighted, true);
            }
            break;
        case Key::LEFT:
            if (menu->parent_menu_)
            {
                menu->close();  // サブメニューを閉じ、親のメニューへ戻る
            }
            break;
        case Key::RETURN:
        case Key::SPACE:
            choose(highlighted);
            break;
        case Key::ESCAPE:
            menu->close();
            break;
        default:
            e.accepted = false;
            break;
        }
    }
};

const float MenuView::PADDING = 14;
const float MenuView::GAP = 32;
const float MenuView::ARROW = 16;
const float MenuView::SEPARATOR = 9;

}  // namespace detail

// --- MenuItem ---

MenuItem::MenuItem(Menu* owner_menu, const std::string& initial_text, Menu* sub)
    : Object(owner_menu),
      GUI_PROPERTY_MEMBER(MenuItem, std::string, text),
      GUI_PROPERTY_MEMBER(MenuItem, std::string, shortcut_text),
      GUI_PROPERTY_MEMBER(MenuItem, bool, enabled),
      GUI_READONLY_PROPERTY(MenuItem, Menu*, submenu),
      GUI_HANDLER_PROPERTY(MenuItem, NotifyHandler, on_click),
      menu_(owner_menu),
      text_(initial_text),
      enabled_(true),
      submenu_(sub)
{
}

void MenuItem::changed()
{
    if (menu_->view_)
    {
        menu_->view_->invalidate();
        menu_->view_->request_layout();
    }
}

void MenuItem::set_text(const std::string& v)
{
    text_ = v;
    changed();
}

void MenuItem::set_shortcut_text(const std::string& v)
{
    shortcut_text_ = v;
    changed();
}

void MenuItem::set_enabled(const bool& v)
{
    enabled_ = v;
    changed();
}

// --- Menu ---

Menu::Menu(Object* owner) : Object(owner), parent_menu_(nullptr), popup_(nullptr), view_(nullptr), anchor_(nullptr) {}

Menu::~Menu()
{
    // サブメニューを先に破棄する(親のポップアップの OS のウィンドウを先に消すと、OS が子のポップアップも消すため)。
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        if (entries_[i] && entries_[i]->submenu_)
        {
            delete entries_[i]->submenu_;
            entries_[i]->submenu_ = nullptr;
        }
    }
    delete popup_;
}

MenuItem* Menu::add_item(const std::string& text, const NotifyHandler& on_click)
{
    MenuItem* item = new MenuItem(this, text, nullptr);
    item->on_click = on_click;
    entries_.push_back(item);
    if (view_)
    {
        view_->request_layout();
    }
    return item;
}

void Menu::add_separator()
{
    entries_.push_back(nullptr);
    if (view_)
    {
        view_->request_layout();
    }
}

Menu* Menu::add_submenu(const std::string& text)
{
    Menu* sub = new Menu(this);
    sub->parent_menu_ = this;
    entries_.push_back(new MenuItem(this, text, sub));
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
    };
}

void Menu::popup(Widget* anchor_widget, const Point& at)
{
    if (!anchor_widget)
    {
        return;
    }
    ensure_popup(anchor_widget);
    view_->highlighted = -1;
    const Size size = view_->preferred_size;
    view_->bounds = Rect(Point(), size);
    popup_->open(Rect(at, Size()), size, PopupPlacement::AT);
    view_->focused = true;
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

void Menu::activate(MenuItem* item)
{
    const NotifyHandler handler = item->on_click_;
    root()->close();
    if (handler)
    {
        handler(item);
    }
}

}  // namespace gui
