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
#include <boost/date_time/local_time/local_date_time.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <algorithm>
#include <array>
#include <iterator>
#include <string>

namespace boost {
namespace beast {
namespace http {

namespace impl {

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

/*
BOOST_BEAST_DECL
short
month_from_str(string_view s) noexcept
{
    const auto it = std::find(months_strings.begin(), months_strings.end(), s);
    return it != months_strings.end() ?
        std::distance(months_strings.begin(), it) + 1 : -1;
}
*/

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

/*  Formats: dd$Mmm$ (where $ is sep)
    Precondition: s.length() >= 7
*/
BOOST_BEAST_DECL
boost::gregorian::date
parse_daymonth(string_view s, char sep) noexcept
{
    using namespace boost::gregorian;
    using boost::date_time::special_values::not_a_date_time;
    const auto error = []{ return date{not_a_date_time}; };

    if (!(detail::is_digit(s[0]) && detail::is_digit(s[1])))
        return error();
    const greg_day day = svtous_unchecked({&s[0], 2});

    if (s[2] != sep)
        return error();

    const auto month_value = month_from_short_str(s.substr(3, 3));
    if (month_value == -1)
        return error();
    const greg_month month(month_value);

    if (s[6] != sep)
        return error();

    return {1970, month, day};
}

/*  Format: hh:mm:ss
    Precondition: s.length() >= 8
*/
BOOST_BEAST_DECL
boost::posix_time::time_duration
parse_time(string_view s) noexcept
{
    const auto error = []{ return boost::date_time::not_a_date_time; };
    if(!((s[0] == '0' || s[0] == '1') && detail::is_digit(s[1])) &&
       !(s[0] == '2' && (s[1] >= '0' && s[1] <= '3')))
        return error();
    const auto hour = svtous_unchecked(s.substr(0, 2));

    if(s[2] != ':')
        return error();

    if(s[3] < '0' || s[3] > '5')
        return error();
    if(!detail::is_digit(s[4]))
        return error();
    const auto minute = svtous_unchecked(s.substr(3, 2));

    if(s[5] != ':')
        return error();

    if(s[6] < '0' || s[6] > '5')
        return error();
    if(!detail::is_digit(s[7]))
        return error();
    const auto second = svtous_unchecked(s.substr(6, 2));

    return {hour, minute, second};
}

/*  Format: "Www, dd Mmm yyyy hh:mm:ss GMT" (fixed length)
    Preconds:
    - s[3] is a space character (`' '`)
*/
BOOST_BEAST_DECL
date_time
parse_rfc1123(string_view s) noexcept
{
    using namespace boost::gregorian;
    using boost::date_time::special_values::not_a_date_time;
    const auto error = []{ return date_time{not_a_date_time, nullptr}; };

    constexpr std::size_t fmt_len = sizeof("Www, dd Mmm yyyy hh:mm:ss GMT") - 1;
    if(s.length() < fmt_len)
        return error();

    const auto wkday_value = weekday_from_short_str(s.substr(0, 3));
    if (wkday_value == -1)
        return error();
    const greg_weekday wkday(wkday_value);

    // No need to validate s[3] as its value is a precondition

    if (s[4] != ' ')
        return error();

    const auto day_month = parse_daymonth(s.substr(5, 7), ' ');
    if (day_month.is_not_a_date())
        return error();

    if (!std::all_of(&s[12], &s[16], detail::is_digit))
        return error();
    const auto year = svtous_unchecked(s.substr(12, 4));
    if(year < 1400)
        return error();
    const date full_date = {year, day_month.month(), day_month.day()};
     if(full_date.day_of_week() != wkday)
        return error();

    if (s[16] != ' ')
        return error();

    const auto time_of_day = parse_time(s.substr(17, 8));
    if(time_of_day.is_not_a_date_time())
        return error();


    if(s.substr(25, 4) != " GMT")
        return error();

    return {full_date, time_of_day, nullptr, false};
}

/*  Format: "Wwww, dd-Mmm-yy hh:mm:ss GMT"
    Preconds:
     - s.length >= 23
     - s[3] is a ','
*/
BOOST_BEAST_DECL
date_time
parse_rfc850(string_view s) noexcept
{
    using namespace boost::gregorian;
    using boost::date_time::special_values::not_a_date_time;
    const auto error = []{ return date_time{not_a_date_time, nullptr}; };

    constexpr auto longest_weekday_len = sizeof("Wednesday") - 1;

    const char* const it =
        std::find(s.data(), s.data() + longest_weekday_len, ',');
    if(it == s.data() + longest_weekday_len)
        return error();

    const string_view weekday_sv{s.substr(0, it - s.data())};
    const auto weekday_value = weekday_from_str(weekday_sv);
    if(weekday_value == -1)
        return error();
    const greg_weekday wkday(weekday_value);

    const auto full_datetime_length =
        weekday_sv.length() + (sizeof(", dd-Mmm-yy hh:mm:ss GMT") - 1);
    if(s.length() < full_datetime_length)
        return error();

    s = s.substr(it - s.data() + 1);

    if(s[0] != ' ')
        return error();

    const auto day_month = parse_daymonth(s.substr(1, 11), '-');
    if(day_month.is_not_a_date())
        return error();

    if(!std::all_of(&s[8], &s[10], detail::is_digit))
        return error();
    auto year_value = svtous_unchecked(s.substr(8, 2));
    year_value += (year_value < 70 ? 2000 : 1900);
    const date full_date = {year_value, day_month.month(), day_month.day()};
    if(full_date.day_of_week() != wkday)
        return error();

    if(s[10] != ' ')
        return error();

    const auto time = parse_time(s.substr(11, 8));
    if(time.is_not_a_date_time())
        return error();

    if(s.substr(19, 4) != " GMT")
        return error();

    return {full_date, time, nullptr, false};
}

/*  Format: "Www Mmm  d hh:mm:ss yyyy"
    Preconds:
    - s.length >= 24
*/
BOOST_BEAST_DECL
date_time
parse_asctime(string_view s) noexcept
{
    using namespace boost::gregorian;
    using boost::date_time::special_values::not_a_date_time;
    const auto error = []{ return date_time{not_a_date_time, nullptr}; };

    const auto wkday_value = weekday_from_short_str(s.substr(0, 3));
    if (wkday_value == -1)
        return error();
    const greg_weekday wkday(wkday_value);

    // No need to validate s[3] as its value is a precondition

    const auto month_value = month_from_short_str(s.substr(4, 3));
    if(month_value == -1)
        return error();
    const greg_month month(month_value);

    if(s[7] != ' ')
        return error();

    if(!detail::is_digit(s[9]))
        return error();
    int day = s[9] - '0';
    if(s[8] >= '1' && s[8] <= '3')
        day += (s[8] - '0') * 10;
    else if(s[8] != ' ')
        return error();
    if(day > 31)
        return error();


    if(s[10] != ' ')
        return error();

    const auto time_of_day = parse_time(s.substr(11, 8));
    if(time_of_day.is_not_a_date_time())
        return error();

    if(s[19] != ' ')
        return error();

    if(!std::all_of(&s[20], &s[24], detail::is_digit))
        return error();
    const auto year = svtous_unchecked(s.substr(20, 4));
    if(year < 1400)
        return error();
    const date full_date = {year, month, day};
    if(full_date.day_of_week() != wkday)
        return error();

    return {full_date, time_of_day, nullptr, false};
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

    const auto date = dt.date();
    const auto ymd = date.year_month_day();
    copy_at(storage, impl::weekdays_strings[date.day_of_week()]);
    copy_at(storage + 3, ", ");
    put_decimals(storage + 5, ymd.day, 2);
    storage[7] = ' ';
    copy_at(storage + 8, impl::months_strings[ymd.month - 1]);
    storage[11] = ' ';
    put_decimals(storage + 12, ymd.year, 4);
    storage[16] = ' ';
    const auto time = dt.time_of_day();
    put_decimals(storage + 17, time.hours(), 2);
    storage[19] = ':';
    put_decimals(storage + 20, time.minutes(), 2);
    storage[22] = ':';
    put_decimals(storage + 23, time.seconds(), 2);
    copy_at(storage + 25, " GMT");
}

}

/** Turns an HTTP date string into a date_time

    RFC2616 defines HTTP dates with the following ABNF:
    HTTP-date    = rfc1123-date | rfc850-date | asctime-date
    rfc1123-date = wkday "," SP date1 SP time SP "GMT"
    rfc850-date  = weekday "," SP date2 SP time SP "GMT"
    asctime-date = wkday SP date3 SP time SP 4DIGIT
    date1        = 2DIGIT SP month SP 4DIGIT
                   ; day month year (e.g., 02 Jun 1982)
    date2        = 2DIGIT "-" month "-" 2DIGIT
                   ; day-month-year (e.g., 02-Jun-82)
    date3        = month SP ( 2DIGIT | ( SP 1DIGIT ))
                   ; month day (e.g., Jun  2)
    time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT
                   ; 00:00:00 - 23:59:59
    wkday        = "Mon" | "Tue" | "Wed"
                 | "Thu" | "Fri" | "Sat" | "Sun"
    weekday      = "Monday" | "Tuesday" | "Wednesday"
                 | "Thursday" | "Friday" | "Saturday" | "Sunday"
    month        = "Jan" | "Feb" | "Mar" | "Apr"
                 | "May" | "Jun" | "Jul" | "Aug"
                 | "Sep" | "Oct" | "Nov" | "Dec"

    @return The date_time with date information,
    or set to not_a_date_time on error
*/
date_time
parse_datetime(string_view http_date_str) noexcept
{
    using boost::date_time::special_values::not_a_date_time;

    constexpr std::size_t shortest_date_length = sizeof("Sun Nov  6 08:49:37 1994") - 1;
    if(http_date_str.length() < shortest_date_length)
        return date_time{not_a_date_time, nullptr};;

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
    if(dt.is_special())
        return false;

    impl::stringify_datetime_at_unchecked(dt, storage);
    return true;
}

std::string
stringify_datetime(date_time const& dt)
{
    std::string s{};
    if(dt.is_special())
        return s;

    s.resize(29);
    impl::stringify_datetime_at_unchecked(dt, &s[0]);
    return s;
}

} // namespace http
} // namespace beast
} // namespace boost

#endif