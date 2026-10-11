#include <tokotachi/widgets/calendar.hpp>

#include <algorithm>
#include <cmath>
#include <string>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/graphics/text_layout.hpp>

#include "../core/locale.hpp"
#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

namespace
{
const int CELLS = 42;          // 6 週 × 7 日
const float PADDING = 6;       // 周りの余白
const float BUTTON = 24;       // 前・次の月のボタンの大きさ

// 曜日の短い名前(言語に合わせる)。
std::string weekday_name(int d)
{
    static const char* const JA[] = {"日", "月", "火", "水", "木", "金", "土"};
    static const char* const EN[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
    return detail::prefers_japanese() ? JA[d] : EN[d];
}

// 「2026年10月」・「October 2026」。
std::string month_title(int year, int month)
{
    static const char* const EN[] = {"January", "February", "March",     "April",   "May",      "June",
                                     "July",    "August",   "September", "October", "November", "December"};
    if (detail::prefers_japanese())
    {
        return std::to_string(year) + "年" + std::to_string(month) + "月";
    }
    return std::string(EN[month - 1]) + " " + std::to_string(year);
}

}  // namespace

Calendar::Calendar(Widget* parent_widget)
    : Widget(parent_widget),
      TOKOTACHI_PROPERTY_GETTER_SELF_NOTIFY(Calendar, Date, date),
      TOKOTACHI_PROPERTY_GETTER(Calendar, Date, minimum),
      TOKOTACHI_PROPERTY_GETTER(Calendar, Date, maximum),
      TOKOTACHI_PROPERTY_GETTER(Calendar, DayOfWeek, first_day_of_week),
      TOKOTACHI_READONLY_PROPERTY(Calendar, int, shown_year),
      TOKOTACHI_READONLY_PROPERTY(Calendar, int, shown_month),
      TOKOTACHI_HANDLER_PROPERTY(Calendar, NotifyHandler, on_change),
      TOKOTACHI_HANDLER_PROPERTY(Calendar, NotifyHandler, on_activate),
      minimum_(1, 1, 1),
      maximum_(9999, 12, 31),
      first_day_(DayOfWeek::SUNDAY),
      hover_cell_(-1)
{
    focus_policy = FocusPolicy::TAB_AND_CLICK;
    const Date today = Date::today();
    shown_year_ = today.year;
    shown_month_ = today.month;
}

void Calendar::set_date(const Date& v)
{
    Date d = v;
    if (!d.is_null() && !d.is_valid())
    {
        return;
    }
    if (!d.is_null())
    {
        d = std::max(minimum_, std::min(maximum_, d));
        show_month(d.year, d.month);
    }
    if (d == date_)
    {
        return;
    }
    date_ = d;
    invalidate();
    notify_changed(date);
    Object* self = this;
    detail::fire(on_change_, self);
}

void Calendar::set_minimum(const Date& v)
{
    if (!v.is_valid())
    {
        return;
    }
    minimum_ = v;
    if (maximum_ < minimum_)
    {
        maximum_ = minimum_;
        notify_changed(maximum);
    }
    if (!date_.is_null() && date_ < minimum_)
    {
        set_date(minimum_);
    }
    invalidate();
}

void Calendar::set_maximum(const Date& v)
{
    if (!v.is_valid())
    {
        return;
    }
    maximum_ = v;
    if (minimum_ > maximum_)
    {
        minimum_ = maximum_;
        notify_changed(minimum);
    }
    if (!date_.is_null() && date_ > maximum_)
    {
        set_date(maximum_);
    }
    invalidate();
}

void Calendar::set_first_day_of_week(const DayOfWeek& v)
{
    first_day_ = v;
    invalidate();
}

void Calendar::show_month(int year, int month)
{
    // 範囲の外の月は、範囲の端の月にする
    const Date first = std::max(Date(minimum_.year, minimum_.month, 1),
                                std::min(Date(maximum_.year, maximum_.month, 1), Date(year, std::max(1, std::min(12, month)), 1)));
    if (first.year == shown_year_ && first.month == shown_month_)
    {
        return;
    }
    const bool year_changed = first.year != shown_year_;
    shown_year_ = first.year;
    shown_month_ = first.month;
    hover_cell_ = -1;
    invalidate();
    if (year_changed)
    {
        notify_changed(shown_year);
    }
    notify_changed(shown_month);
}

// ---- 寸法 ----

float Calendar::cell_width() const
{
    const float w = TextLayout("00", Font(), display_scale).width();
    return std::max(32.0f, std::ceil(w) + 14);
}

float Calendar::cell_height() const { return std::ceil(TextLayout("Ag", Font(), display_scale).height()) + 10; }

float Calendar::title_height() const { return std::max(BUTTON, cell_height()) + 4; }

Size Calendar::compute_preferred_size() const
{
    return Size(cell_width() * 7 + PADDING * 2, title_height() + cell_height() * 7 + PADDING * 2);
}

Size Calendar::compute_minimum_size() const { return compute_preferred_size(); }

Date Calendar::first_cell() const
{
    const Date first(shown_year_, shown_month_, 1);
    const int offset = (static_cast<int>(first.day_of_week()) - static_cast<int>(first_day_) + 7) % 7;
    return first.add_days(-offset);
}

Rect Calendar::cell_rect(int cell) const
{
    const float w = cell_width(), h = cell_height();
    const float x0 = (width.get() - w * 7) / 2;
    const float y0 = PADDING + title_height() + h;  // 曜日の行の下
    return Rect(x0 + (cell % 7) * w, y0 + (cell / 7) * h, w, h);
}

int Calendar::cell_at(const Point& p) const
{
    for (int c = 0; c < CELLS; ++c)
    {
        if (cell_rect(c).contains(p))
        {
            return c;
        }
    }
    return -1;
}

Rect Calendar::previous_button() const
{
    const float x0 = (width.get() - cell_width() * 7) / 2;
    return Rect(x0, PADDING + (title_height() - BUTTON) / 2, BUTTON, BUTTON);
}

Rect Calendar::next_button() const
{
    const float x1 = (width.get() + cell_width() * 7) / 2;
    return Rect(x1 - BUTTON, PADDING + (title_height() - BUTTON) / 2, BUTTON, BUTTON);
}

bool Calendar::selectable(const Date& d) const { return d.is_valid() && d >= minimum_ && d <= maximum_; }

// ---- 描画 ----

void Calendar::paint(Canvas& canvas)
{
    const bool enabled_now = effectively_enabled;
    const float scale = display_scale;
    // 見出し: 前・次の月のボタンと「2026年10月」
    const TextLayout title(month_title(shown_year_, shown_month_), Font(16, true), scale);
    canvas.draw_text(title, Point((width.get() - title.width()) / 2, PADDING + (title_height() - title.height()) / 2),
                     enabled_now ? theme::text() : theme::text_disabled());
    const Rect buttons[] = {previous_button(), next_button()};
    for (int k = 0; k < 2; ++k)
    {
        const Rect b = buttons[k];
        if (enabled_now && hover_cell_ == -2 - k)
        {
            canvas.fill_round_rect(b, theme::radius(), theme::selection_inactive());
        }
        const float cx = b.x + b.width / 2, cy = b.y + b.height / 2, d = k == 0 ? -1.0f : 1.0f;
        Path mark;
        mark.move_to(cx - d * 2, cy - 4);
        mark.line_to(cx + d * 2, cy);
        mark.line_to(cx - d * 2, cy + 4);
        canvas.stroke_path(mark, 1.5f, enabled_now ? theme::text() : theme::text_disabled(), LineCap::ROUND);
    }
    // 曜日の行
    const float w = cell_width(), h = cell_height();
    const Rect first_rect = cell_rect(0);
    for (int i = 0; i < 7; ++i)
    {
        const int day = (static_cast<int>(first_day_) + i) % 7;
        const TextLayout l(weekday_name(day), Font(), scale);
        canvas.draw_text(l, Point(first_rect.x + i * w + (w - l.width()) / 2, first_rect.y - h + (h - l.height()) / 2),
                         enabled_now ? theme::color(ColorRole::TEXT_SECONDARY) : theme::text_disabled());
    }
    // 日のマス
    const Date start = first_cell();
    const Date today = Date::today();
    for (int c = 0; c < CELLS; ++c)
    {
        const Date d = start.add_days(c);
        const Rect r = cell_rect(c);
        const Rect mark = r.adjusted(2, 2, 2, 2);
        const bool in_month = d.month == shown_month_;
        const bool can = enabled_now && selectable(d);
        const bool chosen = d == date_;
        if (chosen)
        {
            canvas.fill_round_rect(mark, theme::radius(), can ? theme::accent() : theme::accent_disabled());
        }
        else if (can && c == hover_cell_)
        {
            canvas.fill_round_rect(mark, theme::radius(), theme::selection_inactive());
        }
        if (d == today && !chosen)
        {
            canvas.stroke_round_rect(mark.adjusted(0.5f, 0.5f, 0.5f, 0.5f), theme::radius(), 1, theme::accent());
        }
        if (chosen && focused)
        {
            canvas.stroke_round_rect(mark.adjusted(-2, -2, -2, -2), theme::radius() + 2, 1.5f, theme::focus_ring());
        }
        const Color fg = chosen ? theme::on_accent() : !can ? theme::text_disabled() : in_month ? theme::text() : theme::placeholder();
        const TextLayout l(std::to_string(d.day), Font(), scale);
        canvas.draw_text(l, Point(r.x + (r.width - l.width()) / 2, r.y + (r.height - l.height()) / 2), fg);
    }
    Widget::paint(canvas);
}

// ---- 入力 ----

void Calendar::choose(const Date& d)
{
    if (!d.is_valid())
    {
        return;
    }
    const Date c = std::max(minimum_, std::min(maximum_, d));
    show_month(c.year, c.month);
    set_date(c);
}

void Calendar::activate()
{
    if (date_.is_null())
    {
        return;
    }
    Object* self = this;
    detail::fire(on_activate_, self);
}

void Calendar::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    e.accepted = true;
    if (previous_button().contains(e.pos))
    {
        show_month(Date(shown_year_, shown_month_, 1).add_months(-1).year, Date(shown_year_, shown_month_, 1).add_months(-1).month);
        return;
    }
    if (next_button().contains(e.pos))
    {
        const Date next = Date(shown_year_, shown_month_, 1).add_months(1);
        show_month(next.year, next.month);
        return;
    }
    const int c = cell_at(e.pos);
    if (c < 0)
    {
        return;
    }
    const Date d = first_cell().add_days(c);
    if (!selectable(d))
    {
        return;
    }
    choose(d);
    activate();  // ハンドラの中でポップアップを閉じてもよいよう、最後に呼ぶ
}

void Calendar::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    const int hover = previous_button().contains(e.pos) ? -2 : next_button().contains(e.pos) ? -3 : cell_at(e.pos);
    if (hover != hover_cell_)
    {
        hover_cell_ = hover;
        invalidate();
    }
}

void Calendar::mouse_leave()
{
    if (hover_cell_ != -1)
    {
        hover_cell_ = -1;
        invalidate();
    }
    Widget::mouse_leave();
}

void Calendar::mouse_wheel(WheelEvent& e)
{
    Widget::mouse_wheel(e);
    const float ticks = e.delta.y != 0 ? e.delta.y : e.delta.x;
    if (ticks == 0)
    {
        return;
    }
    e.accepted = true;
    const Date next = Date(shown_year_, shown_month_, 1).add_months(ticks > 0 ? -1 : 1);
    show_month(next.year, next.month);
}

void Calendar::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    // 選んでいなければ、見せている月の 1 日(今日の月なら今日)から動かす
    const Date today = Date::today();
    const Date base = !date_.is_null() ? date_
                      : today.year == shown_year_ && today.month == shown_month_ ? today
                                                                                 : Date(shown_year_, shown_month_, 1);
    const bool shift = e.has_modifier(MODIFIER_SHIFT);
    Date next;
    switch (e.key)
    {
    case Key::LEFT:
        next = base.add_days(-1);
        break;
    case Key::RIGHT:
        next = base.add_days(1);
        break;
    case Key::UP:
        next = base.add_days(-7);
        break;
    case Key::DOWN:
        next = base.add_days(7);
        break;
    case Key::PAGE_UP:
        next = shift ? base.add_years(-1) : base.add_months(-1);
        break;
    case Key::PAGE_DOWN:
        next = shift ? base.add_years(1) : base.add_months(1);
        break;
    case Key::HOME:
        next = Date(base.year, base.month, 1);
        break;
    case Key::END:
        next = Date(base.year, base.month, Date::days_in_month(base.year, base.month));
        break;
    case Key::RETURN:
    case Key::SPACE:
        e.accepted = true;
        if (date_.is_null())
        {
            choose(base);
        }
        activate();
        return;
    default:
        return;
    }
    e.accepted = true;
    choose(date_.is_null() && e.key != Key::PAGE_UP && e.key != Key::PAGE_DOWN ? base : next);
}

void Calendar::focus_in()
{
    invalidate();
    Widget::focus_in();
}

void Calendar::focus_out()
{
    invalidate();
    Widget::focus_out();
}

}  // namespace tokotachi
