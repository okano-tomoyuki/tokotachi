#include <tokotachi/widgets/tab_widget.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/windows/window.hpp>

#include "../core/impl.hpp"
#include "theme.hpp"
#include "widget_access.hpp"

namespace tokotachi
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

class Tab::Impl
{
    TOKOTACHI_IMPL(Tab);

    explicit Impl(Tab* object) : q(object) {}

    void set_title(const std::string& v);
    void set_enabled(const bool& v);
    Widget* get_page() const { return page_; }

    TabWidget* tab_widget_;
    Widget* page_;
    std::string title_;
    bool enabled_;
    mutable TextLayout layout_;
};

class TabWidget::Impl : public detail::WidgetHooks
{
    TOKOTACHI_IMPL(TabWidget);

    explicit Impl(TabWidget* object) : q(object) {}

    // 内部のフック(docs/adr/0058)
    void child_removed(Widget* child) override;

    std::vector<Tab*> get_tabs() const { return tabs_; }
    int get_current_index() const { return current_; }
    void set_current_index(const int& v);
    Tab* get_current_tab() const { return current_ >= 0 ? tabs_[current_] : nullptr; }

    /// 番号 i のタブを除く(ページの親は変えない)。
    void erase_tab(int i);
    /// 見出しが変わった・タブの有効が変わった。
    void tab_changed();
    /// 選ぶタブを変え、ページの表示を切り替え、知らせる。
    void select(int i);
    /// 有効なタブを from から step の向きに探す(見つからなければ -1)。
    int next_enabled(int from, int step) const;

    /// 見出しの並びの高さ・各見出しの幅(dp)。
    float bar_height() const;
    float tab_width(const Tab* tab) const;
    const TextLayout& title_layout(const Tab* tab) const;
    float total_width() const;
    /// 見出しを並べられる幅(はみ出すときは ◀▶ の分を除く)。
    float visible_width() const;
    bool overflowing() const;
    void scroll_to(float x);
    void ensure_visible(int i);
    /// 位置 p にある見出しの番号(無ければ -1)。◀▶ の上なら -2・-3。
    int hit(const Point& p) const;

    std::vector<Tab*> tabs_;
    int current_;
    NotifyHandler on_change_;
    float scroll_;  ///< 見出しの並びの横のスクロール(dp)
    int hover_;     ///< ポインタが乗っている見出し(-1 なら無し)
    bool destroying_;
};

Tab::Tab(TabWidget* tab_widget, Widget* initial_page, const std::string& initial_title)
    : Object(tab_widget),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, title),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, enabled),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Widget*, page),
      d_(new Impl(this))
{
    d_->tab_widget_ = tab_widget;
    d_->page_ = initial_page;
    d_->title_ = initial_title;
    d_->enabled_ = true;
}

Tab::~Tab() {
    delete d_;
}

void Tab::Impl::set_title(const std::string& v)
{
    title_ = v;
    layout_ = TextLayout();
    if (tab_widget_)
    {
        tab_widget_->d_->tab_changed();
    }
}

void Tab::Impl::set_enabled(const bool& v)
{
    enabled_ = v;
    if (tab_widget_)
    {
        tab_widget_->d_->tab_changed();
    }
}

TabWidget::TabWidget(Widget* parent_widget)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::vector<Tab*>, tabs),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, int, current_index),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Tab*, current_tab),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_change),
      d_(new Impl(this))
{
    d_->install(this);
    d_->current_ = -1;
    d_->scroll_ = 0;
    d_->hover_ = -1;
    d_->destroying_ = false;
    focus_policy = FocusPolicy::CLICK;
}

TabWidget::~TabWidget()
{
    // ページ(子)は Widget のデストラクタが破棄する。タブはここで先に破棄する(Tab から自分を参照させない)。
    d_->destroying_ = true;
    for (std::size_t i = 0; i < d_->tabs_.size(); ++i)
    {
        d_->tabs_[i]->d_->tab_widget_ = nullptr;
        delete d_->tabs_[i];
    }
    d_->tabs_.clear();
    delete d_;
}

Tab* TabWidget::add_tab(Widget* page, const std::string& title)
{
    return insert_tab(static_cast<int>(d_->tabs_.size()), page, title);
}

Tab* TabWidget::insert_tab(int index, Widget* page, const std::string& title)
{
    if (!page || page == this)
    {
        return nullptr;
    }
    for (std::size_t i = 0; i < d_->tabs_.size(); ++i)
    {
        if (d_->tabs_[i]->d_->page_ == page)
        {
            return d_->tabs_[i];  // 既にこのページのタブがある
        }
    }
    page->parent = this;  // 別の TabWidget のページだったら、そちらのタブは消える
    if (page->parent.get() != this)
    {
        return nullptr;  // 子にできなかった(ウィンドウ・祖先など)
    }
    // 選ぶまで隠す(利用者の visible は変えない。docs/adr/0041)
    detail::WidgetAccess::set_hidden_by_parent(page, true);
    const int n = static_cast<int>(d_->tabs_.size());
    const int at = index >= 0 && index <= n ? index : n;
    Tab* tab = new Tab(this, page, title);
    d_->tabs_.insert(d_->tabs_.begin() + at, tab);
    notify_changed(tabs);
    request_layout();
    invalidate();
    if (d_->current_ < 0)
    {
        d_->select(at);  // 最初のタブを選ぶ
    }
    else if (at <= d_->current_)
    {
        d_->select(d_->current_ + 1);  // 選んでいるタブは同じで、番号がずれる
    }
    return tab;
}

Widget* TabWidget::remove_tab(Tab* tab)
{
    const std::vector<Tab*>::iterator it = std::find(d_->tabs_.begin(), d_->tabs_.end(), tab);
    if (it == d_->tabs_.end())
    {
        return nullptr;
    }
    Widget* page = tab->d_->page_;
    d_->erase_tab(static_cast<int>(it - d_->tabs_.begin()));
    page->parent = nullptr;  // タブは消してあるので、child_removed では何もしない
    detail::WidgetAccess::set_hidden_by_parent(page, false);
    return page;
}

void TabWidget::Impl::erase_tab(int i)
{
    Tab* tab = tabs_[i];
    tabs_.erase(tabs_.begin() + i);
    tab->d_->tab_widget_ = nullptr;
    delete tab;
    q->notify_changed(q->tabs);
    q->request_layout();
    scroll_to(scroll_);
    hover_ = -1;
    q->invalidate();
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

void TabWidget::Impl::child_removed(Widget* child)
{
    if (destroying_)
    {
        return;
    }
    for (std::size_t i = 0; i < tabs_.size(); ++i)
    {
        if (tabs_[i]->d_->page_ == child)
        {
            erase_tab(static_cast<int>(i));
            detail::WidgetAccess::set_hidden_by_parent(child, false);  // 別の親へ移った: もう隠さない
            return;
        }
    }
}

void TabWidget::Impl::tab_changed()
{
    q->request_layout();
    scroll_to(scroll_);
    q->invalidate();
}

void TabWidget::Impl::set_current_index(const int& v)
{
    if (v < 0 || v >= static_cast<int>(tabs_.size()) || v == current_)
    {
        return;
    }
    select(v);
}

void TabWidget::Impl::select(int i)
{
    // 隠すページの中にフォーカスがあれば、TabWidget へ移す(続けて Ctrl+Tab などで選べるように)。
    bool focus_inside = false;
    Window* w = q->window;
    for (Widget* f = w ? w->focused_widget.get() : nullptr; f && f != q; f = f->parent)
    {
        if (current_ >= 0 && current_ < static_cast<int>(tabs_.size()) && f == tabs_[current_]->d_->page_ && current_ != i)
        {
            focus_inside = true;
        }
    }
    current_ = i;
    for (std::size_t k = 0; k < tabs_.size(); ++k)
    {
        detail::WidgetAccess::set_hidden_by_parent(tabs_[k]->d_->page_, static_cast<int>(k) != i);
    }
    if (i >= 0)
    {
        ensure_visible(i);
    }
    if (focus_inside)
    {
        q->focused = true;
    }
    q->invalidate();
    // 結びつけた相手へ伝えてから、ハンドラを呼ぶ。
    q->notify_changed(q->current_index);
    q->notify_changed(q->current_tab);
    Object* self = q;
    detail::fire(on_change_, self);
}

int TabWidget::Impl::next_enabled(int from, int step) const
{
    const int n = static_cast<int>(tabs_.size());
    for (int i = from; i >= 0 && i < n; i += step)
    {
        if (tabs_[i]->d_->enabled_)
        {
            return i;
        }
    }
    return -1;
}

const TextLayout& TabWidget::Impl::title_layout(const Tab* tab) const
{
    const float scale = q->display_scale;
    if (tab->d_->layout_.is_null() || tab->d_->layout_.scale() != scale)
    {
        tab->d_->layout_ = TextLayout(tab->d_->title_, Font(), scale);
    }
    return tab->d_->layout_;
}

float TabWidget::Impl::bar_height() const
{
    return std::ceil(TextLayout("Ag", Font(), q->display_scale).height()) + TAB_PADDING_Y * 2;
}

float TabWidget::Impl::tab_width(const Tab* tab) const
{
    return std::max(MIN_TAB, std::ceil(title_layout(tab).width()) + TAB_PADDING_X * 2);
}

float TabWidget::Impl::total_width() const
{
    float w = 0;
    for (std::size_t i = 0; i < tabs_.size(); ++i)
    {
        w += tab_width(tabs_[i]);
    }
    return w;
}

bool TabWidget::Impl::overflowing() const { return total_width() > q->width.get(); }

float TabWidget::Impl::visible_width() const
{
    const float w = q->width;
    return std::max(0.0f, overflowing() ? w - ARROW_WIDTH * 2 : w);
}

void TabWidget::Impl::scroll_to(float x)
{
    const float limit = std::max(0.0f, total_width() - visible_width());
    const float s = std::max(0.0f, std::min(x, limit));
    if (s != scroll_)
    {
        scroll_ = s;
        q->invalidate();
    }
}

void TabWidget::Impl::ensure_visible(int i)
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

int TabWidget::Impl::hit(const Point& p) const
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
    const float bar = d_->bar_height();
    const Rect area(0, bar, r.width, std::max(0.0f, r.height - bar));
    for (std::size_t i = 0; i < d_->tabs_.size(); ++i)
    {
        d_->tabs_[i]->d_->page_->bounds = area;
    }
    d_->scroll_to(d_->scroll_);
    if (d_->current_ >= 0)
    {
        d_->ensure_visible(d_->current_);
    }
}

Size TabWidget::compute_preferred_size() const
{
    // どのページを選んでも収まる大きさ。
    Size s(0, 0);
    for (std::size_t i = 0; i < d_->tabs_.size(); ++i)
    {
        const Size p = d_->tabs_[i]->d_->page_->preferred_size;
        s.width = std::max(s.width, p.width);
        s.height = std::max(s.height, p.height);
    }
    return Size(std::max(s.width, d_->total_width()), s.height + d_->bar_height());
}

Size TabWidget::compute_minimum_size() const
{
    // 見出しは横にスクロールできるので、幅はページの最小の幅で決める。
    Size s(0, 0);
    for (std::size_t i = 0; i < d_->tabs_.size(); ++i)
    {
        const Size m = d_->tabs_[i]->d_->page_->minimum_hint;
        s.width = std::max(s.width, m.width);
        s.height = std::max(s.height, m.height);
    }
    return Size(std::max(s.width, MIN_TAB + ARROW_WIDTH * 2), s.height + d_->bar_height());
}

void TabWidget::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const float bar = d_->bar_height();
    const bool enabled_now = effectively_enabled;
    canvas.fill_rect(Rect(0, bar - 1, r.width, 1), theme::border());

    const float view = d_->visible_width();
    canvas.save();
    canvas.clip_rect(Rect(0, 0, view, bar));
    float x = -d_->scroll_;
    for (std::size_t i = 0; i < d_->tabs_.size(); ++i)
    {
        const Tab* tab = d_->tabs_[i];
        const float w = d_->tab_width(tab);
        if (x + w > 0 && x < view)
        {
            const bool selected = static_cast<int>(i) == d_->current_;
            const bool usable = enabled_now && tab->d_->enabled_;
            const Rect cell(x, 0, w, bar);
            if (usable && static_cast<int>(i) == d_->hover_ && !selected)
            {
                canvas.fill_round_rect(cell.adjusted(2, 4, 2, 4), theme::field_radius(), theme::selection_inactive());
            }
            const Color fg = !usable ? theme::text_disabled() : selected ? theme::accent() : theme::color(ColorRole::TEXT_SECONDARY);
            const TextLayout& l = d_->title_layout(tab);
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

    if (d_->overflowing())
    {
        // ◀▶: 端まで動かしたら薄くする。
        const float limit = std::max(0.0f, d_->total_width() - view);
        const float cy = bar / 2;
        for (int k = 0; k < 2; ++k)
        {
            const float cx = view + ARROW_WIDTH * k + ARROW_WIDTH / 2;
            const bool can = k == 0 ? d_->scroll_ > 0 : d_->scroll_ < limit;
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
    const int h = d_->hit(e.pos);
    if (h == -1)
    {
        return;
    }
    e.accepted = true;
    if (h == HIT_LEFT)
    {
        d_->scroll_to(d_->scroll_ - SCROLL_STEP);
    }
    else if (h == HIT_RIGHT)
    {
        d_->scroll_to(d_->scroll_ + SCROLL_STEP);
    }
    else if (d_->tabs_[h]->d_->enabled_)
    {
        current_index = h;
    }
}

void TabWidget::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    const int h = d_->hit(e.pos);
    const int next = h >= 0 ? h : -1;
    if (next != d_->hover_)
    {
        d_->hover_ = next;
        invalidate();
    }
}

void TabWidget::mouse_leave()
{
    if (d_->hover_ != -1)
    {
        d_->hover_ = -1;
        invalidate();
    }
    Widget::mouse_leave();
}

void TabWidget::mouse_wheel(WheelEvent& e)
{
    Widget::mouse_wheel(e);
    if (d_->hit(e.pos) == -1 || !d_->overflowing())
    {
        return;  // 見出しの並びの外・スクロールできないときは親へ送る
    }
    e.accepted = true;
    const float ticks = e.delta.x != 0 ? e.delta.x : -e.delta.y;
    d_->scroll_to(d_->scroll_ + ticks * SCROLL_STEP / 2);
}

void TabWidget::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    const int n = static_cast<int>(d_->tabs_.size());
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
        const int i = e.key == Key::HOME ? d_->next_enabled(0, 1) : d_->next_enabled(n - 1, -1);
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
    int i = d_->current_;
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
        if (d_->tabs_[i]->d_->enabled_)
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
    for (std::size_t i = 0; i < d_->tabs_.size(); ++i)
    {
        d_->tabs_[i]->d_->layout_ = TextLayout();
    }
}

}  // namespace tokotachi
