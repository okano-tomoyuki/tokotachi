#include <gui/spin_box.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <gui/canvas.hpp>
#include <gui/line_edit.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace gui
{

namespace
{
const float BUTTON_WIDTH = 20;  // 上下のボタンの幅(dp)
const int REPEAT_DELAY = 400;   // 押し続けたときに繰り返し始めるまで(ミリ秒)
const int REPEAT_INTERVAL = 60;  // 繰り返しの間隔(ミリ秒)
}  // namespace

namespace detail
{

// SpinBox の中の入力欄。枠は SpinBox が描くので、文字だけを描く。
class SpinEdit : public LineEdit
{
public:
    explicit SpinEdit(SpinBox* spin) : LineEdit(spin), spin_(spin) {}

protected:
    void paint(Canvas& canvas) override
    {
        paint_text(canvas);
        Widget::paint(canvas);
    }

    void focus_in() override
    {
        LineEdit::focus_in();
        spin_->invalidate();  // 枠の色を変える
    }

    void focus_out() override
    {
        LineEdit::focus_out();
        spin_->commit();
        spin_->invalidate();
    }

private:
    SpinBox* spin_;
};

}  // namespace detail

SpinBox::SpinBox(Widget* parent_widget)
    : RangeBase(parent_widget),
      GUI_PROPERTY_GETTER(SpinBox, int, decimals),
      GUI_READONLY_PROPERTY(SpinBox, std::string, text),
      edit_(nullptr),
      decimals_(0),
      pressed_(0)
{
    edit_ = new detail::SpinEdit(this);
    edit_->text = format(value);
    edit_->on_submit = [this](Object*) { commit(); };
    edit_->on_change = [this](Object*) { notify_changed(text); };
    repeat_.on_timer = [this](Object*)
    {
        repeat_.interval = REPEAT_INTERVAL;
        if (pressed_ != 0)
        {
            step_by(pressed_ * single_step.get());
        }
    };
}

void SpinBox::set_decimals(const int& v)
{
    decimals_ = std::max(0, std::min(v, 10));
    edit_->text = format(value);
    request_layout();
}

std::string SpinBox::get_text() const { return edit_->text; }

std::string SpinBox::format(double v) const
{
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", decimals_, v);
    std::string s = buf;
    // 丸めて 0 になった負の値は「-0」にしない。
    if (s[0] == '-' && s.find_first_not_of("-0.") == std::string::npos)
    {
        s.erase(0, 1);
    }
    return s;
}

void SpinBox::commit()
{
    const std::string s = edit_->text;
    const char* begin = s.c_str();
    char* end = nullptr;
    const double parsed = std::strtod(begin, &end);
    while (end && (*end == ' ' || *end == '\t'))
    {
        ++end;
    }
    if (end != begin && end && *end == '\0' && std::isfinite(parsed))
    {
        const double scale = std::pow(10.0, decimals_);
        value = std::floor(parsed * scale + 0.5) / scale;
    }
    // 読めなかったとき・範囲に丸めたときも、今の値の表示に戻す。
    edit_->text = format(value);
}

void SpinBox::range_changed()
{
    RangeBase::range_changed();
    if (edit_)
    {
        edit_->text = format(value);
    }
}

Rect SpinBox::up_button() const
{
    const Rect r = local_rect;
    return Rect(r.width - BUTTON_WIDTH, 0, BUTTON_WIDTH, r.height / 2);
}

Rect SpinBox::down_button() const
{
    const Rect r = local_rect;
    return Rect(r.width - BUTTON_WIDTH, r.height / 2, BUTTON_WIDTH, r.height - r.height / 2);
}

void SpinBox::arrange_children()
{
    const Rect r = local_rect;
    edit_->bounds = Rect(0, 0, std::max(0.0f, r.width - BUTTON_WIDTH), r.height);
}

Size SpinBox::compute_preferred_size() const
{
    // 範囲の両端の数値が収まる幅にする。
    const Font f = edit_->font;
    const float scale = display_scale;
    const float w = std::max(TextLayout(format(minimum), f, scale).width(), TextLayout(format(maximum), f, scale).width());
    const Size edit = edit_->preferred_size;
    return Size(std::max(72.0f, std::ceil(w) + theme::field_padding() * 2 + 4) + BUTTON_WIDTH, edit.height);
}

Size SpinBox::compute_minimum_size() const { return Size(48 + BUTTON_WIDTH, compute_preferred_size().height); }

void SpinBox::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool enabled_now = effectively_enabled;
    const bool has_focus = edit_->focused;
    canvas.fill_round_rect(r, theme::field_radius(), enabled_now ? theme::field() : theme::field_disabled());

    // 上下のボタン: 押しているほうを塗り、区切りの線と山形の印を描く。
    const Rect up = up_button();
    const Rect down = down_button();
    if (pressed_ != 0)
    {
        canvas.fill_rect((pressed_ > 0 ? up : down).adjusted(0, 1, 1, 1), theme::selection_inactive());
    }
    canvas.fill_rect(Rect(up.x, 1, 1, r.height - 2), theme::border());
    canvas.fill_rect(Rect(up.x, down.y, up.width - 1, 1), theme::border());
    const Color mark = enabled_now ? theme::text() : theme::text_disabled();
    const float cx = up.x + up.width / 2;
    const float half = 4;
    Path up_mark;
    up_mark.move_to(cx - half, up.y + up.height / 2 + half / 2);
    up_mark.line_to(cx, up.y + up.height / 2 - half / 2);
    up_mark.line_to(cx + half, up.y + up.height / 2 + half / 2);
    canvas.stroke_path(up_mark, 1.5f, mark, LineCap::ROUND);
    Path down_mark;
    down_mark.move_to(cx - half, down.y + down.height / 2 - half / 2);
    down_mark.line_to(cx, down.y + down.height / 2 + half / 2);
    down_mark.line_to(cx + half, down.y + down.height / 2 - half / 2);
    canvas.stroke_path(down_mark, 1.5f, mark, LineCap::ROUND);

    canvas.stroke_round_rect(r, theme::field_radius(), has_focus ? 2.0f : 1.0f,
                             has_focus ? theme::accent() : theme::border());
    Widget::paint(canvas);
}

void SpinBox::press(int direction)
{
    commit();
    pressed_ = direction;
    step_by(direction * single_step.get());
    repeat_.interval = REPEAT_DELAY;
    repeat_.enabled = true;
    invalidate();
}

void SpinBox::stop_repeat()
{
    repeat_.enabled = false;
    if (pressed_ != 0)
    {
        pressed_ = 0;
        invalidate();
    }
}

void SpinBox::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    e.accepted = true;
    edit_->focused = true;  // 続けてキーで操作できるように
    if (up_button().contains(e.pos))
    {
        press(1);
    }
    else if (down_button().contains(e.pos))
    {
        press(-1);
    }
}

void SpinBox::mouse_up(MouseEvent& e)
{
    Widget::mouse_up(e);
    if (e.button == MouseButton::LEFT)
    {
        e.accepted = true;
        stop_repeat();
    }
}

void SpinBox::mouse_wheel(WheelEvent& e)
{
    Widget::mouse_wheel(e);
    if (!edit_->focused)
    {
        return;  // 親(スクロール領域など)へ送る
    }
    e.accepted = true;
    commit();
    const float ticks = e.delta.y != 0 ? e.delta.y : e.delta.x;
    step_by(ticks * single_step.get());
}

void SpinBox::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    double amount = 0;
    switch (e.key)
    {
    case Key::UP:
        amount = single_step;
        break;
    case Key::DOWN:
        amount = -single_step.get();
        break;
    case Key::PAGE_UP:
        amount = page_step;
        break;
    case Key::PAGE_DOWN:
        amount = -page_step.get();
        break;
    default:
        return;
    }
    e.accepted = true;
    commit();
    step_by(amount);
}

}  // namespace gui
