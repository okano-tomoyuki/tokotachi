#include <tokotachi/widgets/combo_box.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/layout/box_layout.hpp>
#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/widgets/scroll_area.hpp>

#include "../core/property_impl.hpp"
#include "popup_list.hpp"
#include "theme.hpp"

namespace tokotachi
{

namespace
{
const float ARROW_AREA = 24;     // 右の下向きの矢印の幅(dp)
const std::size_t MAX_ROWS = 10;  // 一覧に一度に見せる項目の数
}

ComboBox::ComboBox(Widget* parent_widget)
    : Widget(parent_widget),
      TOKOTACHI_PROPERTY_GETTER_SELF_NOTIFY(ComboBox, std::vector<std::string>, items),
      TOKOTACHI_PROPERTY_GETTER_SELF_NOTIFY(ComboBox, int, current_index),
      TOKOTACHI_READONLY_PROPERTY(ComboBox, std::string, current_text),
      TOKOTACHI_PROPERTY_MEMBER(ComboBox, std::string, placeholder_text),
      TOKOTACHI_PROPERTY_MEMBER(ComboBox, Font, font),
      TOKOTACHI_HANDLER_PROPERTY(ComboBox, NotifyHandler, on_change),
      current_index_(-1),
      hovered_(false),
      popup_(nullptr),
      scroll_(nullptr),
      list_(nullptr)
{
    focus_policy = FocusPolicy::TAB_AND_CLICK;
}

void ComboBox::set_items(const std::vector<std::string>& v)
{
    items_ = v;
    layout_ = TextLayout();
    invalidate();
    request_layout();
    notify_changed(items);
    if (current_index_ >= static_cast<int>(items_.size()))
    {
        set_current_index(-1);
    }
}

void ComboBox::set_current_index(const int& v)
{
    const int index = v >= 0 && v < static_cast<int>(items_.size()) ? v : -1;
    if (index == current_index_)
    {
        return;
    }
    current_index_ = index;
    layout_ = TextLayout();
    invalidate();
    // 結びつけた相手へ伝えてから、ハンドラを呼ぶ。
    notify_changed(current_index);
    notify_changed(current_text);
    Object* self = this;
    detail::fire(on_change_, self);
}

std::string ComboBox::get_current_text() const
{
    return current_index_ >= 0 ? items_[static_cast<std::size_t>(current_index_)] : std::string();
}

void ComboBox::set_placeholder_text(const std::string& v)
{
    placeholder_text_ = v;
    layout_ = TextLayout();
    invalidate();
}

void ComboBox::set_font(const Font& v)
{
    font_ = v;
    layout_ = TextLayout();
    invalidate();
    request_layout();
}

const TextLayout& ComboBox::text_layout() const
{
    const float scale = display_scale;
    if (layout_.is_null() || layout_.scale() != scale)
    {
        layout_ = TextLayout(current_index_ >= 0 ? get_current_text() : placeholder_text_, font_, scale);
    }
    return layout_;
}

bool ComboBox::is_popup_open() const { return popup_ && popup_->visible; }

void ComboBox::open_popup()
{
    if (items_.empty() || is_popup_open())
    {
        return;
    }
    if (!popup_)
    {
        // 一覧は ComboBox が持つポップアップに入れ、項目が多ければスクロールする。
        popup_ = new Popup(this);
        BoxLayout* box = new BoxLayout(popup_);
        box->margins = Margins(1);
        scroll_ = new ScrollArea(popup_);
        box->add(scroll_, 1);
        list_ = new detail::PopupList(nullptr);
        scroll_->content = list_;
        list_->on_choose = [this](int i)
        {
            close_popup();
            current_index = i;  // ハンドラの中で破棄されてもよいよう、最後に代入する
        };
        list_->on_cancel = [this]() { close_popup(); };
        list_->on_reveal = [this](const Rect& r) { scroll_->scroll_to_visible(r); };
    }
    list_->set_items(items_, font_);
    const float rows = static_cast<float>(std::min(items_.size(), MAX_ROWS));
    const Size size(std::max(width.get(), list_->preferred_size.get().width), list_->row_height() * rows + 2);
    popup_->open(local_rect, size, PopupPlacement::BELOW);
    popup_->apply_layout();
    list_->highlight(current_index_);
    list_->focused = true;
    invalidate();
}

void ComboBox::close_popup()
{
    if (is_popup_open())
    {
        popup_->close();
        invalidate();
    }
}

Size ComboBox::compute_preferred_size() const
{
    float w = text_layout().width();
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        w = std::max(w, TextLayout(items_[i], font_, display_scale).width());
    }
    return Size(std::ceil(w) + theme::field_padding() * 2 + ARROW_AREA, std::ceil(text_layout().height()) + 16);
}

Size ComboBox::compute_minimum_size() const { return Size(ARROW_AREA * 3, compute_preferred_size().height); }

void ComboBox::paint(Canvas& canvas)
{
    const bool enabled_now = effectively_enabled, has_focus = focused;
    const Rect r = local_rect;
    canvas.fill_round_rect(r, theme::field_radius(), enabled_now ? theme::field() : theme::field_disabled());
    const bool strong = has_focus || is_popup_open();
    canvas.stroke_round_rect(r, theme::field_radius(), strong ? 2.0f : 1.0f,
                             strong || (hovered_ && enabled_now) ? theme::accent() : theme::border());

    const TextLayout& l = text_layout();
    const Rect text_area = r.adjusted(theme::field_padding(), 0, ARROW_AREA, 0);
    canvas.save();
    canvas.clip_rect(text_area);
    const Color fg = !enabled_now ? theme::text_disabled() : current_index_ >= 0 ? theme::text() : theme::placeholder();
    canvas.draw_text(l, Point(text_area.x, (r.height - l.height()) / 2), fg);
    canvas.restore();

    // 下向きの矢印。
    const float cx = r.right() - ARROW_AREA / 2, cy = r.height / 2;
    Path arrow;
    arrow.move_to(cx - 4, cy - 2);
    arrow.line_to(cx, cy + 2);
    arrow.line_to(cx + 4, cy - 2);
    canvas.stroke_path(arrow, 1.5f, enabled_now ? theme::text() : theme::text_disabled(), LineCap::ROUND);
    Widget::paint(canvas);
}

void ComboBox::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    e.accepted = true;
    if (is_popup_open())
    {
        close_popup();
    }
    else
    {
        open_popup();
    }
}

void ComboBox::mouse_enter()
{
    hovered_ = true;
    invalidate();
    Widget::mouse_enter();
}

void ComboBox::mouse_leave()
{
    hovered_ = false;
    invalidate();
    Widget::mouse_leave();
}

void ComboBox::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    const int n = static_cast<int>(items_.size());
    e.accepted = true;
    if ((e.key == Key::DOWN && e.has_modifier(MODIFIER_ALT)) || e.key == Key::SPACE || e.key == Key::RETURN ||
        e.key == Key::F4)
    {
        open_popup();
    }
    else if (e.key == Key::UP && n > 0)
    {
        current_index = std::max(0, current_index_ - 1);
    }
    else if (e.key == Key::DOWN && n > 0)
    {
        current_index = std::min(n - 1, current_index_ + 1);
    }
    else if (e.key == Key::HOME && n > 0)
    {
        current_index = 0;
    }
    else if (e.key == Key::END && n > 0)
    {
        current_index = n - 1;
    }
    else
    {
        e.accepted = false;
    }
}

void ComboBox::focus_in()
{
    invalidate();
    Widget::focus_in();
}

void ComboBox::focus_out()
{
    invalidate();
    Widget::focus_out();
}

void ComboBox::scale_changed() { layout_ = TextLayout(); }

}  // namespace tokotachi
