//
// Copyright (c) 2016-2020 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_HTTP_IMPL_DATE_IPP
#define BOOST_BEAST_HTTP_IMPL_DATE_IPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/_experimental/http/date.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/string.hpp>
#include <boost/beast/http/detail/rfc7230.hpp>
#include <algorithm>
#include <array>
#include <iterator>
#include <string>

namespace boost {
namespace beast {
namespace http {

namespace impl {

constexpr static int epoch_year = 1970;
constexpr int days_in_month[][12]{
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

template <std::size_t N>
constexpr string_view lit_to_view(const char (&lit)[N]) {
    return {lit, N - 1};
}

constexpr std::array<string_view, 7> weekdays_strings{
    lit_to_view("Sunday"),
    lit_to_view("Monday"),
    lit_to_view("Tuesday"),
    lit_to_view("Wednesday"),
    lit_to_view("Thursday"),
    lit_to_view("Friday"),
    lit_to_view("Saturday"),
};

constexpr std::array<string_view, 12> months_strings{
    lit_to_view("January"),
    lit_to_view("February"),
    lit_to_view("March"),
    lit_to_view("April"),
    lit_to_view("May"),
    lit_to_view("June"),
    lit_to_view("July"),
    lit_to_view("August"),
    lit_to_view("September"),
    lit_to_view("October"),
    lit_to_view("November"),
    lit_to_view("December")
};

BOOST_BEAST_DECL
short
weekday_from_str(string_view s) noexcept
{
  const auto it = std::find(weekdays_strings.begin(),
                            weekdays_strings.end(), s);
  return it != weekdays_strings.end() ?
         std::distance(weekdays_strings.begin(), it) : -1;
}

BOOST_BEAST_DECL
short
weekday_from_short_str(string_view s) noexcept
{
  const auto it = std::find_if(
      weekdays_strings.begin(), weekdays_strings.end(),
      [=](string_view month) { return s == month.substr(0, 3); });
  return it != weekdays_strings.end() ?
         std::distance(weekdays_strings.begin(), it) : -1;
}

BOOST_BEAST_DECL
short
month_from_short_str(string_view s) noexcept
{
    const auto it = std::find_if(
        months_strings.begin(), months_strings.end(),
        [=](string_view month) { return s == month.substr(0, 3); });
    return it != months_strings.end() ?
           std::distance(months_strings.begin(), it) + 1 : -1;
}

/*  string_view to unsigned long conversion
    Precondition: s contains a range of 0 or more numerals
*/
BOOST_BEAST_DECL
unsigned short
svtous_unchecked(string_view s) noexcept
{
    unsigned short res = 0;
    unsigned short factor = 1;
    for (auto rit = s.crbegin(); rit != s.crend(); ++rit)
    {
        res += (*rit - '0') * factor;
        factor *= 10;
    }
    return res;
}


/* Checks if a year is leap
*/
constexpr
bool
is_leap(int y) noexcept
{
    return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
}

/* Date algorithms derived from code by Howard Hinnant
*/

BOOST_BEAST_DECL
int
days_from_ymd(year_month_day ymd) noexcept
{
    using Int = int_fast32_t;
    using Uint = uint_fast32_t;
    unsigned y = ymd.year;
    const auto& m = ymd.month;
    const auto& d = ymd.day;
    y -= m <= 2;
    const Uint era = y / 400;
    const Uint yoe = static_cast<Uint>(y - era * 400);      // [0, 399]
    const Uint doy = (153*(m + (m > 2 ? -3 : 9)) + 2)/5 + d-1;  // [0, 365]
    const Uint doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]
    return era * 146097 + static_cast<Int>(doe) - 719468;
}

BOOST_BEAST_DECL
year_month_day
ymd_from_days(std::uint_fast32_t z) noexcept
{
    using Int = std::int_fast32_t;
    using Uint = std::uint_fast32_t;
    z += 719468;
    const Uint era = z / 146097;
    const Uint doe = static_cast<Uint>(z - era * 146097); // [0, 146096]
    const Uint yoe =
        (doe - doe/1460 + doe/36524 - doe/146096) / 365;  // [0, 399]
    const Int y = static_cast<Int>(yoe) + era * 400;
    const Uint doy = doe - (365*yoe + yoe/4 - yoe/100);   // [0, 365]
    const Uint mp = (5*doy + 2)/153;                      // [0, 11]
    const std::uint_least8_t d = doy - (153*mp+2)/5 + 1;  // [1, 31]
    const std::uint_least8_t m = mp + (mp < 10 ? 3 : -9); // [1, 12]
    return {static_cast<std::uint_least16_t>(y + (m <= 2)), m, d};
}

BOOST_BEAST_DECL
uint_least8_t
weekday_from_days(int days) noexcept {
    return static_cast<uint_least8_t>(
        static_cast<unsigned>(days >= -4 ? (days+4) % 7 : (days+5) % 7 + 6));
}

BOOST_BEAST_DECL
uint_least8_t
weekday_from_date(year_month_day ymd) noexcept {
    return weekday_from_days(days_from_ymd(ymd));
}

BOOST_BEAST_DECL
bool
check_day_of_month(year_month_day const& ymd) noexcept
{
    if(ymd.day < 1)
        return false;
    if(ymd.day > days_in_month[+is_leap(ymd.year)][ymd.month])
        return false;

    return true;
}

BOOST_BEAST_DECL
bool
check_datetime(date_time const& dt) noexcept
{
    if(dt.date.year < epoch_year)
        return false;
    if(dt.date.year > 9999)
        return false;
    if(dt.date.month < 1)
        return false;
    if(dt.date.month > 12)
        return false;
    if(!check_day_of_month(dt.date))
        return false;

    if(dt.time.hour.count() > 23)
        return false;
    if(dt.time.minute.count() > 59)
        return false;
    if(dt.time.second.count() > 59)
        return false;

    return true;
}

/*  Formats: dd$Mmm$ (where $ is sep)
    Precondition: s.length() >= 7
*/
BOOST_BEAST_DECL
boost::optional<year_month_day>
parse_daymonth(string_view s, char sep) noexcept
{
    if (!(detail::is_digit(s[0]) && detail::is_digit(s[1])))
        return boost::none;
    const auto day = svtous_unchecked({&s[0], 2});

    if (s[2] != sep)
        return boost::none;

    const auto month = month_from_short_str(s.substr(3, 3));
    if (month == -1)
        return boost::none;

    if (s[6] != sep)
        return boost::none;

    return {{epoch_year,
             static_cast<uint_least8_t>(month),
             static_cast<uint_least8_t>(day)}};
}

/*  Format: hh:mm:ss
    Precondition: s.length() >= 8
*/
BOOST_BEAST_DECL
boost::optional<time_of_day>
parse_time(string_view s) noexcept
{
    if(!((s[0] == '0' || s[0] == '1') && detail::is_digit(s[1])) &&
       !(s[0] == '2' && (s[1] >= '0' && s[1] <= '3')))
        return boost::none;
    const std::chrono::hours hour{svtous_unchecked(s.substr(0, 2))};

    if(s[2] != ':')
        return boost::none;

    if(s[3] < '0' || s[3] > '5')
        return boost::none;
    if(!detail::is_digit(s[4]))
        return boost::none;
    const std::chrono::minutes minute{svtous_unchecked(s.substr(3, 2))};

    if(s[5] != ':')
        return boost::none;

    if(s[6] < '0' || s[6] > '5')
        return boost::none;
    if(!detail::is_digit(s[7]))
        return boost::none;
    const std::chrono::seconds second{svtous_unchecked(s.substr(6, 2))};

    return {{hour, minute, second}};
}

/*  Format: "Www, dd Mmm yyyy hh:mm:ss GMT" (fixed length)
    Preconds:
    - s[3] is a space character (`' '`)
*/
BOOST_BEAST_DECL
boost::optional<date_time>
parse_rfc1123(string_view s) noexcept
{
    constexpr std::size_t fmt_len = sizeof("Www, dd Mmm yyyy hh:mm:ss GMT") - 1;
    if(s.length() < fmt_len)
        return boost::none;

    const auto weekday = weekday_from_short_str(s.substr(0, 3));
    if (weekday == -1)
        return boost::none;

    // No need to validate s[3] as its value is a precondition

    if (s[4] != ' ')
        return boost::none;

    const auto day_month_opt = parse_daymonth(s.substr(5, 7), ' ');
    if (!day_month_opt)
        return boost::none;
    const auto& day_month = *day_month_opt;


    if (!std::all_of(&s[12], &s[16], detail::is_digit))
        return boost::none;
    const auto year = svtous_unchecked(s.substr(12, 4));
    if(year < 1970)
        return boost::none;
    const year_month_day full_date = {year, day_month.month, day_month.day};
    if(!check_day_of_month(full_date))
        return boost::none;
    if(weekday_from_date(full_date) != weekday)
        return boost::none;

    if (s[16] != ' ')
        return boost::none;

    const auto time_opt = parse_time(s.substr(17, 8));
    if(!time_opt)
        return boost::none;
    const auto& time = *time_opt;

    if(s.substr(25, 4) != " GMT")
        return boost::none;

    return {{full_date, time}};
}

/*  Format: "Wwww, dd-Mmm-yy hh:mm:ss GMT"
    Preconds:
     - s.length >= 23
     - s[3] is a ','
*/
BOOST_BEAST_DECL
boost::optional<date_time>
parse_rfc850(string_view s) noexcept
{
    constexpr auto longest_weekday_len = sizeof("Wednesday") - 1;

    const char* const it =
        std::find(s.data(), s.data() + longest_weekday_len, ',');
    if(it == s.data() + longest_weekday_len)
        return boost::none;

    const string_view weekday_sv{s.substr(0, it - s.data())};
    const auto weekday = weekday_from_str(weekday_sv);
    if(weekday == -1)
        return boost::none;

    const auto full_datetime_length =
        weekday_sv.length() + (sizeof(", dd-Mmm-yy hh:mm:ss GMT") - 1);
    if(s.length() < full_datetime_length)
        return boost::none;

    s = s.substr(it - s.data() + 1);

    if(s[0] != ' ')
        return boost::none;

    const auto day_month_opt = parse_daymonth(s.substr(1, 11), '-');
    if(!day_month_opt)
        return boost::none;
    const auto& day_month = *day_month_opt;

    if(!std::all_of(&s[8], &s[10], detail::is_digit))
        return boost::none;
    auto year_value = svtous_unchecked(s.substr(8, 2));
    year_value += (year_value < 70 ? 2000 : 1900);
    const year_month_day full_date = {year_value, day_month.month, day_month.day};
    if(!check_day_of_month(full_date))
        return boost::none;
    if(weekday_from_date(full_date) != weekday)
        return boost::none;

    if(s[10] != ' ')
        return boost::none;

    const auto time_opt = parse_time(s.substr(11, 8));
    if(!time_opt)
        return boost::none;
    const auto& time = *time_opt;

    if(s.substr(19, 4) != " GMT")
        return boost::none;

    return {{full_date, time}};
}

/*  Format: "Www Mmm  d hh:mm:ss yyyy"
    Preconds:
    - s.length >= 24
*/
BOOST_BEAST_DECL
boost::optional<date_time>
parse_asctime(string_view s) noexcept
{
    const auto weekday = weekday_from_short_str(s.substr(0, 3));
    if(weekday == -1)
        return boost::none;

    // No need to validate s[3] as its value is a precondition

    const auto month = month_from_short_str(s.substr(4, 3));
    if(month == -1)
        return boost::none;

    if(s[7] != ' ')
        return boost::none;

    if(!detail::is_digit(s[9]))
        return boost::none;
    int day = s[9] - '0';
    if(s[8] >= '1' && s[8] <= '3')
        day += (s[8] - '0') * 10;
    else if(s[8] != ' ')
        return boost::none;
    if(day > 31)
        return boost::none;


    if(s[10] != ' ')
        return boost::none;

    const auto time_opt = parse_time(s.substr(11, 8));
    if(!time_opt)
        return boost::none;
    const auto& time = *time_opt;

    if(s[19] != ' ')
        return boost::none;

    if(!std::all_of(&s[20], &s[24], detail::is_digit))
        return boost::none;
    const auto year = svtous_unchecked(s.substr(20, 4));
    if(year < epoch_year)
        return boost::none;
    const year_month_day full_date = {year,
                                      static_cast<uint_least8_t>(month),
                                      static_cast<uint_least8_t>(day)};
    if(!check_day_of_month(full_date))
        return boost::none;
    if(weekday_from_date(full_date) != weekday)
        return boost::none;

    return {{full_date, time}};
}

/*  Format: "Www, dd Mmm yyyy hh:mm:ss GMT" (fixed length)
*/
void
stringify_datetime_at_unchecked(date_time const& dt, char* storage) noexcept
{
    const auto copy_at = [](char* dest, string_view source)
    {
        std::copy(source.begin(), source.end(), dest);
    };

    const auto put_decimals = [](char* dest, unsigned short usi, std::size_t n)
    {
        unsigned factor = 1;
        for (unsigned i = 0; i < n - 1; ++i)
            factor *= 10;

        for (;;)
        {
            const auto div_res = std::div(usi, factor);
            *dest = div_res.quot + '0';
            usi = div_res.rem;
            if(factor == 1)
                break;
            ++dest;
            factor /= 10;
        }
    };

    const auto& ymd = dt.date;
    copy_at(storage, impl::weekdays_strings[(days_from_ymd(ymd) + 4) % 7]);
    copy_at(storage + 3, ", ");
    put_decimals(storage + 5, ymd.day, 2);
    storage[7] = ' ';
    copy_at(storage + 8, impl::months_strings[ymd.month - 1]);
    storage[11] = ' ';
    put_decimals(storage + 12, ymd.year, 4);
    storage[16] = ' ';
    const auto& time = dt.time;
    put_decimals(storage + 17, time.hour.count(), 2);
    storage[19] = ':';
    put_decimals(storage + 20, time.minute.count(), 2);
    storage[22] = ':';
    put_decimals(storage + 23, time.second.count(), 2);
    copy_at(storage + 25, " GMT");
}

}


boost::optional<date_time>
parse_datetime(string_view http_date_str) noexcept
{
    constexpr std::size_t shortest_date_length = sizeof("Sun Nov  6 08:49:37 1994") - 1;
    if(http_date_str.length() < shortest_date_length)
        return boost::none;

    if(http_date_str[3] == ',')
        return impl::parse_rfc1123(http_date_str);
    if(http_date_str[3] == ' ')
        return impl::parse_asctime(http_date_str);
    return impl::parse_rfc850(http_date_str);
}

bool
stringify_datetime_at(date_time const& dt, char* storage) noexcept
{
    BOOST_ASSERT(storage != nullptr);
    if(!impl::check_datetime(dt))
        return false;

    impl::stringify_datetime_at_unchecked(dt, storage);
    return true;
}

std::string
stringify_datetime(date_time const& dt)
{
    std::string s{};
    if(!impl::check_datetime(dt))
        return s;

    s.resize(29);
    impl::stringify_datetime_at_unchecked(dt, &s[0]);
    return s;
}

bool
operator==(year_month_day lhs, year_month_day rhs) noexcept
{
    return (lhs.year == rhs.year)
        && (lhs.month == rhs.month)
        && (lhs.day == rhs.day);
}

bool
operator!=(year_month_day lhs, year_month_day rhs) noexcept
{
    return !(lhs == rhs);
}

bool
operator<(year_month_day lhs, year_month_day rhs) noexcept
{
    return (lhs.year < rhs.year)
        && (lhs.month < rhs.month)
        && (lhs.day < rhs.day);
}

bool
operator>(year_month_day lhs, year_month_day rhs) noexcept
{
    return (lhs.year > rhs.year)
        && (lhs.month > rhs.month)
        && (lhs.day > rhs.day);
}

bool
operator<=(year_month_day lhs, year_month_day rhs) noexcept
{
    return !(lhs > rhs);
}

bool
operator>=(year_month_day lhs, year_month_day rhs) noexcept
{
    return !(lhs < rhs);
}


bool
operator==(time_of_day const& lhs, time_of_day const& rhs) noexcept
{
    return (lhs.hour == rhs.hour)
        && (lhs.minute == rhs.minute)
        && (lhs.second == rhs.second);
}

bool
operator!=(time_of_day const& lhs, time_of_day const& rhs) noexcept
{
    return !(lhs == rhs);
}

bool
operator<(time_of_day const& lhs, time_of_day const& rhs) noexcept
{
    return (lhs.hour < rhs.hour)
        && (lhs.minute < rhs.minute)
        && (lhs.second < rhs.second);
}

bool
operator>(time_of_day const& lhs, time_of_day const& rhs) noexcept
{
    return (lhs.hour > rhs.hour)
        && (lhs.minute > rhs.minute)
        && (lhs.second > rhs.second);
}

bool
operator<=(time_of_day const& lhs, time_of_day const& rhs) noexcept
{
    return !(lhs > rhs);
}

bool
operator>=(time_of_day const& lhs, time_of_day const& rhs) noexcept
{
    return !(lhs < rhs);
}


bool
operator==(date_time const& lhs, date_time const& rhs) noexcept
{
    return (lhs.date == rhs.date) && (lhs.time == rhs.time);
}

bool
operator!=(date_time const& lhs, date_time const& rhs) noexcept
{
    return !(lhs == rhs);
}

bool
operator<(date_time const& lhs, date_time const& rhs) noexcept
{
    return (lhs.date < rhs.date) && (lhs.time < rhs.time);
}

bool
operator>(date_time const& lhs, date_time const& rhs) noexcept
{
    return (lhs.date > rhs.date) && (lhs.time > rhs.time);
}

bool
operator<=(date_time const& lhs, date_time const& rhs) noexcept
{
    return !(lhs > rhs);
}

bool
operator>=(date_time const& lhs, date_time const& rhs) noexcept
{
    return !(lhs < rhs);
}


unsigned long long
to_posix(date_time const& dt) noexcept
{
    unsigned long long result{};
    result += impl::days_from_ymd(dt.date) * 24 * 3600;
    result += dt.time.hour.count() * 3600;
    result += dt.time.minute.count() * 60;
    result += dt.time.second.count();
    return result;
}

date_time
from_posix(unsigned long long t) noexcept
{
    using std::chrono::duration_cast;
    const auto ymd = impl::ymd_from_days(t / (3600 * 24));
    std::chrono::seconds rem{t % (3600 * 24)};
    const auto hour = duration_cast<std::chrono::hours>(rem);
    rem -= hour;
    const auto minute = duration_cast<std::chrono::minutes>(rem);
    rem -= minute;
    return {ymd, {hour, minute, rem}};
}

} // namespace http
} // namespace beast
} // namespace boost

#endif