#include <tokotachi/layout/place_layout.hpp>

#include <algorithm>

#include <tokotachi/widgets/widget.hpp>

namespace tokotachi
{

PlaceLayout::PlaceLayout(Widget* target) : Layout(target) {}

void PlaceLayout::add(Widget* w, const Placement& placement)
{
    if (!adopt(w))
    {
        return;
    }
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        if (items_[i].widget == w)
        {
            items_[i].placement = placement;
            request_layout();
            return;
        }
    }
    const Item item = {w, placement};
    items_.push_back(item);
    request_layout();
}

void PlaceLayout::widget_removed(Widget* w)
{
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        if (items_[i].widget == w)
        {
            items_.erase(items_.begin() + static_cast<std::ptrdiff_t>(i));
            return;
        }
    }
}

Size PlaceLayout::compute_preferred_size() const
{
    // 割合で置く項目は親の大きさに従うので、絶対の位置・大きさで置く項目だけから決める。
    Size s;
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        const Item& it = items_[i];
        const Placement& p = it.placement;
        if (!is_managed_visible(it.widget) || p.relative_x != 0 || p.relative_y != 0)
        {
            continue;
        }
        const Size pref = preferred_of(it.widget);
        s.width = std::max(s.width, p.x + (p.width > 0 ? p.width : pref.width));
        s.height = std::max(s.height, p.y + (p.height > 0 ? p.height : pref.height));
    }
    return s;
}

Size PlaceLayout::compute_minimum_size() const { return Size(); }

void PlaceLayout::arrange(const Rect& area)
{
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        const Item& it = items_[i];
        if (!is_managed_visible(it.widget))
        {
            continue;
        }
        const Placement& p = it.placement;
        const Size pref = preferred_of(it.widget);
        float w = p.width + p.relative_width * area.width;
        float h = p.height + p.relative_height * area.height;
        if (p.width == 0 && p.relative_width == 0)
        {
            w = pref.width;
        }
        if (p.height == 0 && p.relative_height == 0)
        {
            h = pref.height;
        }
        float x = area.x + p.x + p.relative_x * area.width;
        float y = area.y + p.y + p.relative_y * area.height;
        // アンカーの点が (x, y) に来るようにずらす。
        const int a = static_cast<int>(p.anchor);
        const int column = a % 3, row = a / 3;  // TOP_LEFT, TOP, TOP_RIGHT, LEFT, ... の並び
        x -= w * static_cast<float>(column) / 2;
        y -= h * static_cast<float>(row) / 2;
        place_in_cell(it.widget, Rect(x, y, w, h), Align::FILL, Align::FILL);
    }
}

}  // namespace tokotachi
