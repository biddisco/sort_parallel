//----------------------------------------------------------------------------
/// @file insertion_sort.hpp
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
#ifndef __HPX_PARALLEL_SORT_DETAIL_INSERTION_SORT_HPP
#define __HPX_PARALLEL_SORT_DETAIL_INSERTION_SORT_HPP

#include <iterator>
#include <functional>
#include <utility> // std::swap
#include <hpx/parallel/sort/detail/util/compare_traits.hpp>

namespace hpx		{
namespace parallel	{
namespace sort		{
namespace detail	{

//-----------------------------------------------------------------------------
//  function : insertion_sort
/// @brief : Insertion sort algorithm
/// @param [in] first: iterator to the first element of the range
/// @param [in] last : iterator to the next element of the last in the range
/// @param [in] comp : object for to do the comparison between the elements
/// @remarks This algorithm is O(N²)
//-----------------------------------------------------------------------------
template < class Iter_t, typename Compare = util::compare_iter<Iter_t> >
void insertion_sort (Iter_t first, Iter_t last, Compare comp = Compare() )
{   //------------------------- begin ----------------------
    typedef typename std::iterator_traits<Iter_t>::value_type value_t ;

    if ( (last-first) < 2 ) return ;
    for ( Iter_t alfa = first +1 ;alfa != last ; ++alfa)
    {   value_t aux = std::move ( *alfa);
        Iter_t beta  = alfa ;

        while( beta != first and comp ( aux, *(beta-1) ) )
        {   *beta = std::move ( *(beta-1));
            --beta ;
        };
        *beta = std::move ( aux );
    };
};
//
//****************************************************************************
};//    End namespace detail
};//    End namespace sort
};//    End namespace parallel
};//    End namespace hpx
//****************************************************************************
//
#endif