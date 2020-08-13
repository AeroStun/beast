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
#include <boost/date_time/local_time/local_date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <ctime>
#include <string>

namespace boost {
namespace beast {
namespace http {

class date_test : public unit_test::suite
{
public:
    using ptime = posix_time::ptime;
    constexpr static auto rfc1123_fmt = "%a, %d %b %Y %H:%M:%S GMT";

    static
    std::tm
    ptime_to_tm(ptime pt)
    {
        auto tm = boost::gregorian::to_tm(pt.date());
        const auto tod = pt.time_of_day();
        tm.tm_hour = tod.hours();
        tm.tm_min = tod.minutes();
        tm.tm_sec = tod.seconds();
        return tm;
    }

    static
    void
    test_parse(string_view str, ptime const& expected)
    {
        const date_time expected_comp{expected, nullptr};
        BEAST_EXPECT(parse_datetime(str) == expected_comp);
    }

    static
    void
    test_stringify_against_cfmt(ptime pt, size_t len, const char* fmt)
    {
        const auto ours = stringify_datetime(date_time{pt, nullptr});
        std::string s(len, '\0');
        const bool at_res = stringify_datetime_at(date_time{pt, nullptr}, &s[0]);


        if(pt.is_special())
        {
            BEAST_EXPECT(ours.empty());
            BEAST_EXPECT(!at_res);
            BEAST_EXPECT(s == std::string(len, '\0'));
            return;
        }
        const auto tm = ptime_to_tm(pt);
        std::string cstds;
        cstds.resize(len);
        BEAST_EXPECT(std::strftime(&cstds[0], len + 1, fmt, &tm) == len);
        BEAST_EXPECT(ours == cstds);
        BEAST_EXPECT(s == cstds);
    }

    static
    void
    test_parse()
    {
        test_parse("", {});
        test_parse("<!DOCTYPE HTML><html><head><m", {});

        // RFC1123
        test_parse("Sun, 06 Nov 1994 08:49:37 GMT",
                   {{1994, 11, 6}, {8, 49, 37}});
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
                   {{2020, 8, 8}, {19, 6, 22}});
        test_parse("Sunday, 06-Nov-94 08:49:37 GMT",
                   {{1994, 11, 6}, {8, 49, 37}});
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
                   {{1994, 11, 6}, {8, 49, 37}});
        test_parse("Wed Nov 16 08:49:37 1994",
                   {{1994, 11, 16}, {8, 49, 37}});
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
        ptime pt{{2020, 8, 7}, {19, 52, 12}};
        test_stringify_against_cfmt(pt, 29, rfc1123_fmt);
        pt = {{1970, 1, 1}, {0, 0, 0}};
        test_stringify_against_cfmt(pt, 29, rfc1123_fmt);

        pt = {{2010, 5, 3}, boost::date_time::not_a_date_time};
        test_stringify_against_cfmt(pt, 29, rfc1123_fmt);
    }

    void
    run() override
    {
        test_parse();
        test_stringify();
    }
};

BEAST_DEFINE_TESTSUITE(beast,http,date);

}
}
}

