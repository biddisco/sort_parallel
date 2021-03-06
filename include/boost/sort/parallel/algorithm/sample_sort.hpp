//----------------------------------------------------------------------------
/// @file sample_sort.hpp
/// @brief Sample Sort algorithm
///
/// @author Copyright (c) 2010 Francisco José Tapia (fjtapia@gmail.com )\n
///         Distributed under the Boost Software License, Version 1.0.\n
///         ( See accompanyingfile LICENSE_1_0.txt or copy at
///           http://www.boost.org/LICENSE_1_0.txt  )
/// @version 0.1
///
/// @remarks
//-----------------------------------------------------------------------------
#ifndef __BOOST_SORT_GENERAL_ALGORITHM_SAMPLE_SORT_HPP
#define __BOOST_SORT_GENERAL_ALGORITHM_SAMPLE_SORT_HPP

#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include <thread>
#include <algorithm>
#include <boost/sort/parallel/util/atomic.hpp>
#include <boost/sort/parallel/util/nthread.hpp>
#include <boost/sort/parallel/util/util_iterator.hpp>
#include <boost/sort/parallel/util/algorithm.hpp>
#include <boost/sort/parallel/algorithm/smart_merge_sort.hpp>
#include <boost/sort/parallel/util/range.hpp>
#include <boost/sort/parallel/algorithm/indirect.hpp>

namespace boost
{
namespace sort
{
namespace parallel
{
namespace algorithm
{
namespace bspu = boost::sort::parallel::util;
using bspu::NThread ;
using bspu::NThread_HW ;
//
///---------------------------------------------------------------------------
/// @struct sample_sort_tag
/// @brief This a structure for to implement a sample sort, exception
///        safe
/// @tparam
/// @remarks
//----------------------------------------------------------------------------
template < class iter_t,
           typename compare=std::less<typename iter_value<iter_t>::type >
         >
struct sample_sort_tag
{
//------------------------------------------------------------------------
//                     DEFINITIONS
//------------------------------------------------------------------------
typedef typename iter_value<iter_t>::type   value_t ;
typedef range <iter_t>                      range_it ;
typedef range <value_t*>                    range_buf ;

//------------------------------------------------------------------------
//                VARIABLES AND CONSTANTS
//------------------------------------------------------------------------
value_t *Ptr ;
size_t NElem ;
uint32_t NThr, Ninterval ;
static const uint32_t Thread_min = (1<<12) ;
bool construct = false, owner = false  ;
compare comp ;
range_it global_range ;


std::vector <std::vector <range<iter_t  > > > VMem ;
std::vector <std::vector <range<value_t*> > > VBuf ;
std::vector <iter_t>                          VMIni ;
std::vector <value_t*>                        VBIni ;
std::atomic<uint32_t> NJobs ;


//----------------------------------------------------------------------------
//                       FUNCTIONS OF THE STRUCT
//----------------------------------------------------------------------------
void initial_configuration ( void);

//-----------------------------------------------------------------------------
//                 CONSTRUCTOR AND DESTRUCTOR
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
//  function : sample_sort_tag
/// @brief constructor of the class
/// @param [in] first : iterator to the first element of the range
/// @param [in] last : iterator to next element after the last of the range
/// @param [in] comp : object for to compare two elements
/// @param [in] NT : NThread object for to define the number of threads to use
///                  in the process. By default is the number of thread HW
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
sample_sort_tag ( iter_t first, iter_t last, compare cmp, const NThread & NT)
                 :Ptr(nullptr),NElem(0),owner(false),comp( cmp),
                  global_range(first, last)
{   //-------------------------- begin -------------------------------------
    auto N = last- first ;
    assert ( N >=0);
    NElem = size_t ( N );
    construct= false ;
    NThr = NT() ;
    Ninterval = ( NThr <<4);
    NJobs = 0 ;

    if ( NT() <2 or NElem <= ( Thread_min))
    {   smart_merge_sort (first, last, comp);
        return ;
    };

    //------------------- check if sort --------------------------------------
    bool SW = true ;
    for ( iter_t it1 = first, it2 = first+1 ;
          it2 != last and (SW = not comp(*it2,*it1));it1 = it2++);
    if (SW) return ;

    Ptr = std::get_temporary_buffer<value_t>(NElem).first ;
    if ( Ptr == nullptr) throw std::bad_alloc() ;
    owner = true ;

    //------------------------------------------------------------------------
    //                    PROCESS
    //------------------------------------------------------------------------
    initial_configuration () ;

    first_merge ( );
    construct = true ;
    final_merge ( );

};
//
//-----------------------------------------------------------------------------
//  function : sample_sort_tag
/// @brief constructor of the class
/// @param [in] first : iterator to the first element of the range
/// @param [in] last : iterator to next element after the last of the range
/// @param [in] comp : object for to compare two elements
/// @param [in] NT : NThread object for to define the number of threads to use
///                  in the process. By default is the number of thread HW
/// @param [in] P1 : pointer to the auxiliary memory used in the sorting
/// @param [in] NP1 : capacity in number of objects  of the auxiliary memory
///                   pointed by P1
/// @exception
/// @return
/// @remarks This function is for internal use only
//-----------------------------------------------------------------------------
sample_sort_tag ( iter_t first, iter_t last, compare cmp, const NThread &NT,
                  value_t *P1, size_t NP1)
                 :Ptr(P1),NElem(0),comp( cmp),global_range(first, last)
{   //-------------------------- begin -------------------------------------
    auto N = last - first ;
    assert ( N >=0);
    NElem = size_t ( N );
    assert ( NP1 >= NElem and Ptr != nullptr);
    construct= false ;
    NThr = NT() ;
    Ninterval = ( NThr <<3);
    NJobs = 0 ;

    if ( NT() <2 or NElem <= ( Thread_min))
    {   smart_merge_sort (first, last, comp, Ptr, NP1);
        return ;
    };

    //------------------- check if sort --------------------------------------
    bool SW = true ;
    for ( iter_t it1 = first, it2 = first+1 ;
          it2 != last and (SW = not comp(*it2,*it1));it1 = it2++);
    if (SW) return ;

    //------------------------------------------------------------------------
    //                    PROCESS
    //------------------------------------------------------------------------
    initial_configuration () ;
    first_merge ( );
    construct = true ;
    final_merge ( );

};
//
//-----------------------------------------------------------------------------
//  function :~sample_sort_tag
/// @brief destructor of the class. The utility is to destroy the temporary
///        buffer used in the sorting process
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
~sample_sort_tag ( void)
{   //----------------------------------- begin -------------------------
    if ( construct)
    {   destroy ( range<value_t*> ( Ptr, Ptr +NElem));
        construct = false ;
    }
    if ( Ptr != nullptr and owner ) std::return_temporary_buffer ( Ptr) ;
};
//
//-----------------------------------------------------------------------------
//  function : execute first
/// @brief this a function to assign to each thread in the first merge
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
inline void execute_first ( void)
{   //------------------------------- begin ----------------------------------
    uint32_t Job =0 ;
    while ((Job = atomic_add(NJobs, 1)) < Ninterval   )
    {   uninit_merge_level4( VBIni[Job] , VMem[Job],VBuf[Job] ,comp);
    };
};
//
//-----------------------------------------------------------------------------
//  function : execute
/// @brief this is a function to assignt each thread the final merge
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
inline void execute ( void)
{   //------------------------------- begin ----------------------------------
    uint32_t Job =0 ;
    while ((Job = atomic_add(NJobs, 1)) < Ninterval   )
    {   merge_vector4( VBIni[Job], VMIni[Job] ,VBuf[Job], VMem[Job], comp);
    };
};
//
//-----------------------------------------------------------------------------
//  function : first merge
/// @brief Implement the merge of the initially sparse ranges
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
inline void first_merge ( void)
{   //---------------------------------- begin -------------------------------
    NJobs =0 ;
    std::vector <std::thread > VThread ( NThr);
    for ( uint32_t i =0  ; i < NThr ; ++i )
    {   VThread [i] = std::thread
                     (&sample_sort_tag<iter_t,compare>::execute_first , this);
    };
    for ( uint32_t i =0  ; i < NThr ; ++i )
    {   VThread [i].join();
    };
};
//
//-----------------------------------------------------------------------------
//  function : final merge
/// @brief Implement the final merge of the ranges
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
inline void final_merge ( void)
{   //---------------------------------- begin -------------------------------
    NJobs =0 ;

    std::vector <std::thread > VThread ( NThr);
    for ( uint32_t i =0  ; i < NThr ; ++i )
    {   VThread [i] = std::thread (&sample_sort_tag<iter_t,compare>::execute , this);
    };
    for ( uint32_t i =0  ; i < NThr ; ++i )
    {   VThread [i].join();
    };

};
//
//----------------------------------------------------------------------------
};//                    End class sample_sort_tag
//----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
//  function : initial_configuration
/// @brief Create the internal data structures, and obtain the inital set of
///        ranges to merge
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
template <class iter_t, typename compare>
void sample_sort_tag<iter_t,compare>::initial_configuration ( void)
{   //--------------------------- begin --------------------------------------
    std::vector <range_it> Vrange_thread ;
    std::vector <value_t*> Vbuf_thread   ;

    //------------------------------------------------------------------------
    size_t cupo = (NElem + NThr -1) / NThr ;
    iter_t it_first = global_range.first ;

    for ( uint32_t i =0 ; i < NThr ; ++i)
    {   size_t Nlast = (((i+1)*cupo)< NElem )?((i+1)*cupo):NElem ;
    	Vrange_thread.emplace_back ( it_first+ (i*cupo) , it_first+Nlast);
        Vbuf_thread.push_back ( Ptr + ( i*cupo) );
    };
    //------------------------------------------------------------------------
    // Sorting of the ranges
    //------------------------------------------------------------------------
    std::vector <std::thread > VThread ( NThr );
    for ( uint32_t i =0  ; i < NThr ; ++i )
    {   VThread [i] = std::thread (smart_merge_sort<iter_t,value_t,compare>,
                                   Vrange_thread[i].first,Vrange_thread[i].last, comp,
                                   Vbuf_thread[i] , Vrange_thread[i].size());
    };
    for ( uint32_t i =0  ; i < NThr ; ++i )
    {   VThread [i].join();
    };
    //------------------------------------------------------------------------
    // Obtain the vector of milestones
    //------------------------------------------------------------------------
    std::vector<iter_t> Vsample;
    Vsample.reserve ( NThr * (Ninterval-1)) ;

    for ( uint32_t i =0 ; i < NThr ; ++i)
    {   size_t distance = Vrange_thread[i].size() / Ninterval ;
        for ( size_t j = 1 ,pos = distance; j < Ninterval; ++j,pos+=distance)
        {   Vsample.push_back (Vrange_thread[i].first + pos );
        };
    };
    typedef less_ptr_no_null <iter_t, compare>  compare_ptr ;
    smart_merge_sort  ( Vsample.begin() , Vsample.end(), compare_ptr(comp) );

    //------------------------------------------------------------------------
    // Create the final milestone vector
    //------------------------------------------------------------------------
    std::vector<iter_t> Vmilestone ;
    Vmilestone.reserve ( Ninterval);

    for ( uint32_t Pos =NThr >>1 ; Pos < Vsample.size() ; Pos += NThr )
        Vmilestone.push_back ( Vsample [ Pos]);

    //------------------------------------------------------------------------
    // Creation of the first vector of ranges
    //------------------------------------------------------------------------
    std::vector< std::vector<range <iter_t> > > VR  (NThr);

    for ( uint32_t i =0 ; i < NThr; ++i)
    {   iter_t itaux = Vrange_thread[i].first ;
        for ( uint32_t k =0 ; k < (Ninterval -1) ; ++k)
        {   iter_t it2 = std::upper_bound ( itaux,
                                            Vrange_thread[i].last ,
                                            * Vmilestone[k], comp );
            VR[i].emplace_back ( itaux, it2);
            itaux = it2 ;
        };
        VR[i].emplace_back(itaux,Vrange_thread[i].last );
    };

    //------------------------------------------------------------------------
    // Copy in buffer and  creation of the final matrix of ranges
    //------------------------------------------------------------------------
    VMem.resize ( Ninterval);
    VBuf.resize ( Ninterval);
    VMIni.reserve (Ninterval);
    VBIni.reserve (Ninterval);

    for ( uint32_t i =0 ; i < Ninterval ; ++i)
    {   VMem[i].reserve ( NThr);
        VBuf[i].reserve ( NThr);
    };
    iter_t it = global_range.first ;
    value_t * it_buf = Ptr ;
    for ( uint32_t k =0 ; k < Ninterval ; ++k)
    {   VMIni.push_back (it );
        VBIni.push_back (it_buf) ;
        size_t N =0 ;
        for ( uint32_t i = 0 ; i< NThr ; ++i)
        {   size_t N2 = VR[i][k].size();
            if ( N2 != 0 ) VMem[k].push_back(VR[i][k] );
            N += N2 ;
        };
        it += N ;
        it_buf += N ;
    };
};
//
//############################################################################
//                                                                          ##
//                 F U N C T I O N   F O R M A T                            ##
//                             A N D                                        ##
//               I N D I R E C T   F U N C T I O N S                        ##
//                                                                          ##
// These functions are for to select the correct format depending of the    ##
// number and type of the parameters                                        ##
//############################################################################
//
//-----------------------------------------------------------------------------
//  function : sample_sort
/// @brief envelope function for to call to a sample_sort_tag object
/// @tparam iter_t : iterator used for to access to the data
/// @param [in] first : iterator to the first element of the range
/// @param [in] last : iterator to next element after the last of the range
/// @param [in] NT : NThread object for to define the number of threads to use
///                  in the process. By default is the number of thread HW
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
template < class iter_t >
void sample_sort ( iter_t first, iter_t last , const NThread &NT = NThread() )
{   //----------------------------------- begin ------------------------------
    typedef std::less<typename iter_value<iter_t>::type > compare;
    sample_sort_tag <iter_t,compare> ( first, last, compare(),NT);
};
//
//-----------------------------------------------------------------------------
//  function : sample_sort
/// @brief envelope function for to call to a sample_sort_tag object
/// @tparam iter_t : iterator used for to access to the data
/// @tparam compare : object for to compare two elements
/// @param [in] first : iterator to the first element of the range
/// @param [in] last : iterator to next element after the last of the range
/// @param [in] comp : object for to compare two elements
/// @param [in] NT : NThread object for to define the number of threads to use
///                  in the process. By default is the number of thread HW
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
template < class iter_t,
          typename compare = std::less <typename iter_value<iter_t>::type>  >
void sample_sort ( iter_t first, iter_t last,
                   compare comp, const NThread &NT = NThread() )
{   //----------------------------- begin ----------------------------------
    sample_sort_tag<iter_t,compare> ( first, last,comp,NT);
};


//############################################################################
//                                                                          ##
//                I N D I R E C T     F U N C T I O N S                     ##
//                                                                          ##
//############################################################################
//
//-----------------------------------------------------------------------------
//  function : indirect_sample_sort
/// @brief indirect sorting using the sample_sort algorithm
/// @tparam iter_t : iterator used for to access to the data
/// @param [in] first : iterator to the first element of the range
/// @param [in] last : iterator to next element after the last of the range
/// @param [in] NT : NThread object for to define the number of threads to use
///                  in the process. By default is the number of thread HW
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
template < class iter_t >
void indirect_sample_sort ( iter_t first, iter_t last ,
                                   const NThread &NT = NThread() )
{   //------------------------------- begin--------------------------
    typedef std::less <typename iter_value<iter_t>::type> compare ;
    typedef less_ptr_no_null <iter_t, compare>      compare_ptr ;

    std::vector<iter_t> VP ;
    create_index ( first , last , VP);
    sample_sort  ( VP.begin() , VP.end(), compare_ptr(),NT );
    sort_index ( first , VP) ;
};
//
//-----------------------------------------------------------------------------
//  function : indirect_sample_sort
/// @brief indirect sorting using the sample_sort algorithm
/// @tparam iter_t : iterator used for to access to the data
/// @tparam compare : object for to compare two elements
/// @param [in] first : iterator to the first element of the range
/// @param [in] last : iterator to next element after the last of the range
/// @param [in] comp : object for to compare two elements
/// @param [in] NT : NThread object for to define the number of threads to use
///                  in the process. By default is the number of thread HW
/// @exception
/// @return
/// @remarks
//-----------------------------------------------------------------------------
template < class iter_t,
          typename compare = std::less <typename iter_value<iter_t>::type> >
void indirect_sample_sort ( iter_t first, iter_t last,
                            compare comp1, const NThread &NT = NThread() )
{   //----------------------------- begin ----------------------------------
    typedef less_ptr_no_null <iter_t, compare>      compare_ptr ;

    std::vector<iter_t> VP ;
    create_index ( first , last , VP);
    sample_sort  ( VP.begin() , VP.end(), compare_ptr(comp1),NT );
    sort_index ( first , VP) ;
};
//
//****************************************************************************
};//    End namespace algorithm
};//    End namespace parallel
};//    End namespace sort
};//    End namespace boost
//****************************************************************************
//
#endif
