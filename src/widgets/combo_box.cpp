#include <tokotachi/widgets/combo_box.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/layout/box_layout.hpp>
#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/widgets/scroll_area.hpp>

#include "../core/impl.hpp"
#include "popup_list.hpp"
#include "theme.hpp"

namespace tokotachi
{

namespace
{
const float ARROW_AREA = 24;     // 右の下向きの矢印の幅(dp)
const std::size_t MAX_ROWS = 10;  // 一覧に一度に見せる項目の数
}

class ComboBox::Impl
{
    TOKOTACHI_IMPL(ComboBox);

    explicit Impl(ComboBox* object) : q(object) {}

    bool get_popup_open() const { return popup_ && popup_->visible; }

    std::vector<std::string> get_items() const { return items_; }
    void set_items(const std::vector<std::string>& v);
    int get_current_index() const { return current_index_; }
    void set_current_index(const int& v);
    std::string get_current_text() const;
    void set_placeholder_text(const std::string& v);
    void set_font(const Font& v);
    const TextLayout& text_layout() const;

    std::vector<std::string> items_;
    int current_index_;
    std::string placeholder_text_;
    Font font_;
    NotifyHandler on_change_;
    bool hovered_;
    Popup* popup_;
    ScrollArea* scroll_;
    detail::PopupList* list_;
    mutable TextLayout layout_;
};

ComboBox::ComboBox(Widget* parent_widget)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, std::vector<std::string>, items),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, int, current_index),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::string, current_text),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, placeholder_text),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, Font, font),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_change),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, bool, popup_open),
      d_(new Impl(this))
{
    d_->current_index_ = -1;
    d_->hovered_ = false;
    d_->popup_ = nullptr;
    d_->scroll_ = nullptr;
    d_->list_ = nullptr;
    focus_policy = FocusPolicy::TAB_AND_CLICK;
}

ComboBox::~ComboBox() { delete d_; }

void ComboBox::Impl::set_items(const std::vector<std::string>& v)
{
    items_ = v;
    layout_ = TextLayout();
    q->invalidate();
    q->request_layout();
    q->notify_changed(q->items);
    if (current_index_ >= static_cast<int>(items_.size()))
    {
        set_current_index(-1);
    }
}

void ComboBox::Impl::set_current_index(const int& v)
{
    const int index = v >= 0 && v < static_cast<int>(items_.size()) ? v : -1;
    if (index == current_index_)
    {
        return;
    }
    current_index_ = index;
    layout_ = TextLayout();
    q->invalidate();
    // 結びつけた相手へ伝えてから、ハンドラを呼ぶ。
    q->notify_changed(q->current_index);
    q->notify_changed(q->current_text);
    Object* self = q;
    detail::fire(on_change_, self);
}

std::string ComboBox::Impl::get_current_text() const
{
    return current_index_ >= 0 ? items_[static_cast<std::size_t>(current_index_)] : std::string();
}

void ComboBox::Impl::set_placeholder_text(const std::string& v)
{
    placeholder_text_ = v;
    layout_ = TextLayout();
    q->invalidate();
}

void ComboBox::Impl::set_font(const Font& v)
{
    font_ = v;
    layout_ = TextLayout();
    q->invalidate();
    q->request_layout();
}

const TextLayout& ComboBox::Impl::text_layout() const
{
    const float scale = q->display_scale;
    if (layout_.is_null() || layout_.scale() != scale)
    {
        layout_ = TextLayout(current_index_ >= 0 ? get_current_text() : placeholder_text_, font_, scale);
    }
    return layout_;
}

void ComboBox::open_popup()
{
    if (d_->items_.empty() || popup_open.get())
    {
        return;
    }
    if (!d_->popup_)
    {
        // 一覧は ComboBox が持つポップアップに入れ、項目が多ければスクロールする。
        d_->popup_ = new Popup(this);
        BoxLayout* box = new BoxLayout(d_->popup_);
        box->margins = Margins(1);
        d_->scroll_ = new ScrollArea(d_->popup_);
        box->add(d_->scroll_, 1);
        d_->list_ = new detail::PopupList(nullptr);
        d_->scroll_->content = d_->list_;
        d_->list_->on_choose = [this](int i)
        {
            close_popup();
            current_index = i;  // ハンドラの中で破棄されてもよいよう、最後に代入する
        };
        d_->list_->on_cancel = [this]() { close_popup(); };
        d_->list_->on_reveal = [this](const Rect& r) { d_->scroll_->scroll_to_visible(r); };
        // 外側を押して閉じたときも含め、閉じたら知らせる
        d_->popup_->on_closed = [this](Object*)
        {
            invalidate();
            notify_changed(popup_open);
        };
    }
    d_->list_->set_items(d_->items_, d_->font_);
    const float rows = static_cast<float>(std::min(d_->items_.size(), MAX_ROWS));
    const Size size(std::max(width.get(), d_->list_->preferred_size.get().width), d_->list_->row_height() * rows + 2);
    d_->popup_->open(local_rect, size, PopupPlacement::BELOW);
    d_->popup_->apply_layout();
    d_->list_->highlight(d_->current_index_);
    d_->list_->focused = true;
    invalidate();
    notify_changed(popup_open);
}

void ComboBox::close_popup()
{
    if (popup_open.get())
    {
        d_->popup_->close();
        invalidate();
    }
}

Size ComboBox::compute_preferred_size() const
{
    float w = d_->text_layout().width();
    for (std::size_t i = 0; i < d_->items_.size(); ++i)
    {
        w = std::max(w, TextLayout(d_->items_[i], d_->font_, display_scale).width());
    }
    return Size(std::ceil(w) + theme::field_padding() * 2 + ARROW_AREA, std::ceil(d_->text_layout().height()) + 16);
}

Size ComboBox::compute_minimum_size() const { return Size(ARROW_AREA * 3, compute_preferred_size().height); }

void ComboBox::paint(Canvas& canvas)
{
    const bool enabled_now = effectively_enabled, has_focus = focused;
    const Rect r = local_rect;
    canvas.fill_round_rect(r, theme::field_radius(), enabled_now ? theme::field() : theme::field_disabled());
    const bool strong = has_focus || popup_open.get();
    canvas.stroke_round_rect(r, theme::field_radius(), strong ? 2.0f : 1.0f,
                             strong || (d_->hovered_ && enabled_now) ? theme::accent() : theme::border());

    const TextLayout& l = d_->text_layout();
    const Rect text_area = r.adjusted(theme::field_padding(), 0, ARROW_AREA, 0);
    canvas.save();
    canvas.clip_rect(text_area);
    const Color fg = !enabled_now ? theme::text_disabled() : d_->current_index_ >= 0 ? theme::text() : theme::placeholder();
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
    if (popup_open.get())
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
    d_->hovered_ = true;
    invalidate();
    Widget::mouse_enter();
}

void ComboBox::mouse_leave()
{
    d_->hovered_ = false;
    invalidate();
    Widget::mouse_leave();
}

void ComboBox::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    const int n = static_cast<int>(d_->items_.size());
    e.accepted = true;
    if ((e.key == Key::DOWN && e.has_modifier(MODIFIER_ALT)) || e.key == Key::SPACE || e.key == Key::RETURN ||
        e.key == Key::F4)
    {
        open_popup();
    }
    else if (e.key == Key::UP && n > 0)
    {
        current_index = std::max(0, d_->current_index_ - 1);
    }
    else if (e.key == Key::DOWN && n > 0)
    {
        current_index = std::min(n - 1, d_->current_index_ + 1);
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

void ComboBox::scale_changed() { d_->layout_ = TextLayout(); }

}  // namespace tokotachi
