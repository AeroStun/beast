//
// Copyright (c) 2016-2020 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

// Test that header file is self-contained.
#include <boost/beast/_experimental/http/cookies.hpp>

#include <boost/beast/_experimental/http/detail/cookies.hpp>
#include <boost/beast/http/rfc7230.hpp>
#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <string>
#include <vector>

namespace boost {
namespace beast {
namespace http {

class cookies_test : public beast::unit_test::suite
{
public:

    template<class List, class In>
    static
    std::vector<std::pair<std::string, std::string>>
    to_vector(const In& in)
    {
        std::vector<std::pair<std::string, std::string>> v;
        List list{in};
        for(auto const& s :
                List{in})
            v.emplace_back(std::string{s.name().data(), s.name().size()},
                           std::string{s.value().data(), s.value().size()});
        return v;
    }

    template<class In, class Fn>
    static
    std::vector<std::pair<std::string, std::string>>
    to_vector_fn(const In& in, Fn&& fn)
    {
      std::vector<std::pair<std::string, std::string>> v;
      for(auto const& s :
          fn(in))
        v.emplace_back(std::string{s.name().data(), s.name().size()},
                       std::string{s.value().data(), s.value().size()});
      return v;
    }

    template<class List>
    void
    validate(string_view in,
             std::vector<std::pair<std::string, std::string>> const& v)
    {
        BEAST_EXPECT(to_vector<List>(in) == v);
    }

    template<class List>
    void
    good(string_view in,
         std::vector<std::pair<std::string, std::string>> const& v)
    {
        BEAST_EXPECT(validate_list(
            List{in}));
        validate<List>(in, v);
    }

    template<class List>
    void
    bad(string_view in)
    {
        BEAST_EXPECT(! validate_list(
            List{in}));
    }

    void
    testCookieList()
    {
        using list = cookie_list;
        good<list>("", {});
        good<list>("foo=", {{"foo", ""}});
        good<list>("foo=''", {{"foo", "''"}});
        good<list>("foo=\"\"", {{"foo", ""}});
        good<list>("foo=bar", {{"foo", "bar"}});
        good<list>("foo=\"bar\"", {{"foo", "bar"}});
        good<list>("foo=; bar=", {{"foo", ""}, {"bar", ""}});
        good<list>("foo=bar; baz=", {{"foo", "bar"}, {"baz", ""}});
        bad<list>("foo");
        bad<list>("foo ");
        bad<list>("=foo");
        bad<list>("foo=\"");
        bad<list>("foo=\";");
        bad<list>("foo=;bar=");
        bad<list>("foo=, bar=");
    }

    void
    testRequestCookieList()
    {
        http::fields f;

        const auto check =
            [&](std::vector<std::pair<std::string, std::string>> v)
        {
            BEAST_EXPECT(to_vector_fn(f,
                           list_all_cookies<decltype(f)::allocator_type>) == v);
        };

        check({});
        f.insert(field::referer, "foo.bar");
        check({});
        f.insert(field::cookie, "foo=bar");
        check({{"foo", "bar"}});
    }

    void
    run() override
    {
        testCookieList();
        //testRequestCookieList();
    }
};

BEAST_DEFINE_TESTSUITE(beast,http,cookies);

} // http
} // beast
} // boost