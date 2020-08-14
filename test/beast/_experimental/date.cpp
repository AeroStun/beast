//
// Copyright (c) 2016-2020 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

// Test that header file is self-contained.
#include <boost/beast/_experimental/http/date.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <chrono>
#include <ctime>
#include <string>

namespace boost {
namespace beast {
namespace http {

class date_test : public unit_test::suite
{
public:
    constexpr static auto rfc1123_fmt = "%a, %d %b %Y %H:%M:%S GMT";

    static
    date_time
    make_dt(int y, int mo, int d, int h, int mi, int s) noexcept {
        return {
            {static_cast<uint_least16_t>(y),
             static_cast<uint_least8_t>(mo),
             static_cast<uint_least8_t>(d)},
            {std::chrono::hours{h},
             std::chrono::minutes{mi},
             std::chrono::seconds{s}}};
    }

    static
    int
    zeller_weekday(year_month_day ymd) noexcept {
        int adjustment = (14 - ymd.month) / 12;
        int mm = ymd.month + 12 * adjustment - 2;
        int yy = ymd.year - adjustment;
        return (ymd.day + (13 * mm - 1) / 5 +
                yy + yy / 4 - yy / 100 + yy / 400) % 7;
    }

    static
    std::tm
    dt_to_tm(date_time const& pt)
    {
        std::tm tm{};
        const auto& date = pt.date;
        tm.tm_year = date.year - 1900;
        tm.tm_mon = date.month - 1;
        tm.tm_mday = date.day;
        tm.tm_wday = zeller_weekday(date);
        const auto& tod = pt.time;
        tm.tm_hour = tod.hour.count();
        tm.tm_min = tod.minute.count();
        tm.tm_sec = tod.second.count();
        return tm;
    }

    static
    void
    test_parse(string_view str, boost::optional<date_time> expected)
    {
        BEAST_EXPECT(parse_datetime(str) == expected);
    }

    static
    void
    test_stringify_against_cfmt(date_time const& dt, size_t len, const char* fmt)
    {
        const auto ours = stringify_datetime(dt);
        std::string s(len, '\0');
        const bool at_res = stringify_datetime_at(dt, &s[0]);
        if(fmt) {
            const auto tm = dt_to_tm(dt);
            std::string cstds;
            cstds.resize(len);
            BEAST_EXPECT(std::strftime(&cstds[0], len + 1, fmt, &tm) == len);
            BEAST_EXPECT(at_res);
            BEAST_EXPECT(ours == cstds);
            BEAST_EXPECT(s == cstds);
        }
        else
        {
            BEAST_EXPECT(ours.empty());
            BEAST_EXPECT(!at_res);
        }
    }

    static
    void
    test_to_posix(date_time const& dt)
    {
        const auto tm = dt_to_tm(dt);
        const std::time_t ours = to_posix(dt);
        const std::tm res = *std::gmtime(&ours);
        BEAST_EXPECT(res.tm_year == tm.tm_year);
        BEAST_EXPECT(res.tm_mon == tm.tm_mon);
        BEAST_EXPECT(res.tm_mday == tm.tm_mday);
        BEAST_EXPECT(res.tm_hour == tm.tm_hour);
        BEAST_EXPECT(res.tm_min == tm.tm_min);
        BEAST_EXPECT(res.tm_sec == tm.tm_sec);
    }

    static
    void
    test_parse()
    {
        test_parse("", {});
        test_parse("<!DOCTYPE HTML><html><head><m", {});

        // RFC1123
        test_parse("Sun, 06 Nov 1994 08:49:37 GMT",
                   make_dt(1994, 11, 6, 8, 49, 37));
        test_parse("Sun, 06 Nov 1994 08:49:37 CET", {});
        test_parse("NaD, 06 Nov 1994 08:49:37 GMT", {});
        test_parse("Sun,;06 Nov 1994 08:49:37 GMT", {});
        test_parse("Sun, aa Nov 1994 08:49:37 GMT", {});
        test_parse("Sun, 06:Nov 1994 08:49:37 GMT", {});
        test_parse("Sun, 06 Nov:1994 08:49:37 GMT", {});
        test_parse("Sun, 06 Nov 1234 08:49:37 GMT", {});
        test_parse("Sun, 06 Nov 1994;08:49:37 GMT", {});
        test_parse("Sun, 06 Nov numb 08:49:37 GMT", {});
        test_parse("Sun, 06 Nov 1994 08-49-37 GMT", {});
        test_parse("Sun, 06 Nov 1994 08-49:37 GMT", {});
        test_parse("Sun, 06 Nov 1994 hh-49:37 GMT", {});
        test_parse("Sun, 06 Nov 1994 0h-49:37 GMT", {});
        test_parse("Sun, 06 Nov 1994 08:49-37 GMT", {});
        test_parse("Sun, 06 Nov 1994 08:mm-37 GMT", {});
        test_parse("Sun, 06 Nov 1994 08:0m-37 GMT", {});
        test_parse("Sun, 06 Nov 1994 08:49:0s GMT", {});
        test_parse("Sun, 06 Nov 1994 08:49:ss GMT", {});
        test_parse("Sun, 06 Foo 1994 08:49:37 GMT", {});
        test_parse("Sun, 06 Jan 1994 08:49:37 GMT", {});
        test_parse("Sun, 06 Nov 1995 08:49:37 GMT", {});
        test_parse("Sun, 06 Nov 1994 24:49:37 GMT", {});
        test_parse("Sun, 06 Nov 1994 08:60:37 GMT", {});
        test_parse("Sun, 06 Nov 1994 08:49:60 GMT", {});
        test_parse("Sun,Nov  6 08:49:37 1994", {});

        // RFC850
        test_parse("Saturday, 08-Aug-20 19:06:22 GMT",
                  make_dt(2020, 8, 8, 19, 6, 22));
        test_parse("Sunday, 06-Nov-94 08:49:37 GMT",
                   make_dt(1994, 11, 6, 8, 49, 37));
        test_parse("Monday, 08-Aug-20 19:06:22 GMT", {});
        test_parse("Foobarday, 08-Aug-20 19:06:22 GMT", {});
        test_parse("Otherday, 08-Aug-20 19:06:22 GMT", {});
        test_parse("Saturday,;08-Aug-20 19:06:22 GMT", {});
        test_parse("Saturday, ab-Aug-20 19:06:22 GMT", {});
        test_parse("Saturday, 08 Aug-20 19:06:22 GMT", {});
        test_parse("Saturday, 08-Baz-20 19:06:22 GMT", {});
        test_parse("Saturday, 08-Aug 20 19:06:22 GMT", {});
        test_parse("Saturday, 08-Aug-yy 19:06:22 GMT", {});
        test_parse("Saturday, 08-Aug-20;19:06:22 GMT", {});
        test_parse("Saturday, 08-Aug-20 hh:06:22 GMT", {});
        test_parse("Saturday, 08-Aug-20 19:mm:22 GMT", {});
        test_parse("Saturday, 08-Aug-20 19:06:ss GMT", {});
        test_parse("Saturday, 08-Aug-20 19:06:22 CET", {});
        test_parse("Saturday, 08-Aug-20 19:06:22", {});

        // ANSI C time
        test_parse("Sun Nov  6 08:49:37 1994",
                   make_dt(1994, 11, 6, 8, 49, 37));
        test_parse("Wed Nov 16 08:49:37 1994",
                   make_dt(1994, 11, 16, 8, 49, 37));
        test_parse("Foo Nov  6 08:49:37 1994", {});
        test_parse("Sat Nov  6 08:49:37 1994", {});
        test_parse("Sun-Nov  6 08:49:37 1994", {});
        test_parse("Sun Bar  6 08:49:37 1994", {});
        test_parse("Sun Nov\t 6 08:49:37 1994", {});
        test_parse("Sun Nov  l 08:49:37 1994", {});
        test_parse("Sun Nov 06 08:49:37 1994", {});
        test_parse("Sun Nov 32 08:49:37 1994", {});
        test_parse("Sun Nov  6\t08:49:37 1994", {});
        test_parse("Sun Nov  6 __time__ 1994", {});
        test_parse("Sun Nov  6 08:49:37-1994", {});
        test_parse("Sun Nov  6 08:49:37 1234", {});
        test_parse("Sun Nov  6 08:49:37 abcd", {});
    }

    static
    void
    test_stringify()
    {
        auto dt = make_dt(2020, 8, 7, 19, 52, 12);
        test_stringify_against_cfmt(dt, 29, rfc1123_fmt);
        dt = make_dt(1970, 1, 1, 0, 0, 0);
        test_stringify_against_cfmt(dt, 29, rfc1123_fmt);

        dt = make_dt(2010, 5, 3, 24, 60, 60);
        test_stringify_against_cfmt(dt, 29, nullptr);
    }

    static
    void
    test_to_posix()
    {
        test_to_posix(make_dt(1970, 1, 1, 0, 0, 0));
        test_to_posix(make_dt(2020, 8, 14, 17, 17, 33));
    }

    static
    void
    test_from_posix()
    {
        const auto now = std::time(nullptr);
        const auto dt = from_posix(now);
        const std::tm tm = *std::gmtime(&now);
        const auto res = dt_to_tm(dt);
        BEAST_EXPECT(res.tm_year == tm.tm_year);
        BEAST_EXPECT(res.tm_mon == tm.tm_mon);
        BEAST_EXPECT(res.tm_mday == tm.tm_mday);
        BEAST_EXPECT(res.tm_hour == tm.tm_hour);
        BEAST_EXPECT(res.tm_min == tm.tm_min);
        BEAST_EXPECT(res.tm_sec == tm.tm_sec);
    }

    void
    run() override
    {
        test_parse();
        test_stringify();
        test_to_posix();
        test_from_posix();
    }
};

BEAST_DEFINE_TESTSUITE(beast,http,date);

}
}
}

