#include <tokotachi/layout/box_layout.hpp>

#include <algorithm>

#include <tokotachi/widgets/widget.hpp>

#include "../core/property_impl.hpp"
#include "distribute.hpp"

namespace tokotachi
{

BoxLayout::BoxLayout(Widget* target, Orientation initial_orientation)
    : Layout(target),
      TOKOTACHI_PROPERTY_GETTER(BoxLayout, Orientation, orientation),
      TOKOTACHI_PROPERTY_GETTER(BoxLayout, float, spacing),
      orientation_(initial_orientation),
      spacing_(8)
{
}

void BoxLayout::set_orientation(const Orientation& v)
{
    orientation_ = v;
    request_layout();
}

void BoxLayout::set_spacing(const float& v)
{
    spacing_ = v;
    request_layout();
}

BoxLayout::Item* BoxLayout::find(Widget* w)
{
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        if (items_[i].widget == w)
        {
            return &items_[i];
        }
    }
    return nullptr;
}

void BoxLayout::add(Widget* w, int stretch, Align cross_alignment)
{
    if (!adopt(w))
    {
        return;
    }
    Item* item = find(w);
    if (!item)
    {
        const Item it = {w, 0, stretch, cross_alignment};
        items_.push_back(it);
    }
    else
    {
        item->stretch = stretch;
        item->align = cross_alignment;
    }
    request_layout();
}

void BoxLayout::add_spacing(float size)
{
    const Item it = {nullptr, size, 0, Align::FILL};
    items_.push_back(it);
    request_layout();
}

void BoxLayout::add_stretch(int stretch)
{
    const Item it = {nullptr, 0, stretch, Align::FILL};
    items_.push_back(it);
    request_layout();
}

void BoxLayout::set_stretch(Widget* w, int stretch)
{
    if (Item* item = find(w))
    {
        item->stretch = stretch;
        request_layout();
    }
}

void BoxLayout::set_alignment(Widget* w, Align cross_alignment)
{
    if (Item* item = find(w))
    {
        item->align = cross_alignment;
        request_layout();
    }
}

void BoxLayout::widget_removed(Widget* w)
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

float BoxLayout::main_of(const Size& s) const { return orientation_ == Orientation::HORIZONTAL ? s.width : s.height; }

float BoxLayout::cross_of(const Size& s) const { return orientation_ == Orientation::HORIZONTAL ? s.height : s.width; }

Size BoxLayout::total(bool minimum) const
{
    float main = 0, cross = 0;
    int count = 0;
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        const Item& it = items_[i];
        if (it.widget && !is_managed_visible(it.widget))
        {
            continue;
        }
        ++count;
        if (!it.widget)
        {
            main += it.space;
            continue;
        }
        const Size s = minimum ? minimum_of(it.widget) : preferred_of(it.widget);
        main += main_of(s);
        cross = std::max(cross, cross_of(s));
    }
    if (count > 1)
    {
        main += spacing_ * static_cast<float>(count - 1);
    }
    return orientation_ == Orientation::HORIZONTAL ? Size(main, cross) : Size(cross, main);
}

Size BoxLayout::compute_preferred_size() const { return total(false); }

Size BoxLayout::compute_minimum_size() const { return total(true); }

void BoxLayout::arrange(const Rect& area)
{
    const bool horizontal = orientation_ == Orientation::HORIZONTAL;
    std::vector<const Item*> shown;
    std::vector<float> sizes, minimums, maximums;
    std::vector<int> stretches;
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        const Item& it = items_[i];
        if (it.widget && !is_managed_visible(it.widget))
        {
            continue;
        }
        shown.push_back(&it);
        if (it.widget)
        {
            sizes.push_back(main_of(preferred_of(it.widget)));
            minimums.push_back(main_of(minimum_of(it.widget)));
            maximums.push_back(main_of(maximum_of(it.widget)));
        }
        else
        {
            sizes.push_back(it.space);
            minimums.push_back(it.space);
            maximums.push_back(it.stretch > 0 ? 1e9f : it.space);
        }
        stretches.push_back(it.stretch);
    }
    if (shown.empty())
    {
        return;
    }
    const float gaps = spacing_ * static_cast<float>(shown.size() - 1);
    const float available = (horizontal ? area.width : area.height) - gaps;
    detail::distribute(sizes, minimums, maximums, stretches, available);

    float pos = horizontal ? area.x : area.y;
    for (std::size_t i = 0; i < shown.size(); ++i)
    {
        if (Widget* w = shown[i]->widget)
        {
            if (horizontal)
            {
                place_in_cell(w, Rect(pos, area.y, sizes[i], area.height), Align::FILL, shown[i]->align);
            }
            else
            {
                place_in_cell(w, Rect(area.x, pos, area.width, sizes[i]), shown[i]->align, Align::FILL);
            }
        }
        pos += sizes[i] + spacing_;
    }
}

}  // namespace tokotachi
