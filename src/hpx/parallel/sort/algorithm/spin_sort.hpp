//----------------------------------------------------------------------------
/// @file spin_sort.hpp
/// @brief Spin Sort algorithm
///
/// @author Copyright (c) 2010 Francisco José Tapia (fjtapia@gmail.com )\n
///         Distributed under the Boost Software License, Version 1.0.\n
///         ( See accompanyingfile LICENSE_1_0.txt or copy at
///           http://www.boost.org/LICENSE_1_0.txt  )
/// @version 0.1
///
/// @remarks
//-----------------------------------------------------------------------------
#ifndef __SORT_ALGORITHM_SPIN_SORT_HPP
#define __SORT_ALGORITHM_SPIN_SORT_HPP

#include <functional>
#include <memory>
#include <cstdlib>
#include <type_traits>
#include <vector>
#include <hpx/parallel/sort/tools/util_iterator.hpp>
#include <hpx/parallel/sort/tools/algorithm.hpp>
#include <hpx/parallel/sort/algorithm/indirect.hpp>
#include <hpx/parallel/sort/algorithm/insertion_sort.hpp>
#include <hpx/parallel/sort/util/half_merge.hpp>
#include <hpx/parallel/sort/util/full_merge.hpp>

namespace hpx
{
namespace parallel
{
namespace sort
{
namespace algorithm
{
//****************************************************************************
//                 NAMESPACES AND USING SENTENCES
//****************************************************************************
namespace su = sort::util ;
using su::range ;
using tools::NBits ;
using tools::less_ptr_no_null ;

//-----------------------------------------------------------------------------
//  function : range_sort
/// @brief this function divide RA in two parts, sort it ,and merge moving the
///        elements to RB
/// @param [in] RA : range with the elements to sort
/// @param [in] RB : range with the elements sorted
/// @param [in] comp : object for to compare two elements
/// @param [in] level : when is 0, sort with the insertion_sort algorithm
///                     if not make a recursive call swaping the ranges
/// @return range with all the elements sorted and moved
//-----------------------------------------------------------------------------

template <class iter_t  , class iterb_t, class compare>
inline range<iterb_t> range_sort (range<iter_t> RA , range<iterb_t> RB,
                                	compare comp, uint32_t level )
{   //---------------------------- begin ------------------------------------
#if __DEBUG_SORT != 0
	assert ( RA.size() <= RB.size());
#endif
    size_t N1 = (RA.size() +1) >>1;
    RB.last = RB.first + RA.size() ;

    range<iter_t> RA1 ( RA.first, RA.first + N1),RA2(RA.first + N1, RA.last);
    range<iterb_t> RB1 ( RB.first, RB.first + N1),RB2(RB.first + N1, RB.last);

    if ( level == 0)
    {   insertion_sort (RA1.first, RA1.last,comp );
        insertion_sort (RA2.first ,RA2.last, comp );
    }
    else
    {   RA1 = range_sort (RB1, RA1,comp,level-1);
        RA2 = range_sort (RB2, RA2,comp,level-1);
    };
    return full_merge (RB, RA1, RA2,comp );
};

//---------------------------------------------------------------------------
/// @struct spin_sort_tag
/// @brief this is a struct for to do a stable sort exception safe
/// @tparam iter_t : iterator to the elements
/// @tparam compare : object for to compare the elements pointed by iter_t
/// @remarks
//----------------------------------------------------------------------------
template < class iter_t,
           typename compare=std::less<typename iter_value<iter_t>::type >
         >
struct spin_sort_tag
{
//****************************************************************************
//               DEFINITIONS AND CONSTANTS
//****************************************************************************
typedef typename iter_value<iter_t>::type value_t ;
static const uint32_t Sort_min = 36 ;

//****************************************************************************
//                      VARIABLES
//****************************************************************************
value_t *Ptr ;
size_t NPtr ;
bool construct = false , owner = false ;


//-----------------------------------------------------------------------------
//  function : spin_sort_tag
/// @brief constructor of the struct
/// @param [in] R : range of elements to sort
/// @param [in] comp : object for to compare two elements
//-----------------------------------------------------------------------------
spin_sort_tag ( range<iter_t> R , compare comp)
               :Ptr(nullptr),NPtr(0),construct(false),owner(false)
{   //-------------------------- begin -------------------------------------
#if __DEBUG_SORT != 0
	assert ( R.valid());
#endif
    size_t N = R.size();
    owner= construct= false ;

    NPtr = (N +1 ) >>1 ;
    size_t N1 = NPtr ;
	size_t N2 = N - N1 ;
	
    if ( N <= ( Sort_min))
    {   insertion_sort (R.first, R.last, comp);
        return ;
    };
    //------------------- check if sort --------------------------------------
    bool SW = true ;
    for ( iter_t it1 = R.first, it2 = R.first+1 ;
          it2 != R.last and (SW = not comp(*it2,*it1));it1 = it2++);
    if (SW) return ;

    Ptr = std::get_temporary_buffer<value_t>(NPtr).first ;
    if ( Ptr == nullptr) throw std::bad_alloc() ;
    owner = true ;
    range<value_t*> RB ( Ptr , (Ptr + NPtr));
    //------------------------------------------------------------------------
    //                  Process
    //------------------------------------------------------------------------
    uint32_t NLevel = NBits ( (N -1)/Sort_min)-1;

	if ( (NLevel &1) != 0 )
	{	range<iter_t> RA1( R.first , R.first + N2), RA2 ( R.first + N2, R.last) ;
		RB= uninit_move ( RB , RA2) ;
		construct = true ;
		RA2 = range_sort ( RB, RA2, comp , NLevel-1) ;
		RB = range_sort ( RA1, RB , comp,NLevel-1) ;
		R = half_merge ( R, RB, RA2, comp ) ;
	}
	else
	{	range<iter_t> RA1( R.first , R.first + N1), RA2 ( R.first + N1, R.last) ;
		RB = uninit_move ( RB, RA1) ;
		construct = true ;
		RB = range_sort ( RA1, RB , comp, NLevel-1);
		RA1.last = RA1.first + RA2.size() ;
		RA2 = range_sort ( RA1, RA2 , comp, NLevel-1 );
		R = half_merge ( R, RB, RA2, comp ) ;
	};
};

//-----------------------------------------------------------------------------
//  function : spin_sort_tag
/// @brief constructor of the struct
/// @param [in] R : range of elements to sort
/// @param [in] comp : object for to compare two elements
/// @param [in] RB : range used as auxiliary memory
//-----------------------------------------------------------------------------
spin_sort_tag ( range<iter_t> R, compare comp,
                range <value_t *> RB):Ptr(RB.first), NPtr(RB.size()),
                 construct(false),owner(false)
{   //-------------------------- begin -------------------------------------
    size_t N = R.size();
    owner= construct= false ;

    NPtr = (N +1 ) >>1 ;
#if __DEBUG_SORT != 0
    assert ( RB.size() >= NPtr) ;
#endif
    size_t N1 = NPtr;
    size_t N2 = N - N1 ;

    if ( N <= ( Sort_min))
    {   insertion_sort (R.first, R.last, comp);
        return ;
    };
    //------------------------------------------------------------------------
    //                  Process
    //------------------------------------------------------------------------
    uint32_t NLevel = NBits ( (N -1)/Sort_min)-1;

	if ( (NLevel &1) != 0 )
	{	range<iter_t> RA1( R.first , R.first + N2), RA2 ( R.first + N2, R.last) ;
		RB= uninit_move ( RB , RA2) ;
		construct = true ;
		RA2 = range_sort ( RB, RA2, comp , NLevel-1) ;
		RB = range_sort ( RA1, RB , comp,NLevel-1) ;
		R = half_merge ( R, RB, RA2, comp ) ;
	}
	else
	{	range<iter_t> RA1( R.first , R.first + N1), RA2 ( R.first + N1, R.last) ;
		RB = uninit_move ( RB, RA1) ;
		construct = true ;
		RB = range_sort ( RA1, RB , comp, NLevel-1);
		RA1.last = RA1.first + RA2.size() ;
		RA2 = range_sort ( RA1, RA2 , comp, NLevel-1 );
		R = half_merge ( R, RB, RA2, comp ) ;
	};
};

//-----------------------------------------------------------------------------
//  function :~spin_sort_tag
/// @brief destructor of the struct. Deallocate all the data structure used
///        in the sorting
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
~spin_sort_tag ( void)
{   //----------------------------------- begin -------------------------
    if ( construct)
    {   destroy (range<value_t*>( Ptr, Ptr +NPtr));
        construct = false ;
    };
    if ( owner and Ptr != nullptr) std::return_temporary_buffer ( Ptr) ;
};

//
//----------------------------------------------------------------------------
};//        End of class spin_sort_tag
//----------------------------------------------------------------------------

//
//---------------------------------------------------------------------------
//  function : spin_sort
/// @brief Spin Sort algorithm ( stable sort)
/// @param [in] first : first iterator to the elements to sort
/// @param [in] last : last iterator to the elements to sort
/// @param [in] comp : object for to compare two elements
//---------------------------------------------------------------------------
template < class iter_t,
           typename compare = std::less<typename iter_value<iter_t>::type> >
void spin_sort (iter_t first, iter_t last, compare comp = compare() )
{   //----------------------------- begin ----------------------------
	range<iter_t> RA ( first, last);
    spin_sort_tag<iter_t,compare> ( RA , comp);
};

//
//---------------------------------------------------------------------------
//  function : spin_sort
/// @brief Spin Sort algorithm ( stable sort)
/// @param [in] first : first iterator to the elements to sort
/// @param [in] last : last iterator to the elements to sort
/// @param [in] comp : object for to compare two elements
/// @param [in] bfirst : pointer of a data area with at least (N+1)/2 elements
/// @param [in] Nb : capacity of the memory area pointed by bfirst
/// @exception
/// @return
/// @remarks
//---------------------------------------------------------------------------
template < class iter_t,
           typename value_t= typename iter_value<iter_t>::type,
           typename compare = std::less<value_t>  >
void spin_sort ( iter_t first, iter_t last, compare comp,
                 value_t* bfirst, size_t Nb)
{   //------------------------------- begin--------------------------
	typedef typename iter_value<iter_t>::type value2_t ;
    static_assert ( std::is_same<value_t, value2_t>::value,
                   "Incompatible iterators\n");
    //---------------------------------------------------------------
	range<iter_t> RA ( first, last);
	range<value_t*> RB ( bfirst , bfirst + Nb);
    spin_sort_tag<iter_t,compare> ( RA , comp,RB );
};

//############################################################################
//                                                                          ##
//                I N D I R E C T     F U N C T I O N S                     ##
//                                                                          ##
//############################################################################
//
//---------------------------------------------------------------------------
//  function : indirect_smart_merge_sort
/// @brief Indirect sort using the smart_merge_sort algorithm ( stable sort)
/// @tparam iter_t : iterator to the elements
/// @tparam compare : object for to compare the elements pointed by iter_t
/// @param [in] first : first iterator to the elements to sort
/// @param [in] last : last iterator to the elements to sort
/// @param [in] comp : object for to compare two elements
/// @exception
/// @return
/// @remarks
//---------------------------------------------------------------------------
template < class iter_t,
           typename compare = std::less<typename iter_value<iter_t>::type>  >
void indirect_spin_sort ( iter_t first, iter_t last, compare comp = compare() )
{   //------------------------------- begin-----------------------------------
    typedef less_ptr_no_null <iter_t, compare>      compare_ptr ;
    
    std::vector<iter_t> VP ;
    create_index ( first , last , VP);
    spin_sort  ( VP.begin() , VP.end(), compare_ptr(comp) );
    sort_index ( first , VP) ;
};
//
//****************************************************************************
};//    End namespace algorithm
};//    End namespace sort
};//    End namespace parallel
};//    End namespace hpx
//****************************************************************************
//
#endif
