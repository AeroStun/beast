//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_DETAIL_COOKIES_HPP
#define BOOST_BEAST_DETAIL_COOKIES_HPP

#include <boost/beast/core/string.hpp>
#include <boost/beast/http/detail/rfc7230.hpp>
#include <utility>

namespace boost {
namespace beast {
namespace http {

namespace detail {

BOOST_BEAST_DECL
char
is_cookie_octet(char c) noexcept;

/*
    cookie-list  = cookie-pair *( ";" SP cookie-pair )
    cookie-pair  = cookie-name "=" cookie-value
    cookie-name  = token
    cookie-value =
*/
struct cookie_list_policy {
    using value_type = std::pair<string_view, string_view>;

    BOOST_BEAST_DECL
    bool
    operator()(value_type& v,
             char const*& it, string_view s) const;
};

struct setcookie_list_policy {

};

} // detail

} // http
} // beast
} // boost

#include <boost/beast/_experimental/http/detail/cookies.ipp>

#endif