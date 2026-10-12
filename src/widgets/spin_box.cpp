#include <tokotachi/widgets/spin_box.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/widgets/line_edit.hpp>

#include "text_input_impl.hpp"
#include "../core/impl.hpp"
#include "theme.hpp"

namespace tokotachi
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
        detail::ImplAccess<TextInputBase>::of(this)->paint_text(canvas);
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

class SpinBox::Impl
{
    TOKOTACHI_IMPL(SpinBox);

    explicit Impl(SpinBox* object) : q(object) {}

    int get_decimals() const { return decimals_; }
    void set_decimals(const int& v);
    std::string get_prefix() const { return prefix_; }
    void set_prefix(const std::string& v);
    std::string get_suffix() const { return suffix_; }
    void set_suffix(const std::string& v);
    std::string get_text() const;

    std::string format(double v) const;
    /// 上下のボタンの領域(自分の座標系)。
    Rect up_button() const;
    Rect down_button() const;
    /// ボタンを押し始めた・押し続けている(direction は 1 か -1)。
    void press(int direction);
    void stop_repeat();

    detail::SpinEdit* edit_;
    int decimals_;
    std::string prefix_;
    std::string suffix_;
    int pressed_;  ///< 押しているボタン(1 は上、-1 は下、0 は無し)
    Timer repeat_;
};

SpinBox::SpinBox(Widget* parent_widget)
    : RangeBase(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, int, decimals),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, std::string, prefix),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, std::string, suffix),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::string, text),
      d_(new Impl(this))
{
    d_->edit_ = nullptr;
    d_->decimals_ = 0;
    d_->pressed_ = 0;
    d_->edit_ = new detail::SpinEdit(this);
    d_->edit_->text = d_->format(value);
    d_->edit_->on_submit = [this](Object*) { commit(); };
    d_->edit_->on_change = [this](Object*) { notify_changed(text); };
    d_->repeat_.on_timer = [this](Object*)
    {
        d_->repeat_.interval = REPEAT_INTERVAL;
        if (d_->pressed_ != 0)
        {
            step_by(d_->pressed_ * single_step.get());
        }
    };
}

SpinBox::~SpinBox() { delete d_; }

void SpinBox::Impl::set_decimals(const int& v)
{
    decimals_ = std::max(0, std::min(v, 10));
    edit_->text = format(q->value);
    q->request_layout();
}

namespace
{

// 前後の空白を除く。
std::string trim(const std::string& s)
{
    const char* space = " \t";
    const std::size_t first = s.find_first_not_of(space);
    return first == std::string::npos ? std::string() : s.substr(first, s.find_last_not_of(space) - first + 1);
}

}  // namespace

void SpinBox::Impl::set_prefix(const std::string& v)
{
    prefix_ = v;
    edit_->text = format(q->value);
    q->request_layout();
}

void SpinBox::Impl::set_suffix(const std::string& v)
{
    suffix_ = v;
    edit_->text = format(q->value);
    q->request_layout();
}

std::string SpinBox::Impl::get_text() const { return edit_->text; }

std::string SpinBox::Impl::format(double v) const
{
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", decimals_, v);
    std::string s = buf;
    // 丸めて 0 になった負の値は「-0」にしない。
    if (s[0] == '-' && s.find_first_not_of("-0.") == std::string::npos)
    {
        s.erase(0, 1);
    }
    return prefix_ + s + suffix_;
}

void SpinBox::commit()
{
    // 前・後ろの文字(付いていれば。周りの空白は問わない)を除いて読む
    std::string s = trim(d_->edit_->text);
    const std::string head = trim(d_->prefix_), tail = trim(d_->suffix_);
    if (!head.empty() && s.compare(0, head.size(), head) == 0)
    {
        s = trim(s.substr(head.size()));
    }
    if (!tail.empty() && s.size() >= tail.size() && s.compare(s.size() - tail.size(), tail.size(), tail) == 0)
    {
        s = trim(s.substr(0, s.size() - tail.size()));
    }
    const char* begin = s.c_str();
    char* end = nullptr;
    const double parsed = std::strtod(begin, &end);
    while (end && (*end == ' ' || *end == '\t'))
    {
        ++end;
    }
    if (end != begin && end && *end == '\0' && std::isfinite(parsed))
    {
        const double scale = std::pow(10.0, d_->decimals_);
        value = std::floor(parsed * scale + 0.5) / scale;
    }
    // 読めなかったとき・範囲に丸めたときも、今の値の表示に戻す。
    d_->edit_->text = d_->format(value);
}

void SpinBox::range_changed()
{
    RangeBase::range_changed();
    if (d_->edit_)
    {
        d_->edit_->text = d_->format(value);
    }
}

Rect SpinBox::Impl::up_button() const
{
    const Rect r = q->local_rect;
    return Rect(r.width - BUTTON_WIDTH, 0, BUTTON_WIDTH, r.height / 2);
}

Rect SpinBox::Impl::down_button() const
{
    const Rect r = q->local_rect;
    return Rect(r.width - BUTTON_WIDTH, r.height / 2, BUTTON_WIDTH, r.height - r.height / 2);
}

void SpinBox::arrange_children()
{
    const Rect r = local_rect;
    d_->edit_->bounds = Rect(0, 0, std::max(0.0f, r.width - BUTTON_WIDTH), r.height);
}

Size SpinBox::compute_preferred_size() const
{
    // 範囲の両端の数値が収まる幅にする。
    const Font f = d_->edit_->font;
    const float scale = display_scale;
    const float w = std::max(TextLayout(d_->format(minimum), f, scale).width(), TextLayout(d_->format(maximum), f, scale).width());
    const Size edit = d_->edit_->preferred_size;
    return Size(std::max(72.0f, std::ceil(w) + theme::field_padding() * 2 + 4) + BUTTON_WIDTH, edit.height);
}

Size SpinBox::compute_minimum_size() const { return Size(48 + BUTTON_WIDTH, compute_preferred_size().height); }

void SpinBox::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool enabled_now = effectively_enabled;
    const bool has_focus = d_->edit_->focused;
    canvas.fill_round_rect(r, theme::field_radius(), enabled_now ? theme::field() : theme::field_disabled());

    // 上下のボタン: 押しているほうを塗り、区切りの線と山形の印を描く。
    const Rect up = d_->up_button();
    const Rect down = d_->down_button();
    if (d_->pressed_ != 0)
    {
        canvas.fill_rect((d_->pressed_ > 0 ? up : down).adjusted(0, 1, 1, 1), theme::selection_inactive());
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

void SpinBox::Impl::press(int direction)
{
    q->commit();
    pressed_ = direction;
    q->step_by(direction * q->single_step.get());
    repeat_.interval = REPEAT_DELAY;
    repeat_.enabled = true;
    q->invalidate();
}

void SpinBox::Impl::stop_repeat()
{
    repeat_.enabled = false;
    if (pressed_ != 0)
    {
        pressed_ = 0;
        q->invalidate();
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
    d_->edit_->focused = true;  // 続けてキーで操作できるように
    if (d_->up_button().contains(e.pos))
    {
        d_->press(1);
    }
    else if (d_->down_button().contains(e.pos))
    {
        d_->press(-1);
    }
}

void SpinBox::mouse_up(MouseEvent& e)
{
    Widget::mouse_up(e);
    if (e.button == MouseButton::LEFT)
    {
        e.accepted = true;
        d_->stop_repeat();
    }
}

void SpinBox::mouse_wheel(WheelEvent& e)
{
    Widget::mouse_wheel(e);
    if (!d_->edit_->focused)
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

}  // namespace tokotachi
