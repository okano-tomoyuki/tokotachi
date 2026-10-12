#include <tokotachi/widgets/calendar.hpp>

#include <algorithm>
#include <cmath>
#include <string>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/graphics/text_layout.hpp>

#include "../core/locale.hpp"
#include "../core/impl.hpp"
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

class Calendar::Impl
{
    TOKOTACHI_IMPL(Calendar);

    explicit Impl(Calendar* object) : q(object) {}

    Date get_date() const { return date_; }
    void set_date(const Date& v);
    Date get_minimum() const { return minimum_; }
    void set_minimum(const Date& v);
    Date get_maximum() const { return maximum_; }
    void set_maximum(const Date& v);
    DayOfWeek get_first_day_of_week() const { return first_day_; }
    void set_first_day_of_week(const DayOfWeek& v);
    int get_shown_year() const { return shown_year_; }
    int get_shown_month() const { return shown_month_; }

    /// 寸法(dp)。
    float cell_width() const;
    float cell_height() const;
    float title_height() const;
    /// 見せている月の 1 日を含む週の、最初のマスの日。
    Date first_cell() const;
    /// マス(0〜41)の矩形・位置 p のマス(無ければ -1)。
    Rect cell_rect(int cell) const;
    int cell_at(const Point& p) const;
    Rect previous_button() const;
    Rect next_button() const;
    bool selectable(const Date& d) const;
    /// 利用者の操作で日を選ぶ(範囲に収め、その月を見せる)。
    void choose(const Date& d);
    void activate();

    Date date_;
    Date minimum_;
    Date maximum_;
    DayOfWeek first_day_;
    int shown_year_;
    int shown_month_;
    int hover_cell_;  ///< ポインタの下のマス(-1 なら無し。-2・-3 は前・次の月のボタン)
    NotifyHandler on_change_;
    NotifyHandler on_activate_;
};

Calendar::Calendar(Widget* parent_widget)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, Date, date),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, Date, minimum),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, Date, maximum),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, DayOfWeek, first_day_of_week),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, int, shown_year),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, int, shown_month),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_change),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_activate),
      d_(new Impl(this))
{
    d_->minimum_ = Date(1, 1, 1);
    d_->maximum_ = Date(9999, 12, 31);
    d_->first_day_ = DayOfWeek::SUNDAY;
    d_->hover_cell_ = -1;
    focus_policy = FocusPolicy::TAB_AND_CLICK;
    const Date today = Date::today();
    d_->shown_year_ = today.year;
    d_->shown_month_ = today.month;
}

Calendar::~Calendar() { delete d_; }

void Calendar::Impl::set_date(const Date& v)
{
    Date d = v;
    if (!d.is_null() && !d.is_valid())
    {
        return;
    }
    if (!d.is_null())
    {
        d = std::max(minimum_, std::min(maximum_, d));
        q->show_month(d.year, d.month);
    }
    if (d == date_)
    {
        return;
    }
    date_ = d;
    q->invalidate();
    q->notify_changed(q->date);
    Object* self = q;
    detail::fire(on_change_, self);
}

void Calendar::Impl::set_minimum(const Date& v)
{
    if (!v.is_valid())
    {
        return;
    }
    minimum_ = v;
    if (maximum_ < minimum_)
    {
        maximum_ = minimum_;
        q->notify_changed(q->maximum);
    }
    if (!date_.is_null() && date_ < minimum_)
    {
        set_date(minimum_);
    }
    q->invalidate();
}

void Calendar::Impl::set_maximum(const Date& v)
{
    if (!v.is_valid())
    {
        return;
    }
    maximum_ = v;
    if (minimum_ > maximum_)
    {
        minimum_ = maximum_;
        q->notify_changed(q->minimum);
    }
    if (!date_.is_null() && date_ > maximum_)
    {
        set_date(maximum_);
    }
    q->invalidate();
}

void Calendar::Impl::set_first_day_of_week(const DayOfWeek& v)
{
    first_day_ = v;
    q->invalidate();
}

void Calendar::show_month(int year, int month)
{
    // 範囲の外の月は、範囲の端の月にする
    const Date first = std::max(Date(d_->minimum_.year, d_->minimum_.month, 1),
                                std::min(Date(d_->maximum_.year, d_->maximum_.month, 1), Date(year, std::max(1, std::min(12, month)), 1)));
    if (first.year == d_->shown_year_ && first.month == d_->shown_month_)
    {
        return;
    }
    const bool year_changed = first.year != d_->shown_year_;
    d_->shown_year_ = first.year;
    d_->shown_month_ = first.month;
    d_->hover_cell_ = -1;
    invalidate();
    if (year_changed)
    {
        notify_changed(shown_year);
    }
    notify_changed(shown_month);
}

// ---- 寸法 ----

float Calendar::Impl::cell_width() const
{
    const float w = TextLayout("00", Font(), q->display_scale).width();
    return std::max(32.0f, std::ceil(w) + 14);
}

float Calendar::Impl::cell_height() const { return std::ceil(TextLayout("Ag", Font(), q->display_scale).height()) + 10; }

float Calendar::Impl::title_height() const { return std::max(BUTTON, cell_height()) + 4; }

Size Calendar::compute_preferred_size() const
{
    return Size(d_->cell_width() * 7 + PADDING * 2, d_->title_height() + d_->cell_height() * 7 + PADDING * 2);
}

Size Calendar::compute_minimum_size() const { return compute_preferred_size(); }

Date Calendar::Impl::first_cell() const
{
    const Date first(shown_year_, shown_month_, 1);
    const int offset = (static_cast<int>(first.day_of_week()) - static_cast<int>(first_day_) + 7) % 7;
    return first.add_days(-offset);
}

Rect Calendar::Impl::cell_rect(int cell) const
{
    const float w = cell_width(), h = cell_height();
    const float x0 = (q->width.get() - w * 7) / 2;
    const float y0 = PADDING + title_height() + h;  // 曜日の行の下
    return Rect(x0 + (cell % 7) * w, y0 + (cell / 7) * h, w, h);
}

int Calendar::Impl::cell_at(const Point& p) const
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

Rect Calendar::Impl::previous_button() const
{
    const float x0 = (q->width.get() - cell_width() * 7) / 2;
    return Rect(x0, PADDING + (title_height() - BUTTON) / 2, BUTTON, BUTTON);
}

Rect Calendar::Impl::next_button() const
{
    const float x1 = (q->width.get() + cell_width() * 7) / 2;
    return Rect(x1 - BUTTON, PADDING + (title_height() - BUTTON) / 2, BUTTON, BUTTON);
}

bool Calendar::Impl::selectable(const Date& d) const { return d.is_valid() && d >= minimum_ && d <= maximum_; }

// ---- 描画 ----

void Calendar::paint(Canvas& canvas)
{
    const bool enabled_now = effectively_enabled;
    const float scale = display_scale;
    // 見出し: 前・次の月のボタンと「2026年10月」
    const TextLayout title(month_title(d_->shown_year_, d_->shown_month_), Font(16, true), scale);
    canvas.draw_text(title, Point((width.get() - title.width()) / 2, PADDING + (d_->title_height() - title.height()) / 2),
                     enabled_now ? theme::text() : theme::text_disabled());
    const Rect buttons[] = {d_->previous_button(), d_->next_button()};
    for (int k = 0; k < 2; ++k)
    {
        const Rect b = buttons[k];
        if (enabled_now && d_->hover_cell_ == -2 - k)
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
    const float w = d_->cell_width(), h = d_->cell_height();
    const Rect first_rect = d_->cell_rect(0);
    for (int i = 0; i < 7; ++i)
    {
        const int day = (static_cast<int>(d_->first_day_) + i) % 7;
        const TextLayout l(weekday_name(day), Font(), scale);
        canvas.draw_text(l, Point(first_rect.x + i * w + (w - l.width()) / 2, first_rect.y - h + (h - l.height()) / 2),
                         enabled_now ? theme::color(ColorRole::TEXT_SECONDARY) : theme::text_disabled());
    }
    // 日のマス
    const Date start = d_->first_cell();
    const Date today = Date::today();
    for (int c = 0; c < CELLS; ++c)
    {
        const Date d = start.add_days(c);
        const Rect r = d_->cell_rect(c);
        const Rect mark = r.adjusted(2, 2, 2, 2);
        const bool in_month = d.month == d_->shown_month_;
        const bool can = enabled_now && d_->selectable(d);
        const bool chosen = d == d_->date_;
        if (chosen)
        {
            canvas.fill_round_rect(mark, theme::radius(), can ? theme::accent() : theme::accent_disabled());
        }
        else if (can && c == d_->hover_cell_)
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

void Calendar::Impl::choose(const Date& d)
{
    if (!d.is_valid())
    {
        return;
    }
    const Date c = std::max(minimum_, std::min(maximum_, d));
    q->show_month(c.year, c.month);
    set_date(c);
}

void Calendar::Impl::activate()
{
    if (date_.is_null())
    {
        return;
    }
    Object* self = q;
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
    if (d_->previous_button().contains(e.pos))
    {
        show_month(Date(d_->shown_year_, d_->shown_month_, 1).add_months(-1).year, Date(d_->shown_year_, d_->shown_month_, 1).add_months(-1).month);
        return;
    }
    if (d_->next_button().contains(e.pos))
    {
        const Date next = Date(d_->shown_year_, d_->shown_month_, 1).add_months(1);
        show_month(next.year, next.month);
        return;
    }
    const int c = d_->cell_at(e.pos);
    if (c < 0)
    {
        return;
    }
    const Date d = d_->first_cell().add_days(c);
    if (!d_->selectable(d))
    {
        return;
    }
    d_->choose(d);
    d_->activate();  // ハンドラの中でポップアップを閉じてもよいよう、最後に呼ぶ
}

void Calendar::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    const int hover = d_->previous_button().contains(e.pos) ? -2 : d_->next_button().contains(e.pos) ? -3 : d_->cell_at(e.pos);
    if (hover != d_->hover_cell_)
    {
        d_->hover_cell_ = hover;
        invalidate();
    }
}

void Calendar::mouse_leave()
{
    if (d_->hover_cell_ != -1)
    {
        d_->hover_cell_ = -1;
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
    const Date next = Date(d_->shown_year_, d_->shown_month_, 1).add_months(ticks > 0 ? -1 : 1);
    show_month(next.year, next.month);
}

void Calendar::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    // 選んでいなければ、見せている月の 1 日(今日の月なら今日)から動かす
    const Date today = Date::today();
    const Date base = !d_->date_.is_null() ? d_->date_
                      : today.year == d_->shown_year_ && today.month == d_->shown_month_ ? today
                                                                                 : Date(d_->shown_year_, d_->shown_month_, 1);
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
        if (d_->date_.is_null())
        {
            d_->choose(base);
        }
        d_->activate();
        return;
    default:
        return;
    }
    e.accepted = true;
    d_->choose(d_->date_.is_null() && e.key != Key::PAGE_UP && e.key != Key::PAGE_DOWN ? base : next);
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
