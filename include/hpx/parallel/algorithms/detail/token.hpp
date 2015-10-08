//----------------------------------------------------------------------------
/// @file token.hpp
/// @brief Insertion Sort algorithm
///
/// @author Copyright (c) 2010 2015 Francisco José Tapia (fjtapia@gmail.com )\n
///         Distributed under the Boost Software License, Version 1.0.\n
///         ( See accompanyingfile LICENSE_1_0.txt or copy at
///           http://www.boost.org/LICENSE_1_0.txt  )
/// @version 0.1
///
/// @remarks
//-----------------------------------------------------------------------------
#ifndef __HPX_SORT_PARALLEL_ALGORITHM_TOKEN_HPP
#define __HPX_SORT_PARALLEL_ALGORITHM_TOKEN_HPP

#include <boost/type_traits.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/type_traits/is_union.hpp>

namespace hpx
{
namespace parallel
{
namespace sort
{
namespace detail
{
//
///---------------------------------------------------------------------------
/// @struct token
/// @brief pair of iterators. This struct is used by several sort algorithms
/// @tparam iter_t : iterator to the elements to sort
/// @remarks
//----------------------------------------------------------------------------
template <class iter_t>
struct token
{   //----------- variables -------------
    iter_t first ;
    iter_t last ;
    //------------------- Functions -------------------
    token( void){} ;
    token ( iter_t it_f , iter_t it_l):first ( it_f), last ( it_l){};
};
//
///---------------------------------------------------------------------------
/// @struct token
/// @brief pair of iterators. This struct is used by several sort algorithms
/// @tparam iter_t : iterator to the elements to sort
/// @remarks
//----------------------------------------------------------------------------
template <class iter_t>
struct token_level
{   //----------- variables -------------
    iter_t first ;
    iter_t last ;
    int32_t level ;
    //------------------- Functions -------------------
    token_level( void) {} ;
    token_level ( iter_t it_f, iter_t it_l, int32_t lv)
                 :first (it_f),last(it_l),level(lv) {  };
};
//
//****************************************************************************
};//    End namespace detail
};//    End namespace parallel
};//    End namespace sort
};//    End namespace boost
//****************************************************************************
namespace boost {
  template <typename iter_t>
  struct has_trivial_destructor< hpx::parallel::sort::detail::token_level<iter_t> > : public true_type {};
  template <typename iter_t> 
  struct has_trivial_assign< hpx::parallel::sort::detail::token_level<iter_t> > : public true_type{};
};
//
#endif
