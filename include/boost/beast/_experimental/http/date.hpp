//
// Copyright (c) 2016-2020 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_HTTP_DATE_HPP
#define BOOST_BEAST_HTTP_DATE_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/string.hpp>
#include <boost/date_time/local_time/local_date_time.hpp>
#include <string>

namespace boost {
namespace beast {
namespace http {

/* Note: HTTP time is defined by RFC2616 at section 3.3.1
*/

/** The datatype which represents an HTTP date-time
*/
using date_time = boost::local_time::local_date_time;

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
BOOST_BEAST_DECL
date_time
parse_datetime(string_view http_date_str) noexcept;

/** Turns a date_time object into an RFC1123 date string

    Preconditions:
    - the datetime has GMT time, since HTTP time must be GMT.
      If this precondition is not met, the result will be incorrect
    - there are 29 (or more) bytes available at storage
      If this precondition is not met, the behavior is undefined

    Postconditions:
    - on valid input, only the first 29 bytes at storage have been written to
    - on error, no bytes at storage have been written to

    @param d A date_time structure
    @param storage A pointer to at least 29 bytes of valid `char` storage

*/
BOOST_BEAST_DECL
bool
stringify_datetime_at(date_time const& dt, char* storage) noexcept;

/** Turns a date_time object into an RFC1123 date string

    This function assumes the date_time has GMT time, since HTTP time
    must be GMT.

    @return a 29 characters long string containing the date, or empty on error
*/
BOOST_BEAST_DECL
std::string
stringify_datetime(date_time const& dt);


} // http
} // beast
} // boost

#ifdef BOOST_BEAST_HEADER_ONLY
#include <boost/beast/_experimental/http/impl/date.ipp>
#endif

#endif