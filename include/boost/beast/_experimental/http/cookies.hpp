//
// Copyright (c) 2016-2020 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_HTTP_COOKIES_HPP
#define BOOST_BEAST_HTTP_COOKIES_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/_experimental/http/detail/cookies.hpp>
#include <boost/beast/http/detail/basic_parsed_list.hpp>

namespace boost {
namespace beast {
namespace http {

/** A list of cookies in a semicolon separated HTTP field value.

    This container allows iteration of a list of cookies in a
    header field value. The input is a semicolon separated list of
    cookie-pairs.

    If a parsing error is encountered while iterating the string,
    the behavior of the container will be as if a string containing
    only characters up to but excluding the first invalid character
    was used to construct the list.

    Each item of the list is a pair of the cookie's name and value

    @par BNF
    @code
        cookie-list  = cookie-pair *( ";" SP cookie-pair )
        cookie-pair  = cookie-name "=" cookie-value
        cookie-name  = token
        cookie-value = *cookie-octet / ( DQUOTE *cookie-octet DQUOTE )
        cookie-octet = %x21 / %x23-2B / %x2D-3A / %x3C-5B / %x5D-7E
    @endcode

    To use this class, construct with the string to be parsed and
    then use `begin` and `end`, or range-for to iterate each item:

    @par Example
    @code
    for(auto [name, value] : cookie_list{"fruits=\"pear banana\"; choice=1"})
        std::cout << name << ' ' << value << "\n";
    @endcode
*/
using cookie_list = detail::basic_parsed_list<detail::cookie_list_policy>;

using setcookie_list = detail::basic_parsed_list<detail::setcookie_list_policy>;

class cookie_jar {

};

} // http
} // beast
} // boost

#endif