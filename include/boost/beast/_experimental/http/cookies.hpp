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
#include <boost/beast/_experimental/http/date.hpp>
#include <boost/beast/core/string.hpp>
#include <boost/beast/http/detail/basic_parsed_list.hpp>
#include <boost/range/adaptor/filtered.hpp>

namespace boost {
namespace beast {
namespace http {

/** The type used to represent cookies present in HTTP requests
    when observing them
*/
class request_cookie_view : detail::request_cookie_view_base {
    friend detail::request_cookie_list_policy;

public:
    /// Returns the name of the cookie
    string_view name() const noexcept { return this->name_; }
    /// Returns the value of the cookie
    string_view value() const noexcept { return this->value_; }
};

/** The type used to represent cookies present in HTTP requests
    with ownership of the values
*/
struct request_cookie {
    std::string name;
    std::string value;

    request_cookie() noexcept = default;
    request_cookie(const request_cookie_view& v)
            : name{v.name()}, value{v.value()}
    {
    }
};



struct response_cookie {
    std::string name;
    std::string value;
    date_time expires;

    //  expires-av / max-age-av / domain-av / path-av / secure-av / httponly-av / extension-av

};


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
using cookie_list = detail::basic_parsed_list<detail::request_cookie_list_policy>;


template<class FieldsAllocator>
using request_cookie_list =
    detail::flat_range<boost::transformed_range<
        detail::fields_transformer<detail::request_cookie_list_policy>,
        boost::filtered_range<
            detail::fields_filter<field::cookie>,
            const http::basic_fields<FieldsAllocator>>>>;

/** Produces a list of all the cookies in fields
*/

template<class FieldsAllocator>
auto list_all_cookies(http::basic_fields<FieldsAllocator> const& fields)
    -> decltype(fields
            | boost::adaptors::filtered(detail::fields_filter<field::cookie>{})
            | boost::adaptors::transformed(
              detail::fields_transformer<detail::request_cookie_list_policy>{})
            | detail::flattened) {
    return fields
        | boost::adaptors::filtered(detail::fields_filter<field::cookie>{})
        | boost::adaptors::transformed(
              detail::fields_transformer<detail::request_cookie_list_policy>{})
        | detail::flattened;
}


class cookie_jar {

};

} // http
} // beast
} // boost

#endif