/// @file
/// tokotachi::Date の宣言。
#ifndef TOKOTACHI_CORE_DATE_HPP
#define TOKOTACHI_CORE_DATE_HPP

#include <string>

namespace tokotachi
{

/// 曜日。
enum class DayOfWeek
{
    SUNDAY,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY
};

/// 日付(年・月・日。グレゴリオ暦。docs/adr/0051)。DateEdit・Calendar の値。
///
/// @code
/// tokotachi::Date d(2026, 10, 11);
/// d.add_days(30).format("yyyy/MM/dd");          // "2026/11/10"
/// tokotachi::Date::parse("2026-1-5", "yyyy/MM/dd");  // 区切りは / - . 空白のどれでも
/// tokotachi::Date::today();
/// @endcode
///
/// 年・月・日がすべて 0 の値は「空」(is_null)。範囲は 1 年 1 月 1 日から 9999 年 12 月 31 日。
///
/// @ingroup core_values
struct Date
{
    int year;
    int month;  ///< 1〜12
    int day;    ///< 1〜31

    /// 空の日付。
    Date() : year(0), month(0), day(0) {}
    Date(int year_, int month_, int day_) : year(year_), month(month_), day(day_) {}

    /// 今日(この計算機の時刻帯で)。
    static Date today();
    /// text を pattern(format と同じ書き方)の順の年・月・日として読む。数字の間の区切りは、数字でない文字なら何でもよい。
    /// 年が 2 桁以下なら 2000 年代とする。読めない・正しくない日付なら空。
    static Date parse(const std::string& text, const std::string& pattern = "yyyy/MM/dd");
    /// year 年 month 月の日数(月が範囲の外なら 0)。
    static int days_in_month(int year, int month);
    static bool is_leap_year(int year);

    bool is_null() const { return year == 0 && month == 0 && day == 0; }
    /// 正しい日付か(空なら false)。
    bool is_valid() const;
    DayOfWeek day_of_week() const;
    /// 日・月・年を足した日付。月・年を足して日が無くなったら、その月の最後の日にする(1 月 31 日 + 1 か月 = 2 月 28 日)。
    /// 範囲の外になったら端に止める。空・正しくない日付ならそのまま返す。
    Date add_days(int days) const;
    Date add_months(int months) const;
    Date add_years(int years) const;
    /// other までの日数(other が後なら正)。
    int days_to(const Date& other) const;
    /// pattern の yyyy・yy・MM・M・dd・d を年・月・日に置き換えた文字(それ以外の文字はそのまま)。空の日付なら空の文字。
    std::string format(const std::string& pattern = "yyyy/MM/dd") const;
};

// 比べる(Property<Date> とも比べられるよう、メンバではない関数にする)。
inline bool operator==(const Date& a, const Date& b) { return a.year == b.year && a.month == b.month && a.day == b.day; }
inline bool operator!=(const Date& a, const Date& b) { return !(a == b); }
inline bool operator<(const Date& a, const Date& b)
{
    return a.year != b.year ? a.year < b.year : a.month != b.month ? a.month < b.month : a.day < b.day;
}
inline bool operator>(const Date& a, const Date& b) { return b < a; }
inline bool operator<=(const Date& a, const Date& b) { return !(b < a); }
inline bool operator>=(const Date& a, const Date& b) { return !(a < b); }

}  // namespace tokotachi

#endif
