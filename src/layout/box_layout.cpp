#include <tokotachi/layout/box_layout.hpp>

#include <algorithm>

#include <tokotachi/widgets/widget.hpp>

#include "../core/impl.hpp"
#include "distribute.hpp"

namespace tokotachi
{

class BoxLayout::Impl
{
    TOKOTACHI_IMPL(BoxLayout);

    explicit Impl(BoxLayout* object) : q(object) {}

    struct Item
    {
        Widget* widget;  ///< nullptr なら空き
        float space;     ///< 固定の空き(widget が nullptr のとき)
        int stretch;
        Align align;
    };

    Orientation get_orientation() const { return orientation_; }
    void set_orientation(const Orientation& v);
    float get_spacing() const { return spacing_; }
    void set_spacing(const float& v);
    Item* find(Widget* w);
    /// 並べる向きの長さ・直交する向きの長さを、大きさから取り出す。
    float main_of(const Size& s) const;
    float cross_of(const Size& s) const;
    Size total(bool minimum) const;

    Orientation orientation_;
    float spacing_;
    std::vector<Item> items_;
};

BoxLayout::BoxLayout(Widget* target, Orientation initial_orientation)
    : Layout(target),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, Orientation, orientation),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, float, spacing),
      d_(new Impl(this))
{
    d_->orientation_ = initial_orientation;
    d_->spacing_ = 8;
}

BoxLayout::~BoxLayout() { delete d_; }

void BoxLayout::Impl::set_orientation(const Orientation& v)
{
    orientation_ = v;
    q->request_layout();
}

void BoxLayout::Impl::set_spacing(const float& v)
{
    spacing_ = v;
    q->request_layout();
}

BoxLayout::Impl::Item* BoxLayout::Impl::find(Widget* w)
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
    Impl::Item* item = d_->find(w);
    if (!item)
    {
        const Impl::Item it = {w, 0, stretch, cross_alignment};
        d_->items_.push_back(it);
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
    const Impl::Item it = {nullptr, size, 0, Align::FILL};
    d_->items_.push_back(it);
    request_layout();
}

void BoxLayout::add_stretch(int stretch)
{
    const Impl::Item it = {nullptr, 0, stretch, Align::FILL};
    d_->items_.push_back(it);
    request_layout();
}

void BoxLayout::set_stretch(Widget* w, int stretch)
{
    if (Impl::Item* item = d_->find(w))
    {
        item->stretch = stretch;
        request_layout();
    }
}

void BoxLayout::set_alignment(Widget* w, Align cross_alignment)
{
    if (Impl::Item* item = d_->find(w))
    {
        item->align = cross_alignment;
        request_layout();
    }
}

void BoxLayout::widget_removed(Widget* w)
{
    for (std::size_t i = 0; i < d_->items_.size(); ++i)
    {
        if (d_->items_[i].widget == w)
        {
            d_->items_.erase(d_->items_.begin() + static_cast<std::ptrdiff_t>(i));
            return;
        }
    }
}

float BoxLayout::Impl::main_of(const Size& s) const { return orientation_ == Orientation::HORIZONTAL ? s.width : s.height; }

float BoxLayout::Impl::cross_of(const Size& s) const { return orientation_ == Orientation::HORIZONTAL ? s.height : s.width; }

Size BoxLayout::Impl::total(bool minimum) const
{
    float main = 0, cross = 0;
    int count = 0;
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        const Item& it = items_[i];
        if (it.widget && !q->is_managed_visible(it.widget))
        {
            continue;
        }
        ++count;
        if (!it.widget)
        {
            main += it.space;
            continue;
        }
        const Size s = minimum ? q->minimum_of(it.widget) : q->preferred_of(it.widget);
        main += main_of(s);
        cross = std::max(cross, cross_of(s));
    }
    if (count > 1)
    {
        main += spacing_ * static_cast<float>(count - 1);
    }
    return orientation_ == Orientation::HORIZONTAL ? Size(main, cross) : Size(cross, main);
}

Size BoxLayout::compute_preferred_size() const { return d_->total(false); }

Size BoxLayout::compute_minimum_size() const { return d_->total(true); }

void BoxLayout::arrange(const Rect& area)
{
    const bool horizontal = d_->orientation_ == Orientation::HORIZONTAL;
    std::vector<const Impl::Item*> shown;
    std::vector<float> sizes, minimums, maximums;
    std::vector<int> stretches;
    for (std::size_t i = 0; i < d_->items_.size(); ++i)
    {
        const Impl::Item& it = d_->items_[i];
        if (it.widget && !is_managed_visible(it.widget))
        {
            continue;
        }
        shown.push_back(&it);
        if (it.widget)
        {
            sizes.push_back(d_->main_of(preferred_of(it.widget)));
            minimums.push_back(d_->main_of(minimum_of(it.widget)));
            maximums.push_back(d_->main_of(maximum_of(it.widget)));
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
    const float gaps = d_->spacing_ * static_cast<float>(shown.size() - 1);
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
        pos += sizes[i] + d_->spacing_;
    }
}

}  // namespace tokotachi
