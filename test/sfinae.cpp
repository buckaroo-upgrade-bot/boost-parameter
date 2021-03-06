// Copyright David Abrahams, Daniel Wallin 2003.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/parameter/config.hpp>

#if !defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING) && \
    (BOOST_PARAMETER_MAX_ARITY < 2)
#error Define BOOST_PARAMETER_MAX_ARITY as 2 or greater.
#endif

#include <boost/parameter.hpp>

namespace test {

    BOOST_PARAMETER_NAME((name, keywords) in(name))
    BOOST_PARAMETER_NAME((value, keywords) in(value))
} // namespace test

#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_convertible.hpp>

namespace test {

    template <typename To>
    struct f_predicate
    {
        template <typename From, typename Args>
        struct apply
          : boost::mpl::if_<
                boost::is_convertible<From,To>
              , boost::mpl::true_
              , boost::mpl::false_
            >
        {
        };
    };
} // namespace test

#include <string>

namespace test {

    struct f_parameters
      : boost::parameter::parameters<
            boost::parameter::optional<
                test::keywords::name
              , test::f_predicate<std::string>
            >
          , boost::parameter::optional<
                test::keywords::value
              , test::f_predicate<float>
            >
        >
    {
    };
} // namespace test

#include <boost/core/lightweight_test.hpp>

namespace test {

    // The use of assert_equal_string is just a nasty workaround for a
    // vc++ 6 ICE.
    void assert_equal_string(std::string x, std::string y)
    {
        BOOST_TEST(x == y);
    }

    template <typename P>
    void f_impl(P const& p)
    {
        float v = p[test::value | 3.f];
        BOOST_TEST_EQ(v, 3.f);
        test::assert_equal_string(p[test::name | "bar"], "foo");
    }

    void f()
    {
        test::f_impl(f_parameters()());
    }
} // namespace test

#include <boost/parameter/match.hpp>

namespace test {

    template <typename A0>
    void
        f(
            A0 const& a0
          , BOOST_PARAMETER_MATCH(f_parameters, (A0), args)
        )
    {
        test::f_impl(args(a0));
    }

    template <typename A0, typename A1>
    void
        f(
            A0 const& a0
          , A1 const& a1
          , BOOST_PARAMETER_MATCH(f_parameters, (A0)(A1), args)
        )
    {
        test::f_impl(args(a0, a1));
    }
} // namespace test

#if !defined(BOOST_NO_SFINAE) && \
    !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x592))

#include <boost/core/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>

namespace test {

    // On compilers that actually support SFINAE, add another overload that is
    // an equally good match and can only be in the overload set when the
    // others are not.  This tests that the SFINAE is actually working.  On
    // all other compilers we're just checking that everything about
    // SFINAE-enabled code will work, except of course the SFINAE.
    template <typename A0, typename A1>
    typename boost::enable_if<
        typename boost::mpl::if_<
            boost::is_same<int,A0>
          , boost::mpl::true_
          , boost::mpl::false_
        >::type
      , int
    >::type
        f(A0 const& a0, A1 const& a1)
    {
        return 0;
    }
} // namespace test

#endif  // SFINAE enabled, no Borland workarounds needed.

int main()
{
    test::f("foo");
    test::f("foo", 3.f);
    test::f(test::value = 3.f, test::name = "foo");
#if !defined(BOOST_NO_SFINAE) && \
    !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x592))
    BOOST_TEST_EQ(0, test::f(3, 4));
#endif
    return boost::report_errors();
}

