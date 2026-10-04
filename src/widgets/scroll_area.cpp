#include <gui/widgets/scroll_area.hpp>

#include <algorithm>

#include "../core/property_impl.hpp"

namespace gui
{

ScrollArea::ScrollArea(Widget* parent_widget)
    : Widget(parent_widget),
      GUI_PROPERTY_GETTER(ScrollArea, Widget*, content),
      GUI_PROPERTY_GETTER(ScrollArea, bool, fit_width),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(ScrollArea, Point, scroll_position),
      viewport_(new Widget(this)),
      vertical_(new ScrollBar(this, Orientation::VERTICAL)),
      horizontal_(new ScrollBar(this, Orientation::HORIZONTAL)),
      content_(nullptr),
      fit_width_(true)
{
    vertical_->visible = false;
    horizontal_->visible = false;
    const NotifyHandler moved = [this](Object*)
    {
        place_content();
        notify_changed(scroll_position);
    };
    vertical_->on_change = moved;
    horizontal_->on_change = moved;
}

Widget* ScrollArea::get_content() const
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

void ScrollArea::set_content(Widget* const& v)
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
    mark_layout_dirty();
    request_layout();
}

void ScrollArea::set_fit_width(const bool& v)
{
    fit_width_ = v;
    mark_layout_dirty();
}

Point ScrollArea::get_scroll_position() const { return Point(horizontal_->value, vertical_->value); }

void ScrollArea::set_scroll_position(const Point& p)
{
    // スクロールバーの on_change が中身を動かし、scroll_position の変更を知らせる。
    horizontal_->value = p.x;
    vertical_->value = p.y;
}

void ScrollArea::scroll_to_visible(const Rect& r)
{
    const Rect view = viewport_->local_rect;
    Point p = get_scroll_position();
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
    set_scroll_position(p);
}

void ScrollArea::place_content()
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
    Widget* c = get_content();
    const Size pref = c ? c->preferred_size.get() : Size();

    // 収まらない向きにだけスクロールバーを出す。片方を出すと見えている範囲が狭まり、もう片方が要ることがあるので 2 回確かめる。
    float view_w = area.width, view_h = area.height;
    bool need_v = false, need_h = false;
    for (int i = 0; i < 2; ++i)
    {
        const float content_w = fit_width_ ? view_w : std::max(pref.width, view_w);
        need_v = pref.height > view_h;
        need_h = !fit_width_ && content_w > view_w;
        view_w = area.width - (need_v ? bar : 0);
        view_h = area.height - (need_h ? bar : 0);
    }
    const float content_w = fit_width_ ? view_w : std::max(pref.width, view_w);
    const float content_h = std::max(pref.height, view_h);

    viewport_->bounds = Rect(0, 0, view_w, view_h);
    vertical_->visible = need_v;
    horizontal_->visible = need_h;
    vertical_->bounds = Rect(view_w, 0, bar, view_h);
    horizontal_->bounds = Rect(0, view_h, view_w, bar);
    vertical_->page_step = view_h;
    horizontal_->page_step = view_w;
    vertical_->maximum = need_v ? content_h - view_h : 0;
    horizontal_->maximum = need_h ? content_w - view_w : 0;
    if (c)
    {
        c->bounds = Rect(-horizontal_->value.get(), -vertical_->value.get(), content_w, content_h);
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
    const bool sideways = e.has_modifier(MODIFIER_SHIFT) || e.delta.x != 0 || vertical_->maximum.get() <= 0;
    ScrollBar* bar = sideways ? horizontal_ : vertical_;
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
    Widget* c = get_content();
    return c ? c->preferred_size.get() : Size();
}

Size ScrollArea::compute_minimum_size() const
{
    const float bar = ScrollBar::THICKNESS;
    return Size(bar * 3, bar * 3);
}

}  // namespace gui
