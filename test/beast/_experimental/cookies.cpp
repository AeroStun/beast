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

    template<class Policy>
    static
    std::vector<std::pair<std::string, std::string>>
    to_vector(string_view in)
    {
        std::vector<std::pair<std::string, std::string>> v;
        detail::basic_parsed_list<Policy> list{in};
        for(auto const& s :
                detail::basic_parsed_list<Policy>{in})
            v.emplace_back(std::string{s.first.data(), s.first.size()},
                           std::string{s.second.data(), s.second.size()});
        return v;
    }

    template<class Policy>
    void
    validate(string_view in,
             std::vector<std::pair<std::string, std::string>> const& v)
    {
        BEAST_EXPECT(to_vector<Policy>(in) == v);
    }

    template<class List>
    void
    good(string_view in,
         std::vector<std::pair<std::string, std::string>> const& v)
    {
        BEAST_EXPECT(validate_list(
            List{in}));
        validate<typename List::policy_type>(in, v);
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
    run() override
    {
        testCookieList();
    }
};

BEAST_DEFINE_TESTSUITE(beast,http,cookies);

} // http
} // beast
} // boost