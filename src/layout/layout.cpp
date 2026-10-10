#include <tokotachi/layout/layout.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>

#include <tokotachi/widgets/widget.hpp>

#include "../core/property_impl.hpp"
#include "../widgets/widget_access.hpp"

namespace tokotachi
{

Layout::Layout(Widget* target)
    : Object(target),
      TOKOTACHI_READONLY_PROPERTY(Layout, Widget*, widget),
      TOKOTACHI_PROPERTY_MEMBER(Layout, Margins, margins),
      target_(target)
{
    if (!target_)
    {
        return;
    }
    // 1 つのウィジェットに付くレイアウトは 1 つだけ。古いレイアウトは破棄する(子のウィジェットは破棄しない)。
    Layout* old = detail::WidgetAccess::layout_of(target_);
    detail::WidgetAccess::set_layout(target_, this);
    delete old;
    request_layout();
}

Layout::~Layout()
{
    if (target_ && detail::WidgetAccess::layout_of(target_) == this)
    {
        detail::WidgetAccess::set_layout(target_, nullptr);
        target_->request_layout();
    }
}

void Layout::set_margins(const Margins& v)
{
    margins_ = v;
    request_layout();
}

void Layout::remove(Widget* w)
{
    widget_removed(w);
    request_layout();
}

void Layout::request_layout()
{
    if (target_)
    {
        target_->request_layout();
    }
}

bool Layout::adopt(Widget* w)
{
    if (!w || !target_)
    {
        return false;
    }
    if (detail::WidgetAccess::parent_of(w) != target_)
    {
        w->parent = target_;  // ウィンドウ・target の祖先は付け替えられない(set_parent が拒否する)
    }
    if (detail::WidgetAccess::parent_of(w) != target_)
    {
        std::fprintf(stderr, "tokotachi: Layout::add: the widget cannot be placed in this layout (ignored)\n");
        return false;
    }
    return true;
}

bool Layout::is_managed_visible(const Widget* w) { return detail::WidgetAccess::displayed(w); }

Size Layout::preferred_of(const Widget* w) { return is_managed_visible(w) ? w->preferred_size.get() : Size(); }

Size Layout::minimum_of(const Widget* w) { return is_managed_visible(w) ? w->minimum_hint.get() : Size(); }

Size Layout::maximum_of(const Widget* w) { return w->maximum_size; }

namespace
{

// 1 つの向きで、セルの中の位置と長さを決める。
void align_in(float cell_pos, float cell_len, float preferred, float maximum, Align a, float& pos, float& len)
{
    len = a == Align::FILL ? std::min(cell_len, maximum) : std::min(preferred, cell_len);
    switch (a)
    {
    case Align::START:
    case Align::FILL:
        pos = cell_pos;
        break;
    case Align::CENTER:
        pos = cell_pos + (cell_len - len) / 2;
        break;
    case Align::END:
        pos = cell_pos + cell_len - len;
        break;
    }
}

}  // namespace

void Layout::place_in_cell(Widget* w, const Rect& cell, Align horizontal, Align vertical) const
{
    const Size pref = preferred_of(w), max = maximum_of(w);
    float x = 0, y = 0, width = 0, height = 0;
    align_in(cell.x, cell.width, pref.width, max.width, horizontal, x, width);
    align_in(cell.y, cell.height, pref.height, max.height, vertical, y, height);
    // 辺をピクセルの整数に揃え、分数の倍率でも境界がにじまないようにする(docs/adr/0008)。
    const float s = target_ ? target_->display_scale.get() : 1.0f;
    const float x0 = std::floor(x * s + 0.5f) / s, y0 = std::floor(y * s + 0.5f) / s;
    const float x1 = std::floor((x + width) * s + 0.5f) / s, y1 = std::floor((y + height) * s + 0.5f) / s;
    w->bounds = Rect(x0, y0, std::max(0.0f, x1 - x0), std::max(0.0f, y1 - y0));
}

}  // namespace tokotachi
