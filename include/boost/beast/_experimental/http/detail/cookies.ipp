//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_DETAIL_COOKIES_IPP
#define BOOST_BEAST_DETAIL_COOKIES_IPP

#include <boost/beast/core/string.hpp>
#include <boost/beast/_experimental/http/detail/cookies.hpp>
#include <boost/beast/http/detail/rfc7230.hpp>

namespace boost {
namespace beast {
namespace http {

namespace detail {

char
is_cookie_octet(char c) noexcept
{
    /*
        cookie-octet = %x21 / %x23-2B / %x2D-3A / %x3C-5B / %x5D-7E
    */
    static char constexpr tab[] = {
        0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 0
        0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 16
        0, 1, 0, 1,  1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1, // 32
        1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 0,  1, 1, 1, 1, // 48
        1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, // 64
        1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1, // 80
        1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, // 96
        1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 0, // 112
        0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 128
        0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 144
        0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 160
        0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 176
        0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 192
        0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 208
        0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 224
        0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0  // 240
    };
    BOOST_STATIC_ASSERT(sizeof(tab) == 256);
    return tab[static_cast<unsigned char>(c)];
}

bool
cookie_list_policy::operator()(value_type& v,
                                  char const*& it, string_view s) const
{
    const auto s_end = s.data() + s.size();
    v = {};

    if(it == s_end)
    {
        it = nullptr;
        return true;
    }

    const auto need_semispace = it != s.data();
    if(need_semispace)
    {
          constexpr size_t min_len = sizeof("; a=b");
          if(static_cast<size_t>(s_end - it) < min_len
              || *it++ != ';'
              || *it++ != ' ')
              return false;
    }

    const auto name_begin = it;
    if(!detail::is_token_char(*it++))
        return false;

    for(;; ++it)
    {
        if (it == s_end)
            return false;
        if(detail::is_token_char(*it))
            continue;
        if(*it == '=')
            break;

        return false;
    }
    v.first = {name_begin,
               static_cast<std::size_t>(it - name_begin)};

    if(++it == s_end)
    {
        v.second = {nullptr, 0};
        return true;
    }

    const bool quoted = *it == '"';
    if(quoted && ++it == s_end)
        return false;

    const auto value_begin = it;

    for(;; ++it)
    {
        if(it == s_end || *it == ';') {
            if(quoted)
                return false;
            break;
        }
        if(detail::is_cookie_octet(*it))
            continue;
        if(quoted && *it == '"')
            break;
        return false;
    }

    v.second = {value_begin,
               static_cast<std::size_t>(it - value_begin)};

    if(quoted)
        ++it;

    return true;
}

} // detail

} // http
} // beast
} // boost

#endif