#include <gui/tab_widget.hpp>

#include <algorithm>
#include <cmath>

#include <gui/canvas.hpp>
#include <gui/window.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace gui
{

namespace
{
const float TAB_PADDING_X = 12;  // 見出しの文字の左右の余白(dp)
const float TAB_PADDING_Y = 8;   // 見出しの文字の上下の余白(dp)
const float MIN_TAB = 48;        // 見出しの最小の幅(dp)
const float ARROW_WIDTH = 24;    // ◀▶ のボタンの幅(dp)
const float SCROLL_STEP = 80;    // ◀▶・ホイールの 1 回で動かす量(dp)
const int HIT_LEFT = -2;
const int HIT_RIGHT = -3;
}  // namespace

Tab::Tab(TabWidget* tab_widget, Widget* initial_page, const std::string& initial_title)
    : Object(tab_widget),
      GUI_PROPERTY_MEMBER(Tab, std::string, title),
      GUI_PROPERTY_MEMBER(Tab, bool, enabled),
      GUI_READONLY_PROPERTY(Tab, Widget*, page),
      tab_widget_(tab_widget),
      page_(initial_page),
      title_(initial_title),
      enabled_(true)
{
}

Tab::~Tab() {}

void Tab::set_title(const std::string& v)
{
    title_ = v;
    layout_ = TextLayout();
    if (tab_widget_)
    {
        tab_widget_->tab_changed();
    }
}

void Tab::set_enabled(const bool& v)
{
    enabled_ = v;
    if (tab_widget_)
    {
        tab_widget_->tab_changed();
    }
}

TabWidget::TabWidget(Widget* parent_widget)
    : Widget(parent_widget),
      GUI_READONLY_PROPERTY(TabWidget, std::vector<Tab*>, tabs),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(TabWidget, int, current_index),
      GUI_READONLY_PROPERTY(TabWidget, Tab*, current_tab),
      GUI_HANDLER_PROPERTY(TabWidget, NotifyHandler, on_change),
      current_(-1),
      scroll_(0),
      hover_(-1),
      destroying_(false)
{
    focus_policy = FocusPolicy::CLICK;
}

TabWidget::~TabWidget()
{
    // ページ(子)は Widget のデストラクタが破棄する。タブはここで先に破棄する(Tab から自分を参照させない)。
    destroying_ = true;
    for (std::size_t i = 0; i < tabs_.size(); ++i)
    {
        tabs_[i]->tab_widget_ = nullptr;
        delete tabs_[i];
    }
    tabs_.clear();
}

Tab* TabWidget::add_tab(Widget* page, const std::string& title)
{
    return insert_tab(static_cast<int>(tabs_.size()), page, title);
}

Tab* TabWidget::insert_tab(int index, Widget* page, const std::string& title)
{
    if (!page || page == this)
    {
        return nullptr;
    }
    for (std::size_t i = 0; i < tabs_.size(); ++i)
    {
        if (tabs_[i]->page_ == page)
        {
            return tabs_[i];  // 既にこのページのタブがある
        }
    }
    const bool was_visible = page->visible;
    page->visible = false;
    page->parent = this;  // 別の TabWidget のページだったら、そちらのタブは消える
    if (page->parent.get() != this)
    {
        page->visible = was_visible;
        return nullptr;  // 子にできなかった(ウィンドウ・祖先など)
    }
    const int n = static_cast<int>(tabs_.size());
    const int at = index >= 0 && index <= n ? index : n;
    Tab* tab = new Tab(this, page, title);
    tabs_.insert(tabs_.begin() + at, tab);
    notify_changed(tabs);
    request_layout();
    invalidate();
    if (current_ < 0)
    {
        select(at);  // 最初のタブを選ぶ
    }
    else if (at <= current_)
    {
        select(current_ + 1);  // 選んでいるタブは同じで、番号がずれる
    }
    return tab;
}

Widget* TabWidget::remove_tab(Tab* tab)
{
    const std::vector<Tab*>::iterator it = std::find(tabs_.begin(), tabs_.end(), tab);
    if (it == tabs_.end())
    {
        return nullptr;
    }
    Widget* page = tab->page_;
    erase_tab(static_cast<int>(it - tabs_.begin()));
    page->parent = nullptr;  // タブは消してあるので、child_removed では何もしない
    page->visible = true;
    return page;
}

void TabWidget::erase_tab(int i)
{
    Tab* tab = tabs_[i];
    tabs_.erase(tabs_.begin() + i);
    tab->tab_widget_ = nullptr;
    delete tab;
    notify_changed(tabs);
    request_layout();
    scroll_to(scroll_);
    hover_ = -1;
    invalidate();
    const int n = static_cast<int>(tabs_.size());
    if (i < current_)
    {
        select(current_ - 1);
    }
    else if (i == current_)
    {
        select(n == 0 ? -1 : std::min(i, n - 1));
    }
}

void TabWidget::child_removed(Widget* child)
{
    if (destroying_)
    {
        return;
    }
    for (std::size_t i = 0; i < tabs_.size(); ++i)
    {
        if (tabs_[i]->page_ == child)
        {
            erase_tab(static_cast<int>(i));
            return;
        }
    }
}

void TabWidget::tab_changed()
{
    request_layout();
    scroll_to(scroll_);
    invalidate();
}

void TabWidget::set_current_index(const int& v)
{
    if (v < 0 || v >= static_cast<int>(tabs_.size()) || v == current_)
    {
        return;
    }
    select(v);
}

void TabWidget::select(int i)
{
    // 隠すページの中にフォーカスがあれば、TabWidget へ移す(続けて Ctrl+Tab などで選べるように)。
    bool focus_inside = false;
    Window* w = window;
    for (Widget* f = w ? w->focused_widget.get() : nullptr; f && f != this; f = f->parent)
    {
        if (current_ >= 0 && current_ < static_cast<int>(tabs_.size()) && f == tabs_[current_]->page_ && current_ != i)
        {
            focus_inside = true;
        }
    }
    current_ = i;
    for (std::size_t k = 0; k < tabs_.size(); ++k)
    {
        tabs_[k]->page_->visible = static_cast<int>(k) == i;
    }
    if (i >= 0)
    {
        ensure_visible(i);
    }
    if (focus_inside)
    {
        focused = true;
    }
    invalidate();
    // 結びつけた相手へ伝えてから、ハンドラを呼ぶ。
    notify_changed(current_index);
    notify_changed(current_tab);
    Object* self = this;
    detail::fire(on_change_, self);
}

int TabWidget::next_enabled(int from, int step) const
{
    const int n = static_cast<int>(tabs_.size());
    for (int i = from; i >= 0 && i < n; i += step)
    {
        if (tabs_[i]->enabled_)
        {
            return i;
        }
    }
    return -1;
}

const TextLayout& TabWidget::title_layout(const Tab* tab) const
{
    const float scale = display_scale;
    if (tab->layout_.is_null() || tab->layout_.scale() != scale)
    {
        tab->layout_ = TextLayout(tab->title_, Font(), scale);
    }
    return tab->layout_;
}

float TabWidget::bar_height() const
{
    return std::ceil(TextLayout("Ag", Font(), display_scale).height()) + TAB_PADDING_Y * 2;
}

float TabWidget::tab_width(const Tab* tab) const
{
    return std::max(MIN_TAB, std::ceil(title_layout(tab).width()) + TAB_PADDING_X * 2);
}

float TabWidget::total_width() const
{
    float w = 0;
    for (std::size_t i = 0; i < tabs_.size(); ++i)
    {
        w += tab_width(tabs_[i]);
    }
    return w;
}

bool TabWidget::overflowing() const { return total_width() > width.get(); }

float TabWidget::visible_width() const
{
    const float w = width;
    return std::max(0.0f, overflowing() ? w - ARROW_WIDTH * 2 : w);
}

void TabWidget::scroll_to(float x)
{
    const float limit = std::max(0.0f, total_width() - visible_width());
    const float s = std::max(0.0f, std::min(x, limit));
    if (s != scroll_)
    {
        scroll_ = s;
        invalidate();
    }
}

void TabWidget::ensure_visible(int i)
{
    float x = 0;
    for (int k = 0; k < i; ++k)
    {
        x += tab_width(tabs_[k]);
    }
    const float w = tab_width(tabs_[i]);
    const float view = visible_width();
    if (x < scroll_)
    {
        scroll_to(x);
    }
    else if (x + w > scroll_ + view)
    {
        scroll_to(x + w - view);
    }
}

int TabWidget::hit(const Point& p) const
{
    if (p.y < 0 || p.y >= bar_height())
    {
        return -1;
    }
    const float view = visible_width();
    if (overflowing() && p.x >= view)
    {
        return p.x < view + ARROW_WIDTH ? HIT_LEFT : HIT_RIGHT;
    }
    float x = -scroll_;
    for (std::size_t i = 0; i < tabs_.size(); ++i)
    {
        const float w = tab_width(tabs_[i]);
        if (p.x >= x && p.x < x + w)
        {
            return static_cast<int>(i);
        }
        x += w;
    }
    return -1;
}

void TabWidget::arrange_children()
{
    const Rect r = local_rect;
    const float bar = bar_height();
    const Rect area(0, bar, r.width, std::max(0.0f, r.height - bar));
    for (std::size_t i = 0; i < tabs_.size(); ++i)
    {
        tabs_[i]->page_->bounds = area;
    }
    scroll_to(scroll_);
    if (current_ >= 0)
    {
        ensure_visible(current_);
    }
}

Size TabWidget::compute_preferred_size() const
{
    // どのページを選んでも収まる大きさ。
    Size s(0, 0);
    for (std::size_t i = 0; i < tabs_.size(); ++i)
    {
        const Size p = tabs_[i]->page_->preferred_size;
        s.width = std::max(s.width, p.width);
        s.height = std::max(s.height, p.height);
    }
    return Size(std::max(s.width, total_width()), s.height + bar_height());
}

Size TabWidget::compute_minimum_size() const
{
    // 見出しは横にスクロールできるので、幅はページの最小の幅で決める。
    Size s(0, 0);
    for (std::size_t i = 0; i < tabs_.size(); ++i)
    {
        const Size m = tabs_[i]->page_->minimum_hint;
        s.width = std::max(s.width, m.width);
        s.height = std::max(s.height, m.height);
    }
    return Size(std::max(s.width, MIN_TAB + ARROW_WIDTH * 2), s.height + bar_height());
}

void TabWidget::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const float bar = bar_height();
    const bool enabled_now = effectively_enabled;
    canvas.fill_rect(Rect(0, bar - 1, r.width, 1), theme::border());

    const float view = visible_width();
    canvas.save();
    canvas.clip_rect(Rect(0, 0, view, bar));
    float x = -scroll_;
    for (std::size_t i = 0; i < tabs_.size(); ++i)
    {
        const Tab* tab = tabs_[i];
        const float w = tab_width(tab);
        if (x + w > 0 && x < view)
        {
            const bool selected = static_cast<int>(i) == current_;
            const bool usable = enabled_now && tab->enabled_;
            const Rect cell(x, 0, w, bar);
            if (usable && static_cast<int>(i) == hover_ && !selected)
            {
                canvas.fill_round_rect(cell.adjusted(2, 4, 2, 4), theme::field_radius(), theme::selection_inactive());
            }
            const Color fg = !usable ? theme::text_disabled() : selected ? theme::accent() : theme::color(ColorRole::TEXT_SECONDARY);
            const TextLayout& l = title_layout(tab);
            canvas.draw_text(l, Point(x + (w - l.width()) / 2, (bar - l.height()) / 2), fg);
            if (selected)
            {
                canvas.fill_rect(Rect(x + 4, bar - 3, w - 8, 3), enabled_now ? theme::accent() : theme::accent_disabled());
                if (focused)
                {
                    canvas.stroke_round_rect(cell.adjusted(2, 3, 2, 5), theme::field_radius(), 2, theme::focus_ring());
                }
            }
        }
        x += w;
    }
    canvas.restore();

    if (overflowing())
    {
        // ◀▶: 端まで動かしたら薄くする。
        const float limit = std::max(0.0f, total_width() - view);
        const float cy = bar / 2;
        for (int k = 0; k < 2; ++k)
        {
            const float cx = view + ARROW_WIDTH * k + ARROW_WIDTH / 2;
            const bool can = k == 0 ? scroll_ > 0 : scroll_ < limit;
            const float dir = k == 0 ? -1.0f : 1.0f;
            Path mark;
            mark.move_to(cx - dir * 2, cy - 5);
            mark.line_to(cx + dir * 3, cy);
            mark.line_to(cx - dir * 2, cy + 5);
            canvas.stroke_path(mark, 1.5f, can && enabled_now ? theme::text() : theme::text_disabled(), LineCap::ROUND);
        }
    }
    Widget::paint(canvas);
}

void TabWidget::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    const int h = hit(e.pos);
    if (h == -1)
    {
        return;
    }
    e.accepted = true;
    if (h == HIT_LEFT)
    {
        scroll_to(scroll_ - SCROLL_STEP);
    }
    else if (h == HIT_RIGHT)
    {
        scroll_to(scroll_ + SCROLL_STEP);
    }
    else if (tabs_[h]->enabled_)
    {
        current_index = h;
    }
}

void TabWidget::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    const int h = hit(e.pos);
    const int next = h >= 0 ? h : -1;
    if (next != hover_)
    {
        hover_ = next;
        invalidate();
    }
}

void TabWidget::mouse_leave()
{
    if (hover_ != -1)
    {
        hover_ = -1;
        invalidate();
    }
    Widget::mouse_leave();
}

void TabWidget::mouse_wheel(WheelEvent& e)
{
    Widget::mouse_wheel(e);
    if (hit(e.pos) == -1 || !overflowing())
    {
        return;  // 見出しの並びの外・スクロールできないときは親へ送る
    }
    e.accepted = true;
    const float ticks = e.delta.x != 0 ? e.delta.x : -e.delta.y;
    scroll_to(scroll_ + ticks * SCROLL_STEP / 2);
}

void TabWidget::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    const int n = static_cast<int>(tabs_.size());
    const bool ctrl = e.has_modifier(MODIFIER_CONTROL);
    int step = 0;
    bool wrap = false;
    if (ctrl && (e.key == Key::TAB || e.key == Key::PAGE_UP || e.key == Key::PAGE_DOWN))
    {
        // 中のどこにフォーカスがあっても届く(子が受け取らなかったキーは親へ送られる)。巡回する。
        step = e.key == Key::PAGE_UP || (e.key == Key::TAB && e.has_modifier(MODIFIER_SHIFT)) ? -1 : 1;
        wrap = true;
    }
    else if (focused && (e.key == Key::LEFT || e.key == Key::RIGHT))
    {
        step = e.key == Key::LEFT ? -1 : 1;
    }
    else if (focused && (e.key == Key::HOME || e.key == Key::END))
    {
        e.accepted = true;
        const int i = e.key == Key::HOME ? next_enabled(0, 1) : next_enabled(n - 1, -1);
        if (i >= 0)
        {
            current_index = i;
        }
        return;
    }
    if (step == 0 || n == 0)
    {
        return;
    }
    e.accepted = true;
    int i = current_;
    for (int k = 0; k < n; ++k)
    {
        i += step;
        if (wrap)
        {
            i = (i + n) % n;
        }
        else if (i < 0 || i >= n)
        {
            return;
        }
        if (tabs_[i]->enabled_)
        {
            current_index = i;
            return;
        }
    }
}

void TabWidget::focus_in()
{
    invalidate();
    Widget::focus_in();
}

void TabWidget::focus_out()
{
    invalidate();
    Widget::focus_out();
}

void TabWidget::scale_changed()
{
    for (std::size_t i = 0; i < tabs_.size(); ++i)
    {
        tabs_[i]->layout_ = TextLayout();
    }
}

}  // namespace gui
