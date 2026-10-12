/// @file
/// tokotachi::Calendar の宣言。
#ifndef TOKOTACHI_WIDGETS_CALENDAR_HPP
#define TOKOTACHI_WIDGETS_CALENDAR_HPP

#include "../core/date.hpp"
#include "widget.hpp"

namespace tokotachi
{

/// 月ごとの暦から日を選ぶ(docs/adr/0051)。DateEdit のポップアップの中身にもなる。
///
/// @image html widget_calendar.png
///
/// @code
/// tokotachi::Calendar* calendar_due = new tokotachi::Calendar(&window);
/// calendar_due->date = tokotachi::Date::today();
/// calendar_due->on_change = [=](tokotachi::Object*) { ... calendar_due->date ... };
/// @endcode
///
/// 日を押すと選び(on_change)、on_activate を呼ぶ。上の「&lt;」「&gt;」・ホイール・Page Up/Down で月を、Shift+Page Up/Down で年を変える。
/// 矢印で日を動かし(上下は 1 週)、Home・End で月の始め・終わりへ、Enter・Space で on_activate を呼ぶ。
/// minimum〜maximum の外の日は選べない。今日の日は輪で示す。
///
/// @ingroup widgets_input
class Calendar : public Widget
{
public:
    explicit Calendar(Widget* parent = nullptr);

    /// 選んでいる日(空なら選んでいない)。代入すると、その月を見せる。範囲の外の日は範囲に収める。
    Property<Date> date;
    /// 選べる範囲(既定は 1 年 1 月 1 日〜9999 年 12 月 31 日)。
    Property<Date> minimum;
    Property<Date> maximum;
    /// 週の始まりの曜日(既定は日曜日)。
    Property<DayOfWeek> first_day_of_week;
    /// 見せている月。
    ReadOnlyProperty<int> shown_year;
    ReadOnlyProperty<int> shown_month;
    /// year 年 month 月を見せる(選んでいる日は変えない)。
    void show_month(int year, int month);

    /// date が変わった(利用者の操作・代入のどちらでも)。
    Property<NotifyHandler> on_change;
    /// 日を押した・Enter を押した(選び終えた)。
    Property<NotifyHandler> on_activate;

protected:
    void paint(Canvas& canvas) override;
    void mouse_down(MouseEvent& e) override;
    void mouse_move(MouseEvent& e) override;
    void mouse_leave() override;
    void mouse_wheel(WheelEvent& e) override;
    void key_down(KeyEvent& e) override;
    void focus_in() override;
    void focus_out() override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
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

}  // namespace tokotachi

#endif
