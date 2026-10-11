#include <tokotachi/core/date.hpp>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>

namespace tokotachi
{

namespace
{

const int MIN_YEAR = 1;
const int MAX_YEAR = 9999;

// 0001-01-01 からの日数(グレゴリオ暦を過去へ延ばして数える)。
long serial(int y, int m, int d)
{
    // 3 月始まりの年に直して数える(閏日を年の終わりに置く)
    if (m <= 2)
    {
        y -= 1;
        m += 12;
    }
    const long years = y;
    return 365 * years + years / 4 - years / 100 + years / 400 + (153 * (m - 3) + 2) / 5 + d - 1 - 306;
}

Date from_serial(long n)
{
    // serial の逆。年を見積もってから合わせる
    int y = static_cast<int>(n / 365.2425) + 1;
    while (serial(y, 1, 1) > n)
    {
        --y;
    }
    while (serial(y + 1, 1, 1) <= n)
    {
        ++y;
    }
    int m = 1;
    while (m < 12 && serial(y, m + 1, 1) <= n)
    {
        ++m;
    }
    return Date(y, m, static_cast<int>(n - serial(y, m, 1)) + 1);
}

const long FIRST = serial(MIN_YEAR, 1, 1);
const long LAST = serial(MAX_YEAR, 12, 31);

Date clamped(long n) { return from_serial(std::max(FIRST, std::min(LAST, n))); }

// 年・月・日の順(pattern に y・M・d が現れた順。現れないものは年・月・日の順に後ろへ)。
std::vector<char> field_order(const std::string& pattern)
{
    std::vector<char> order;
    for (std::size_t i = 0; i < pattern.size(); ++i)
    {
        const char c = pattern[i];
        if ((c == 'y' || c == 'M' || c == 'd') && std::find(order.begin(), order.end(), c) == order.end())
        {
            order.push_back(c);
        }
    }
    const char all[] = {'y', 'M', 'd'};
    for (int k = 0; k < 3; ++k)
    {
        if (std::find(order.begin(), order.end(), all[k]) == order.end())
        {
            order.push_back(all[k]);
        }
    }
    return order;
}

}  // namespace

bool Date::is_leap_year(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

int Date::days_in_month(int y, int m)
{
    static const int DAYS[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m < 1 || m > 12)
    {
        return 0;
    }
    return m == 2 && is_leap_year(y) ? 29 : DAYS[m - 1];
}

bool Date::is_valid() const
{
    return year >= MIN_YEAR && year <= MAX_YEAR && month >= 1 && month <= 12 && day >= 1 && day <= days_in_month(year, month);
}

Date Date::today()
{
    const std::time_t now = std::time(nullptr);
    std::tm local;
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    return Date(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
}

DayOfWeek Date::day_of_week() const
{
    // 0001-01-01 は月曜日
    const long n = serial(year, month, day);
    return static_cast<DayOfWeek>(((n % 7) + 7 + 1) % 7);
}

Date Date::add_days(int days) const
{
    if (!is_valid())
    {
        return *this;
    }
    return clamped(serial(year, month, day) + days);
}

Date Date::add_months(int months) const
{
    if (!is_valid())
    {
        return *this;
    }
    long total = static_cast<long>(year) * 12 + (month - 1) + months;
    total = std::max(static_cast<long>(MIN_YEAR) * 12, std::min(static_cast<long>(MAX_YEAR) * 12 + 11, total));
    const int y = static_cast<int>(total / 12), m = static_cast<int>(total % 12) + 1;
    return Date(y, m, std::min(day, days_in_month(y, m)));
}

Date Date::add_years(int years) const { return add_months(years * 12); }

int Date::days_to(const Date& other) const
{
    return static_cast<int>(serial(other.year, other.month, other.day) - serial(year, month, day));
}

std::string Date::format(const std::string& pattern) const
{
    if (is_null())
    {
        return std::string();
    }
    std::string out;
    char buf[16];
    for (std::size_t i = 0; i < pattern.size();)
    {
        const char c = pattern[i];
        std::size_t run = 1;
        while (i + run < pattern.size() && pattern[i + run] == c)
        {
            ++run;
        }
        if (c == 'y')
        {
            if (run <= 2)
            {
                std::snprintf(buf, sizeof(buf), "%02d", year % 100);
            }
            else
            {
                std::snprintf(buf, sizeof(buf), "%04d", year);
            }
            out += buf;
        }
        else if (c == 'M' || c == 'd')
        {
            const int v = c == 'M' ? month : day;
            std::snprintf(buf, sizeof(buf), run >= 2 ? "%02d" : "%d", v);
            out += buf;
        }
        else
        {
            out.append(pattern, i, run);
        }
        i += run;
    }
    return out;
}

Date Date::parse(const std::string& text, const std::string& pattern)
{
    // 数字の並びを 3 つ取り出し、pattern の順で年・月・日にする
    std::vector<std::string> numbers;
    std::string current;
    for (std::size_t i = 0; i <= text.size(); ++i)
    {
        const bool digit = i < text.size() && text[i] >= '0' && text[i] <= '9';
        if (digit)
        {
            current += text[i];
        }
        else if (!current.empty())
        {
            numbers.push_back(current);
            current.clear();
        }
    }
    if (numbers.size() != 3)
    {
        return Date();
    }
    const std::vector<char> order = field_order(pattern);
    Date d;
    for (int k = 0; k < 3; ++k)
    {
        if (numbers[static_cast<std::size_t>(k)].size() > 4)
        {
            return Date();
        }
        const int v = std::atoi(numbers[static_cast<std::size_t>(k)].c_str());
        if (order[static_cast<std::size_t>(k)] == 'y')
        {
            d.year = numbers[static_cast<std::size_t>(k)].size() <= 2 ? 2000 + v : v;
        }
        else if (order[static_cast<std::size_t>(k)] == 'M')
        {
            d.month = v;
        }
        else
        {
            d.day = v;
        }
    }
    return d.is_valid() ? d : Date();
}

}  // namespace tokotachi
