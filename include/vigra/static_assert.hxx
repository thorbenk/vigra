/************************************************************************/
/*                                                                      */
/*               Copyright 2004-2005 by Ullrich Koethe                  */
/*       Cognitive Systems Group, University of Hamburg, Germany        */
/*                                                                      */
/*    This file is part of the VIGRA computer vision library.           */
/*    You may use, modify, and distribute this software according       */
/*    to the terms stated in the LICENSE file included in               */
/*    the VIGRA distribution.                                           */
/*                                                                      */
/*    The VIGRA Website is                                              */
/*        http://kogs-www.informatik.uni-hamburg.de/~koethe/vigra/      */
/*    Please direct questions, bug reports, and contributions to        */
/*        koethe@informatik.uni-hamburg.de                              */
/*                                                                      */
/*  THIS SOFTWARE IS PROVIDED AS IS AND WITHOUT ANY EXPRESS OR          */
/*  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. */
/*                                                                      */
/************************************************************************/

#ifndef VIGRA_STATIC_ASSERT_HXX
#define VIGRA_STATIC_ASSERT_HXX

// based on the static assertion design in boost::mpl (see www.boost.org)

#define VIGRA_PREPROCESSOR_CONCATENATE(a, b) VIGRA_PREPROCESSOR_CONCATENATE_IMPL(a, b)
#define VIGRA_PREPROCESSOR_CONCATENATE_IMPL(a, b) a ## b

namespace vigra {

namespace staticAssert {

template <bool Predicate>
struct AssertBool;

template <>
struct AssertBool<true>
{
    typedef int type;
    typedef void * not_type;
};

template <>
struct AssertBool<false>
{
    typedef void * type;
    typedef int not_type;
};

struct failure{};
struct success {};
inline int check( success ) { return 0; }

template< typename Predicate >
failure ************ (Predicate::************ 
      assertImpl( void (*)(Predicate), typename Predicate::not_type )
    );

template< typename Predicate >
success
assertImpl( void (*)(Predicate), typename Predicate::type );

/* Usage:

1. Define an assertion class, derived from vigra::staticAssert::Assert,
   whose name serves as an error message:
   
    template <int N>
    struct FixedPoint_overflow_error__More_than_31_bits_requested
    : vigra::staticAssert::AssertBool<(N < 32)>
    {};

2. Call VIGRA_STATIC_ASSERT() with the assertion class:

    template <int N>
    void test()
    {
        // signal error if N > 31
        VIGRA_STATIC_ASSERT((FixedPoint_overflow_error__More_than_31_bits_requested<N>));
    }
    
TODO: provide more assertion base classes for other (non boolean) types of tests
*/
#define VIGRA_STATIC_ASSERT(Predicate) \
enum { \
    VIGRA_PREPROCESSOR_CONCATENATE(vigra_assertion_in_line_, __LINE__) = sizeof( \
         staticAssert::check( \
              staticAssert::assertImpl( (void (*) Predicate)0, 1 ) \
            ) \
        ) \
}

} // namespace staticAssert

} // namespace vigra

#endif // VIGRA_STATIC_ASSERT_HXX