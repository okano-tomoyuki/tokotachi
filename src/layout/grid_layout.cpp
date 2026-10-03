#include <gui/grid_layout.hpp>

#include <algorithm>

#include <gui/widget.hpp>

#include "../core/property_impl.hpp"
#include "distribute.hpp"

namespace gui
{

GridLayout::GridLayout(Widget* target)
    : Layout(target),
      GUI_PROPERTY_GETTER(GridLayout, float, row_spacing),
      GUI_PROPERTY_GETTER(GridLayout, float, column_spacing),
      row_spacing_(8),
      column_spacing_(8)
{
}

void GridLayout::set_row_spacing(const float& v)
{
    row_spacing_ = v;
    request_layout();
}

void GridLayout::set_column_spacing(const float& v)
{
    column_spacing_ = v;
    request_layout();
}

void GridLayout::add(Widget* w, int row, int column, int row_span, int column_span, Align horizontal, Align vertical)
{
    if (!adopt(w) || row < 0 || column < 0)
    {
        return;
    }
    const Item item = {w, row, column, std::max(1, row_span), std::max(1, column_span), horizontal, vertical};
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        if (items_[i].widget == w)
        {
            items_[i] = item;
            request_layout();
            return;
        }
    }
    items_.push_back(item);
    request_layout();
}

GridLayout::Track& GridLayout::track(std::vector<Track>& tracks, int index)
{
    if (tracks.size() <= static_cast<std::size_t>(index))
    {
        const Track empty = {0, 0};
        tracks.resize(static_cast<std::size_t>(index) + 1, empty);
    }
    return tracks[static_cast<std::size_t>(index)];
}

void GridLayout::set_row_stretch(int row, int stretch)
{
    if (row >= 0)
    {
        track(rows_, row).stretch = stretch;
        request_layout();
    }
}

void GridLayout::set_column_stretch(int column, int stretch)
{
    if (column >= 0)
    {
        track(columns_, column).stretch = stretch;
        request_layout();
    }
}

void GridLayout::set_row_minimum(int row, float size)
{
    if (row >= 0)
    {
        track(rows_, row).minimum = size;
        request_layout();
    }
}

void GridLayout::set_column_minimum(int column, float size)
{
    if (column >= 0)
    {
        track(columns_, column).minimum = size;
        request_layout();
    }
}

void GridLayout::widget_removed(Widget* w)
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

int GridLayout::count(bool columns) const
{
    int n = static_cast<int>(columns ? columns_.size() : rows_.size());
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        const Item& it = items_[i];
        n = std::max(n, columns ? it.column + it.column_span : it.row + it.row_span);
    }
    return n;
}

namespace
{

// 複数の行(列)にまたがる項目に足りない分を、またがる行(列)に分ける。伸びる行(列)があればそこへ、無ければ均等に。
void spread(std::vector<float>& sizes, const std::vector<int>& stretches, int first, int span, float needed)
{
    if (needed <= 0)
    {
        return;
    }
    int weight = 0;
    for (int i = first; i < first + span; ++i)
    {
        weight += stretches[static_cast<std::size_t>(i)];
    }
    for (int i = first; i < first + span; ++i)
    {
        const float share = weight > 0 ? needed * stretches[static_cast<std::size_t>(i)] / weight : needed / span;
        sizes[static_cast<std::size_t>(i)] += share;
    }
}

}  // namespace

void GridLayout::measure(bool columns, std::vector<float>& preferred, std::vector<float>& minimum,
                         std::vector<bool>& used) const
{
    const int n = count(columns);
    const std::vector<Track>& tracks = columns ? columns_ : rows_;
    const float spacing = columns ? column_spacing_ : row_spacing_;
    preferred.assign(static_cast<std::size_t>(n), 0);
    minimum.assign(static_cast<std::size_t>(n), 0);
    used.assign(static_cast<std::size_t>(n), false);
    std::vector<int> stretches(static_cast<std::size_t>(n), 0);
    for (std::size_t i = 0; i < tracks.size(); ++i)
    {
        preferred[i] = minimum[i] = tracks[i].minimum;
        stretches[i] = tracks[i].stretch;
        used[i] = tracks[i].minimum > 0;
    }
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        const Item& it = items_[i];
        if (!is_managed_visible(it.widget))
        {
            continue;
        }
        const int first = columns ? it.column : it.row;
        const int span = columns ? it.column_span : it.row_span;
        for (int k = first; k < first + span; ++k)
        {
            used[static_cast<std::size_t>(k)] = true;
        }
    }
    // まず 1 つの行(列)に収まる項目、次にまたがる項目。
    for (int pass = 0; pass < 2; ++pass)
    {
        for (std::size_t i = 0; i < items_.size(); ++i)
        {
            const Item& it = items_[i];
            if (!is_managed_visible(it.widget))
            {
                continue;
            }
            const int first = columns ? it.column : it.row;
            const int span = columns ? it.column_span : it.row_span;
            if ((span == 1) != (pass == 0))
            {
                continue;
            }
            const Size p = preferred_of(it.widget), m = minimum_of(it.widget);
            const float want_p = columns ? p.width : p.height, want_m = columns ? m.width : m.height;
            if (span == 1)
            {
                const std::size_t k = static_cast<std::size_t>(first);
                preferred[k] = std::max(preferred[k], want_p);
                minimum[k] = std::max(minimum[k], want_m);
                continue;
            }
            float have_p = spacing * static_cast<float>(span - 1), have_m = have_p;
            for (int k = first; k < first + span; ++k)
            {
                have_p += preferred[static_cast<std::size_t>(k)];
                have_m += minimum[static_cast<std::size_t>(k)];
            }
            spread(preferred, stretches, first, span, want_p - have_p);
            spread(minimum, stretches, first, span, want_m - have_m);
        }
    }
}

namespace
{

// 使われている行(列)の数 - 1 個の間隔。
float gaps_of(const std::vector<bool>& used, float spacing)
{
    int n = 0;
    for (std::size_t i = 0; i < used.size(); ++i)
    {
        n += used[i] ? 1 : 0;
    }
    return n > 1 ? spacing * static_cast<float>(n - 1) : 0;
}

float sum_with_spacing(const std::vector<float>& sizes, const std::vector<bool>& used, float spacing)
{
    float total = 0;
    for (std::size_t i = 0; i < sizes.size(); ++i)
    {
        total += sizes[i];
    }
    return total + gaps_of(used, spacing);
}

}  // namespace

Size GridLayout::compute_preferred_size() const
{
    std::vector<float> cp, cm, rp, rm;
    std::vector<bool> cu, ru;
    measure(true, cp, cm, cu);
    measure(false, rp, rm, ru);
    return Size(sum_with_spacing(cp, cu, column_spacing_), sum_with_spacing(rp, ru, row_spacing_));
}

Size GridLayout::compute_minimum_size() const
{
    std::vector<float> cp, cm, rp, rm;
    std::vector<bool> cu, ru;
    measure(true, cp, cm, cu);
    measure(false, rp, rm, ru);
    return Size(sum_with_spacing(cm, cu, column_spacing_), sum_with_spacing(rm, ru, row_spacing_));
}

void GridLayout::arrange(const Rect& area)
{
    std::vector<float> positions[2], sizes[2];
    for (int d = 0; d < 2; ++d)
    {
        const bool columns = d == 0;
        std::vector<float> preferred, minimum;
        std::vector<bool> used;
        measure(columns, preferred, minimum, used);
        const std::vector<Track>& tracks = columns ? columns_ : rows_;
        const float spacing = columns ? column_spacing_ : row_spacing_;
        std::vector<int> stretches(preferred.size(), 0);
        for (std::size_t i = 0; i < tracks.size() && i < stretches.size(); ++i)
        {
            stretches[i] = tracks[i].stretch;
        }
        // 使われていない行(列)は伸ばさず、大きさ 0 のままにする。
        std::vector<float> maximums(preferred.size(), 1e9f);
        for (std::size_t i = 0; i < used.size(); ++i)
        {
            if (!used[i])
            {
                maximums[i] = 0;
                stretches[i] = 0;
            }
        }
        detail::distribute(preferred, minimum, maximums, stretches,
                           (columns ? area.width : area.height) - gaps_of(used, spacing));
        sizes[d] = preferred;
        // 間隔は、使われている行(列)の間にだけ入れる。
        float pos = columns ? area.x : area.y;
        bool any = false;
        for (std::size_t i = 0; i < preferred.size(); ++i)
        {
            if (used[i] && any)
            {
                pos += spacing;
            }
            positions[d].push_back(pos);
            pos += preferred[i];
            any = any || used[i];
        }
    }
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        const Item& it = items_[i];
        if (!is_managed_visible(it.widget))
        {
            continue;
        }
        const std::size_t c0 = static_cast<std::size_t>(it.column), c1 = c0 + static_cast<std::size_t>(it.column_span) - 1;
        const std::size_t r0 = static_cast<std::size_t>(it.row), r1 = r0 + static_cast<std::size_t>(it.row_span) - 1;
        const float x = positions[0][c0], y = positions[1][r0];
        const Rect cell(x, y, positions[0][c1] + sizes[0][c1] - x, positions[1][r1] + sizes[1][r1] - y);
        place_in_cell(it.widget, cell, it.horizontal, it.vertical);
    }
}

}  // namespace gui
