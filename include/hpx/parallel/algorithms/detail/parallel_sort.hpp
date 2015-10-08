//----------------------------------------------------------------------------
/// @file parallel_sort.hpp
/// @brief Parallel Sort algorithm
///
/// @author Copyright (c) 2015 Francisco Jos�� Tapia (fjtapia@gmail.com )\n
///         Distributed under the Boost Software License, Version 1.0.\n
///         ( See accompanyingfile LICENSE_1_0.txt or copy at
///           http://www.boost.org/LICENSE_1_0.txt  )
/// @version 0.1
///
/// @remarks
//-----------------------------------------------------------------------------
#ifndef __HPX_SORT_PARALLEL_ALGORITHM_PARALLEL_SORT_HPP
#define __HPX_SORT_PARALLEL_ALGORITHM_PARALLEL_SORT_HPP

#include <hpx/runtime/threads/thread.hpp>
#include <hpx/parallel/algorithms/detail/token.hpp>

#include <hpx/parallel/algorithms/detail/intro_sort.hpp>
#include <hpx/parallel/algorithms/detail/indirect.hpp>
#include <hpx/include/util.hpp>
#include <hpx/include/lcos.hpp>
//#include <boost/sort/parallel/util/atomic.hpp>

#include <vector>

namespace hpx
{
namespace parallel
{
namespace sort
{
namespace detail
{
namespace bspu = boost::sort::parallel::util;

//
///---------------------------------------------------------------------------
/// @struct parallel_sort_comp
/// @brief implement the parallel sort using the intro_sort algorithm
/// @tparam iter_t : iterators pointing to the elements
/// @tparam compare : objects for to compare two elements pointed by iter_t
///                   iterators
/// @remarks
//----------------------------------------------------------------------------
template < class iter_t,
           typename compare = std::less <typename iter_value<iter_t>::type>  >
struct parallel_sort_comp
{   //------------------------- begin ----------------------
    size_type MaxPerThread = 65536;
    size_t Min_Parallel = 65536;
    compare comp;

    //-------------------------------------------------------------------------
    //                 Variables
    //-------------------------------------------------------------------------
    std::atomic<difference_type > ND ;

    //
    //------------------------------------------------------------------------
    //  function : parallel_sort_comp
    /// @brief constructor
    /// @param [in] first : iterator to the first element to sort
    /// @param [in] last : iterator to the next element after the last
    /// @param [in] NT : variable indicating the number of threads used in the
    ///                  sorting
    /// @exception
    /// @return
    /// @remarks
    //------------------------------------------------------------------------
    parallel_sort_comp (iter_t first, iter_t last )
                        : parallel_sort_comp ( first,last, compare())
                        {} ;
    //
    //------------------------------------------------------------------------
    //  function : parallel_sort_comp
    /// @brief constructor of the struct
    /// @param [in] first : iterator to the first element to sort
    /// @param [in] last : iterator to the next element after the last
    /// @param [in] comp : object for to compare
    /// @param [in] NT : variable indicating the number of threads used in the
    ///                  sorting
    /// @exception
    /// @return
    /// @remarks
    //------------------------------------------------------------------------
    parallel_sort_comp ( iter_t first, iter_t last, compare comp1)
      : comp(comp1)
    {   //------------------------- begin ----------------------
        difference_type N = last - first;
        assert ( N >=0);
        uint32_t Level = (NBits (N)<<1)  ;

        if ( (size_t)N < Min_Parallel )
        {
            intro_sort_internal ( first, last, Level,comp) ;
            return ;
        } ;
        //------------------- check if sort ----------------------------------
        bool SW = true ;
        for ( iter_t it1 = first, it2 = first+1 ;
            it2 != last and (SW = not comp(*it2,*it1));it1 = it2++);
        if (SW) return ;

        //-------------------------------------------------------------------
        std::atomic_store (&ND, N);

        hpx::future<void> dummy = hpx::async(&parallel_sort_comp::sort_thread, this, first, last, Level);
        dummy.get();

    };

    //
    //------------------------------------------------------------------------
    //  function : sort_thread
    /// @brief this function is the work asigned to each thread in the parallel
    ///        process
    /// @exception
    /// @return
    /// @remarks
    //------------------------------------------------------------------------
    hpx::future<void> sort_thread( iter_t first, iter_t last, uint32_t level)
    {
        using hpx::lcos::local::dataflow;

        //------------------------- begin ----------------------
        difference_type N = last - first ;
        if ( (size_type)N <= MaxPerThread or level == 0)
        {
            atomic_sub ( ND, N );
            return hpx::async(
                [this, first, last, level](){
                  intro_sort_internal(first, last, level, comp);
                }
            );
        };

        //----------------------------------------------------------------
        //                     split
        //----------------------------------------------------------------
        typedef typename iter_value<iter_t>::type  value_t ;

        //------------------- check if sort ------------------------------
        bool SW = true ;
        for ( iter_t it1 = first, it2 = first+1 ;
                it2 != last and (SW = not comp(*it2,*it1));it1 = it2++);
        if (SW)
        {
            atomic_sub ( ND, N );
            return hpx::make_ready_future();
        };
        //---------------------- pivot select ----------------------------
        size_t Nx = ( size_t (N ) >>1 ) ;

        iter_t itA = first +1 ;
        iter_t itB = first + Nx ;
        iter_t itC = last -1 ;

        if ( comp( *itB , *itA )) std::swap ( *itA, *itB);
        if ( comp (*itC , *itB))
        {
            std::swap (*itC , *itB );
            if ( comp( *itB , *itA )) std::swap ( *itA, *itB);
        };
        std::swap ( *first , *itB);
        value_t &  val = const_cast < value_t &>(* first);
        iter_t c_first = first+2 , c_last  = last-2;

        while ( c_first != last and comp (*c_first, val)) ++c_first ;
        while ( comp(val ,*c_last ) ) --c_last ;
        while (not( c_first > c_last ))
        {
            std::swap ( *(c_first++), *(c_last--));
            while ( comp(*c_first,val) ) ++c_first;
            while ( comp(val, *c_last) ) --c_last ;
        }; // End while
        std::swap ( *first , *c_last);

        //----------------------------------------------------------------
        //   Store the tokens en ST
        //----------------------------------------------------------------
        auto N1 = (c_last - first);
        auto N2 = (last - first);
        auto N3 = N - N1- N2 ;
        if ( N3 != 0) atomic_sub ( ND, N3 );
        hpx::future<void> hk1 = hpx::async(&parallel_sort_comp::sort_thread, this, first, c_last, level-1);
        hpx::future<void> hk2 = hpx::async(&parallel_sort_comp::sort_thread, this, c_first, last, level-1);
        return dataflow(
        [] (future<void> f1, future<void> f2) -> void
            {
                f1.get();
                f2.get();
                return;
            }, std::move(hk1), std::move(hk2)
        );
     }
};
//
//-----------------------------------------------------------------------------
//  function : parallel_sort
/// @brief function envelope with the comparison object defined by defect
/// @tparam iter_t : iterator used for to access to the elements
/// @param [in] first : iterator to the first element
/// @param [in] last : iterator to the next element to the last valid iterator
/// @param [in] NT : NThread object for to define the number of threads used
///                  in the process. By default is the number of HW threads
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
template    < class iter_t >
void parallel_sort ( iter_t first, iter_t last )
{   parallel_sort_comp <iter_t> ( first, last);
};
//
//-----------------------------------------------------------------------------
//  function : parallel_sort
/// @brief function envelope with the comparison object defined by defect
/// @tparam iter_t : iterator used for to access to the elements
/// @tparam compare : object for to compare the lements pointed by iter_t
/// @param [in] first : iterator to the first element
/// @param [in] last : iterator to the next element to the last valid iterator
/// @param [in] comp : object for to compare
/// @param [in] NT : NThread object for to define the number of threads used
///                  in the process. By default is the number of HW threads
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
template    < class iter_t,
              typename compare = std::less < typename iter_value<iter_t>::type>
            >
void parallel_sort ( iter_t first, iter_t last, compare comp1)
{   parallel_sort_comp<iter_t,compare> ( first, last,comp1);
};


//############################################################################
//                                                                          ##
//                I N D I R E C T     F U N C T I O N S                     ##
//                                                                          ##
//############################################################################
//
//-----------------------------------------------------------------------------
//  function : indirect_parallel_sort
/// @brief indirect parallel sort
/// @tparam iter_t : iterator used for to access to the elements
/// @param [in] first : iterator to the first element
/// @param [in] last : iterator to the next element to the last valid iterator
/// @param [in] NT : NThread object for to define the number of threads used
///                  in the process. By default is the number of HW threads
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
template < class iter_t >
void indirect_parallel_sort ( iter_t first, iter_t last )
{   //------------------------------- begin--------------------------
    typedef std::less <typename iter_value<iter_t>::type> compare ;
    typedef less_ptr_no_null <iter_t, compare>      compare_ptr ;

    std::vector<iter_t> VP ;
    create_index ( first , last , VP);
    parallel_sort  ( VP.begin() , VP.end(), compare_ptr());
    sort_index ( first , VP) ;
};
//
//-----------------------------------------------------------------------------
//  function : indirect_parallel_sort
/// @brief indirect parallel sort
/// @tparam iter_t : iterator used for to access to the elements
/// @tparam compare : object for to compare the lements pointed by iter_t
/// @param [in] first : iterator to the first element
/// @param [in] last : iterator to the next element to the last valid iterator
/// @param [in] comp : object for to compare
/// @param [in] NT : NThread object for to define the number of threads used
///                  in the process. By default is the number of HW threads
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
template < class iter_t,
          typename compare = std::less < typename iter_value<iter_t>::type >
        >
void indirect_parallel_sort ( iter_t first, iter_t last,
                              compare comp1)
{   //----------------------------- begin ----------------------------------
    typedef less_ptr_no_null <iter_t, compare>      compare_ptr ;

    std::vector<iter_t> VP ;
    create_index ( first , last , VP);
    parallel_sort  ( VP.begin() , VP.end(), compare_ptr(comp1) );
    sort_index ( first , VP) ;
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