#include <tokotachi/widgets/scroll_area.hpp>

#include <algorithm>

#include "widget_access.hpp"
#include "../core/impl.hpp"

namespace tokotachi
{

class ScrollArea::Impl
{
    TOKOTACHI_IMPL(ScrollArea);

    explicit Impl(ScrollArea* object) : q(object) {}

    Widget* get_content() const;
    void set_content(Widget* const& v);
    bool get_fit_width() const { return fit_width_; }
    void set_fit_width(const bool& v);
    Point get_scroll_position() const;
    void set_scroll_position(const Point& p);
    /// スクロールバーの値に合わせて中身を動かす。
    void place_content();

    Widget* viewport_;
    ScrollBar* vertical_;
    ScrollBar* horizontal_;
    Widget* content_;
    bool fit_width_;
};

ScrollArea::ScrollArea(Widget* parent_widget)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, Widget*, content),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, fit_width),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, Point, scroll_position),
      d_(new Impl(this))
{
    d_->viewport_ = new Widget(this);
    d_->vertical_ = new ScrollBar(this, Orientation::VERTICAL);
    d_->horizontal_ = new ScrollBar(this, Orientation::HORIZONTAL);
    d_->content_ = nullptr;
    d_->fit_width_ = true;
    d_->vertical_->visible = false;
    d_->horizontal_->visible = false;
    const NotifyHandler moved = [this](Object*)
    {
        d_->place_content();
        notify_changed(scroll_position);
    };
    d_->vertical_->on_change = moved;
    d_->horizontal_->on_change = moved;
}

ScrollArea::~ScrollArea() { delete d_; }

Widget* ScrollArea::Impl::get_content() const
{
    // 中身が利用者に破棄されていたら、無いものとして扱う。
    if (content_)
    {
        const std::vector<Widget*> children = viewport_->children;
        if (std::find(children.begin(), children.end(), content_) == children.end())
        {
            return nullptr;
        }
    }
    return content_;
}

void ScrollArea::Impl::set_content(Widget* const& v)
{
    Widget* old = get_content();
    if (v == old)
    {
        return;
    }
    content_ = v;
    if (v)
    {
        v->parent = viewport_;
    }
    delete old;
    detail::WidgetAccess::mark_layout_dirty(q);
    q->request_layout();
}

void ScrollArea::Impl::set_fit_width(const bool& v)
{
    fit_width_ = v;
    detail::WidgetAccess::mark_layout_dirty(q);
}

Point ScrollArea::Impl::get_scroll_position() const { return Point(horizontal_->value, vertical_->value); }

void ScrollArea::Impl::set_scroll_position(const Point& p)
{
    // スクロールバーの on_change が中身を動かし、scroll_position の変更を知らせる。
    horizontal_->value = p.x;
    vertical_->value = p.y;
}

void ScrollArea::scroll_to_visible(const Rect& r)
{
    const Rect view = d_->viewport_->local_rect;
    Point p = d_->get_scroll_position();
    if (r.right() > p.x + view.width)
    {
        p.x = r.right() - view.width;
    }
    if (r.x < p.x)
    {
        p.x = r.x;
    }
    if (r.bottom() > p.y + view.height)
    {
        p.y = r.bottom() - view.height;
    }
    if (r.y < p.y)
    {
        p.y = r.y;
    }
    d_->set_scroll_position(p);
}

void ScrollArea::Impl::place_content()
{
    if (Widget* c = get_content())
    {
        c->x = -horizontal_->value.get();
        c->y = -vertical_->value.get();
    }
}

void ScrollArea::arrange_children()
{
    const Rect area = local_rect;
    const float bar = ScrollBar::THICKNESS;
    Widget* c = d_->get_content();
    const Size pref = c ? c->preferred_size.get() : Size();

    // 収まらない向きにだけスクロールバーを出す。片方を出すと見えている範囲が狭まり、もう片方が要ることがあるので 2 回確かめる。
    float view_w = area.width, view_h = area.height;
    bool need_v = false, need_h = false;
    for (int i = 0; i < 2; ++i)
    {
        const float content_w = d_->fit_width_ ? view_w : std::max(pref.width, view_w);
        need_v = pref.height > view_h;
        need_h = !d_->fit_width_ && content_w > view_w;
        view_w = area.width - (need_v ? bar : 0);
        view_h = area.height - (need_h ? bar : 0);
    }
    const float content_w = d_->fit_width_ ? view_w : std::max(pref.width, view_w);
    const float content_h = std::max(pref.height, view_h);

    d_->viewport_->bounds = Rect(0, 0, view_w, view_h);
    d_->vertical_->visible = need_v;
    d_->horizontal_->visible = need_h;
    d_->vertical_->bounds = Rect(view_w, 0, bar, view_h);
    d_->horizontal_->bounds = Rect(0, view_h, view_w, bar);
    d_->vertical_->page_step = view_h;
    d_->horizontal_->page_step = view_w;
    d_->vertical_->maximum = need_v ? content_h - view_h : 0;
    d_->horizontal_->maximum = need_h ? content_w - view_w : 0;
    if (c)
    {
        c->bounds = Rect(-d_->horizontal_->value.get(), -d_->vertical_->value.get(), content_w, content_h);
    }
}

void ScrollArea::mouse_wheel(WheelEvent& e)
{
    Widget::mouse_wheel(e);
    if (e.accepted)
    {
        return;  // 利用者のハンドラが受け取った
    }
    // 縦に動かせれば縦(Shift を押していれば横)、縦に動かせなければ横。
    const bool sideways = e.has_modifier(MODIFIER_SHIFT) || e.delta.x != 0 || d_->vertical_->maximum.get() <= 0;
    ScrollBar* bar = sideways ? d_->horizontal_ : d_->vertical_;
    if (bar->maximum.get() <= 0)
    {
        return;  // 動かせなければ親へ送る
    }
    e.accepted = true;
    const float ticks = sideways ? (e.delta.x != 0 ? e.delta.x : -e.delta.y) : -e.delta.y;
    bar->value = bar->value.get() + ticks * bar->single_step.get();
}

Size ScrollArea::compute_preferred_size() const
{
    // 余裕があれば中身をすべて見せる大きさ。
    Widget* c = d_->get_content();
    return c ? c->preferred_size.get() : Size();
}

Size ScrollArea::compute_minimum_size() const
{
    const float bar = ScrollBar::THICKNESS;
    return Size(bar * 3, bar * 3);
}

}  // namespace tokotachi
