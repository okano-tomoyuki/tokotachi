#include "popup_list.hpp"

#include <algorithm>
#include <cmath>

#include <gui/graphics/canvas.hpp>

#include "theme.hpp"

namespace gui
{
namespace detail
{

namespace
{
const float PADDING_X = 10;
const float PADDING_Y = 6;
}

PopupList::PopupList(Widget* parent_widget) : Widget(parent_widget), highlighted_(-1), pressed_(false)
{
    focus_policy = FocusPolicy::CLICK;
}

void PopupList::set_items(const std::vector<std::string>& items, const Font& font)
{
    items_ = items;
    font_ = font;
    layouts_.clear();
    highlighted_ = -1;
    request_layout();
    invalidate();
}

const TextLayout& PopupList::layout(std::size_t i) const
{
    const float scale = display_scale;
    if (layouts_.size() != items_.size())
    {
        layouts_.assign(items_.size(), TextLayout());
    }
    if (layouts_[i].is_null() || layouts_[i].scale() != scale)
    {
        layouts_[i] = TextLayout(items_[i], font_, scale);
    }
    return layouts_[i];
}

float PopupList::row_height() const
{
    return std::ceil(TextLayout("", font_, display_scale).height()) + PADDING_Y * 2;
}

void PopupList::highlight(int index)
{
    if (index < -1 || index >= static_cast<int>(items_.size()))
    {
        return;
    }
    highlighted_ = index;
    invalidate();
    if (index >= 0 && on_reveal)
    {
        const float h = row_height();
        on_reveal(Rect(0, index * h, width, h));
    }
}

int PopupList::row_at(float y) const
{
    const int i = static_cast<int>(std::floor(y / row_height()));
    return i >= 0 && i < static_cast<int>(items_.size()) ? i : -1;
}

Size PopupList::compute_preferred_size() const
{
    float w = 0;
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        w = std::max(w, layout(i).width());
    }
    return Size(std::ceil(w) + PADDING_X * 2, row_height() * static_cast<float>(items_.size()));
}

void PopupList::paint(Canvas& canvas)
{
    const float h = row_height();
    const float w = width;
    // 見えている行だけ描く。
    const Rect clip = canvas.clip_bounds();
    const int first = std::max(0, static_cast<int>(std::floor(clip.y / h)));
    const int last = std::min(static_cast<int>(items_.size()) - 1, static_cast<int>(std::ceil(clip.bottom() / h)));
    for (int i = first; i <= last; ++i)
    {
        const Rect row(0, i * h, w, h);
        const bool hot = i == highlighted_;
        if (hot)
        {
            canvas.fill_rect(row, theme::selection());
        }
        const TextLayout& l = layout(static_cast<std::size_t>(i));
        canvas.draw_text(l, Point(PADDING_X, row.y + (h - l.height()) / 2), hot ? theme::on_selection() : theme::text());
    }
    Widget::paint(canvas);
}

void PopupList::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    const int i = row_at(e.pos.y);
    if (i >= 0 && i != highlighted_)
    {
        highlighted_ = i;  // ポインタの下の項目を強調する(スクロールはしない)
        invalidate();
    }
}

void PopupList::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button == MouseButton::LEFT)
    {
        e.accepted = true;
        pressed_ = true;
    }
}

void PopupList::mouse_up(MouseEvent& e)
{
    Widget::mouse_up(e);
    if (e.button != MouseButton::LEFT || !pressed_)
    {
        return;
    }
    e.accepted = true;
    pressed_ = false;
    const int i = row_at(e.pos.y);
    if (i >= 0 && on_choose)
    {
        on_choose(i);
    }
}

void PopupList::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    e.accepted = true;
    const int n = static_cast<int>(items_.size());
    const int page = std::max(1, static_cast<int>(height / row_height()) - 1);
    switch (e.key)
    {
    case Key::UP:
        highlight(std::max(0, highlighted_ - 1));
        break;
    case Key::DOWN:
        highlight(std::min(n - 1, highlighted_ + 1));
        break;
    case Key::PAGE_UP:
        highlight(std::max(0, highlighted_ - page));
        break;
    case Key::PAGE_DOWN:
        highlight(std::min(n - 1, highlighted_ + page));
        break;
    case Key::HOME:
        highlight(n > 0 ? 0 : -1);
        break;
    case Key::END:
        highlight(n - 1);
        break;
    case Key::RETURN:
    case Key::SPACE:
        if (highlighted_ >= 0 && on_choose)
        {
            on_choose(highlighted_);
        }
        break;
    case Key::ESCAPE:
        if (on_cancel)
        {
            on_cancel();
        }
        break;
    default:
        e.accepted = false;
        break;
    }
}

void PopupList::scale_changed() { layouts_.clear(); }

}  // namespace detail
}  // namespace gui
