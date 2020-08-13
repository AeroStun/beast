//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_HTTP_DETAIL_COOKIES_HPP
#define BOOST_BEAST_HTTP_DETAIL_COOKIES_HPP

#include <boost/beast/core/string.hpp>
#include <boost/beast/http/detail/rfc7230.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/iterator_range.hpp>
#include <utility>

namespace boost {
namespace beast {
namespace http {

class request_cookie_view;

namespace detail {

/*  cookie-octet = %x21 / %x23-2B / %x2D-3A / %x3C-5B / %x5D-7E
*/
BOOST_BEAST_DECL
char
is_cookie_octet(char c) noexcept;

struct request_cookie_view_base {
    string_view name_;
    string_view value_;
};

/*
    Note: this policy allows empty lists, which the below spec does not allow
    Users of this policy should ensure that their list's .begin() != .end()

    cookie-list  = cookie-pair *( ";" SP cookie-pair )
    cookie-pair  = cookie-name "=" cookie-value
    cookie-name  = token
    cookie-value = *cookie-octet / ( DQUOTE *cookie-octet DQUOTE )
*/
struct request_cookie_list_policy {
    using value_type = request_cookie_view;

    BOOST_BEAST_DECL
    bool
    operator()(request_cookie_view& v,
             char const*& it, string_view s) const;
};

/*
    Note: this is not part of the spec (RFC6265), but helps
    implementing the spec, as well as users implementing
    support for custom cookies attributes they might have

    cookie-av-list  = [ cookie-av *( ";" SP cookie-av ) ]
    cookie-av       = token [ "=" cookie-av-value ]
    cookie-av-value = *<any CHAR except CTLs or ";">
 */
struct response_cookie_attribute_list_policy {

};

template<field Field>
struct fields_filter
{
    template<class FieldsElement>
    bool operator()(FieldsElement const& v) const
    {
        return v.name() == Field;
    }
};

template<class Policy>
struct fields_transformer
{
    template<class FieldsElement>
    basic_parsed_list<Policy> operator()(FieldsElement const& v) const
    {
        return basic_parsed_list<Policy>(v.value());
    }
};

#pragma region flattening_iterator // TODO YEET

template <typename OuterIterator> class flattening_iterator
{
public:
  using outer_iterator = OuterIterator;
  using inner_iterator = typename OuterIterator::value_type::iterator;

  using iterator_category = std::forward_iterator_tag;
  using value_type = typename inner_iterator::value_type;
  using difference_type = typename inner_iterator::difference_type;
  using pointer = typename inner_iterator::pointer;
  using reference = typename inner_iterator::reference;

  flattening_iterator()
  {
  }
  flattening_iterator(outer_iterator it)
      : outer_it_(it)
      , outer_end_(it)
  {
  }
  flattening_iterator(outer_iterator it, outer_iterator end)
      : outer_it_(it)
      , outer_end_(end)
  {
    if (outer_it_ == outer_end_)
    {
      return;
    }

    inner_it_ = outer_it_->begin();
    advance_past_empty_inner_containers();
  }

  reference operator*() const
  {
    return *inner_it_;
  }
  pointer operator->() const
  {
    return &*inner_it_;
  }

  flattening_iterator& operator++()
  {
    ++inner_it_;
    if (inner_it_ == outer_it_->end())
      advance_past_empty_inner_containers();
    return *this;
  }

  flattening_iterator operator++(int)
  {
    flattening_iterator it(*this);
    ++*this;
    return it;
  }

  friend bool operator==(const flattening_iterator& a, const flattening_iterator& b)
  {
    if (a.outer_it_ != b.outer_it_)
      return false;

    if (a.outer_it_ != a.outer_end_ && b.outer_it_ != b.outer_end_ && a.inner_it_ != b.inner_it_)
      return false;

    return true;
  }

  friend bool operator!=(const flattening_iterator& a, const flattening_iterator& b)
  {
    return !(a == b);
  }

private:
  void advance_past_empty_inner_containers()
  {
    while (outer_it_ != outer_end_ && inner_it_ == outer_it_->end())
    {
      ++outer_it_;
      if (outer_it_ != outer_end_)
        inner_it_ = outer_it_->begin();
    }
  }

  outer_iterator outer_it_;
  outer_iterator outer_end_;
  inner_iterator inner_it_;
};

template <typename Iterator> flattening_iterator<Iterator> flatten(Iterator it)
{
    return flattening_iterator<Iterator>(it, it);
}

template <typename Iterator> flattening_iterator<Iterator> flatten(Iterator first, Iterator last)
{
    return flattening_iterator<Iterator>(first, last);
}

template <typename R>
struct flat_range : boost::iterator_range<flattening_iterator<typename boost::range_iterator<R>::type>>
{
public:
    using iterator = flattening_iterator<typename boost::range_iterator<R>::type>;

private:
    using base = boost::iterator_range<iterator>;

public:
    flat_range(R& r)
        : base(iterator(boost::begin(r), boost::end(r)), iterator(boost::end(r)))
    {
    }
};

namespace impl
{
struct flat_forwarder
{
};
}

template <class R> inline flat_range<R> operator|(R&& r, impl::flat_forwarder)
{
    BOOST_RANGE_CONCEPT_ASSERT((boost::SinglePassRangeConcept<R>));

    return flat_range<R>(r);
}

template <class R> inline flat_range<const R> operator|(const R& r, impl::flat_forwarder)
{
    BOOST_RANGE_CONCEPT_ASSERT((boost::SinglePassRangeConcept<const R>));

    return flat_range<const R>(r);
}

namespace
{
const impl::flat_forwarder flattened = impl::flat_forwarder();
}

#pragma endregion


} // detail

} // http
} // beast
} // boost

#ifdef BOOST_BEAST_HEADER_ONLY
#include <boost/beast/_experimental/http/detail/cookies.ipp>
#endif

#endif