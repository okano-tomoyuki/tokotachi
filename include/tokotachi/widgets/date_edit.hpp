/// @file
/// tokotachi::DateEdit の宣言。
#ifndef TOKOTACHI_WIDGETS_DATE_EDIT_HPP
#define TOKOTACHI_WIDGETS_DATE_EDIT_HPP

#include <string>

#include "../core/date.hpp"
#include "../core/export.hpp"
#include "widget.hpp"

namespace tokotachi
{

class Calendar;
class Popup;

namespace detail
{
class DateText;
}

/// 日付の入力欄(docs/adr/0051)。打ち込むか、↑↓・ホイールで年・月・日を増減するか、右のボタンで暦(Calendar)を開いて選ぶ。
///
/// @image html widget_date_edit.png
///
/// @code
/// tokotachi::DateEdit* date_edit_due = new tokotachi::DateEdit(&window);
/// date_edit_due->date = tokotachi::Date::today();
/// date_edit_due->format = "yyyy-MM-dd";                         // 既定は "yyyy/MM/dd"
/// date_edit_due->on_change = [=](tokotachi::Object*) { ... date_edit_due->date ... };
/// @endcode
///
/// 打ち込んだ文字は、Enter・フォーカスが外れたときに読む(数字の間の区切りは / - . 空白のどれでもよい。年が 2 桁以下なら 2000 年代)。
/// 空にすると空の日付、読めなければ元に戻す。範囲の外は範囲に収める。
/// ↑↓・ホイール(フォーカスがあるとき)は、キャレットのある欄(年・月・日)を 1 ずつ増減し、Page Up/Down は月を増減する。
/// Alt+↓・F4 で暦を開き、暦では矢印で日を動かし、Enter・クリックで決め、Esc で閉じる。
///
/// @ingroup widgets_input
class TOKOTACHI_API DateEdit : public Widget
{
public:
    explicit DateEdit(Widget* parent = nullptr);
    ~DateEdit() override;

    /// 日付(既定は空)。範囲の外の値は範囲に収める。
    Property<Date> date;
    /// 選べる範囲(既定は 1 年 1 月 1 日〜9999 年 12 月 31 日)。
    Property<Date> minimum;
    Property<Date> maximum;
    /// 表示の書式(yyyy・yy・MM・M・dd・d を年・月・日に置き換える。既定 "yyyy/MM/dd")。打ち込んだ文字も、この順に読む。
    Property<std::string> format;
    /// 暦の週の始まり(既定は日曜日)。
    Property<DayOfWeek> first_day_of_week;
    /// 日付が空のときに出す文字。
    Property<std::string> placeholder_text;
    /// 入力欄に出している文字(打ち込み中は、まだ date になっていないことがある)。
    ReadOnlyProperty<std::string> text;
    /// date が変わった(利用者の操作・代入のどちらでも)。
    Property<NotifyHandler> on_change;

    /// 打ち込み中の文字を date にする(Enter・フォーカスが外れたときに自動で呼ぶ)。
    void commit();
    void open_calendar();
    void close_calendar();
    bool is_calendar_open() const;

protected:
    void paint(Canvas& canvas) override;
    void mouse_down(MouseEvent& e) override;
    void mouse_wheel(WheelEvent& e) override;
    void key_down(KeyEvent& e) override;
    void arrange_children() override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
