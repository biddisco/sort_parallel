//----------------------------------------------------------------------------
/// @file benchmark_CLANG_TBB_BOOST.cpp
/// @brief Benchmark of several sort methods with different objects
///
/// @author Copyright (c) 2010 2015 Francisco Jos�� Tapia (fjtapia@gmail.com )\n
///         Distributed under the Boost Software License, Version 1.0.\n
///         ( See accompanyingfile LICENSE_1_0.txt or copy at
///           http://www.boost.org/LICENSE_1_0.txt  )
///         This program use for comparison purposes only the TimSort
///         implementation, https://github.com/gfx/cpp-TimSort which license is
///         https://github.com/gfx/cpp-TimSort/blob/master/LICENSE
///         This program use for comparison purposes, the Threading Building
///         Blocks which license is the GNU General Public License, version 2
///         as  published  by  the  Free Software Foundation.
/// @version 0.1
///
/// @remarks
//-----------------------------------------------------------------------------
#ifdef SORT_HAS_PPL
#endif

#ifdef SORT_HAS_HPX
# include <hpx/parallel/algorithms/sort.hpp>
#endif

#ifdef SORT_HAS_GCC_PARALLEL
# include <parallel/basic_iterator.h>
# include <parallel/features.h>
# include <parallel/parallel.h>
# include <parallel/algorithm>
# include <omp.h>
#endif

#ifdef SORT_HAS_TBB
# include "tbb/tbb_stddef.h"
# include "tbb/task_scheduler_init.h"
# include "tbb/parallel_sort.h"
# include <parallel_stable_sort/tbb-lowlevel/parallel_stable_sort.h>
#endif

// standard library
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <random>
#include <vector>
#include <file_vector.hpp>

// TimSort
#include "cpp-TimSort-master/timsort.hpp"

// Boost sort
#include <boost/sort/spreadsort/spreadsort.hpp>
#include <boost/sort/parallel/sort.hpp>
#include <boost/sort/parallel/util/time_measure.hpp>
#include "int_array.hpp"

#define NELEM 25000000
#define NMAXSTRING 2000000
using namespace std ;
namespace bsp = boost::sort::parallel ;
namespace bsp_util = bsp::util ;

using bsp_util::time_point ;
using bsp_util::now;
using bsp_util::subtract_time ;
using bsp_util::fill_vector_uint64;
using bsp_util::write_file_uint64;
using bsp::NThread ;

void Generator_sorted(void );
void Generator_uint64(void );
void Generator_string(void) ;

template <class IA>
void Generator (uint64_t N );

template <class IA>
int Prueba  ( const std::vector <IA> & B);

template <class IA>
int Prueba_spreadsort  ( const std::vector <IA> & B );

template <class IA>
int verify(const std::vector <IA> &A) {
    if (A.size()==0) return 1;
    //
    IA temp = *(A.begin());
    for (typename std::vector<IA>::const_iterator it=A.begin(); it!=A.end(); ++it) {
        if ((*it)<temp) return 0;
        temp = (*it);
    }
    return 1;
}
#define VERIFY(A) \
  if (!verify(A)) { \
    std::cout << "Verify sort failed\n"; \
  }

// we need a callback for the HPX runtime to call for this benchmark
// as we start stop the HPX runtime for each test.
#ifdef SORT_HAS_HPX
template <class IA>
int Prueba_hpx ( std::vector <IA> & B );

template <class IA>
int hpx_test(std::vector<IA> &A, int argc, char ** argv)
{
    int result = Prueba_hpx<IA>(A);
    return hpx::finalize();
}
// we need these fo the init function
int    hpx_argc = 0;
char **hpx_argv = NULL;

#endif

int main (int argc, char *argv[] )
{   //------------------------------ Inicio ----------------------------------
    cout<<"****************************************************************\n";
    cout<<"**                                                            **\n";
    cout<<"**         B O O S T :: S O R T :: P A R A L L E L            **\n";
    cout<<"**                                                            **\n";
    cout<<"**               F A S T  B E N C H M A R K                   **\n";
    cout<<"**                                                            **\n";
    cout<<"****************************************************************\n";
    cout<<std::endl;
    std::cout.flush();
    system ( "lscpu");
    std::cout.flush();
    cout<<"\n";

#ifdef SORT_HAS_HPX
    hpx_argc = argc;
    hpx_argv = argv;
#endif

    //------------------------------------------------------------------------
    // Execution with different object format
    //------------------------------------------------------------------------
    Generator_sorted() ;
    Generator_uint64() ;
    Generator_string () ;
    Generator< int_array<1> >   ( NELEM   );
    Generator< int_array<2> >   ( NELEM>>1);
    Generator< int_array<4> >   ( NELEM>>2);
    Generator< int_array<8> >   ( NELEM>>3);
    Generator< int_array<16> >  ( NELEM>>4);
    Generator< int_array<32> >  ( NELEM>>5);
    Generator< int_array<64> >  ( NELEM>>6);
    return 0 ;
}
void Generator_sorted(void )
{   //---------------------------- begin--------------------------------------
    vector<uint64_t> Z  ;
    Z.reserve ( NELEM) ;
    cout<<"  "<<NELEM<<" uint64_t elements already sorted\n" ;
    cout<<"=================================================\n";
    for ( size_t i =0 ; i < NELEM ; ++i)
        Z.push_back( i );
    Prueba(Z) ;
    Prueba_spreadsort( Z);
    cout<<std::endl ;
}
void Generator_uint64(void )
{   //---------------------------- begin--------------------------------------
    vector<uint64_t> Z  ;
    Z.reserve ( NELEM) ;
    cout<<"  "<< NELEM<<" uint64_t elements randomly filled\n" ;
    cout<<"=================================================\n";

    if ( fill_vector_uint64("input.bin", Z, NELEM) != 0)
    {   std::cout<<"Error in the input file\n";
        return ;
    };
    Prueba(Z) ;
    Prueba_spreadsort( Z);
    cout<<std::endl ;
}
void Generator_string(void)
{   //------------------------------- begin ----------------------------------
    cout<<"  "<< NMAXSTRING<<" strings randomly filled\n" ;
    cout<<"===============================================\n";
    std::vector <std::string> Z ;
    Z.reserve ( NMAXSTRING);

    if ( bsp_util::fill_vector_string("input.bin", Z, NMAXSTRING) != 0)
    {   std::cout<<"Error in the input file\n";
        return ;
    };
    Prueba(Z) ;
    Prueba_spreadsort( Z);
    cout<<std::endl ;
};

template <class IA>
void Generator (uint64_t N )
{   //------------------------------- begin ----------------------------------
    bsp::util::uint64_file_generator gen ( "input.bin");
    vector<IA> Z ;
   	Z.reserve ( N);
   	IA Aux;

    //------------------------ Inicio -----------------------------
    cout<<N<<" elements of size "<<sizeof (IA)<<" randomly filled \n";
    cout<<"=============================================\n";
    gen.reset() ;
    for ( uint32_t i = 0 ; i < N ; i ++)
        Z.emplace_back(IA::generate(Aux,gen)) ;
    Prueba(Z) ;
    cout<<std::endl ;

};

// when hpx::init() is called, the hpx_main function is called
// we set a calback to the actual hpx test each time and call it from here
#ifdef SORT_HAS_HPX

template <class IA>
int Prueba_hpx ( std::vector <IA> & A )
{
    double duracion ;
    time_point start, finish;

    cout<<"HPX parallel introsort       : ";
    start= now() ;
    hpx::parallel::algorithms::parallel_introsort (A.begin() , A.end() );
    VERIFY(A);
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
    return hpx::finalize();

}
#endif

template <class IA>
int Prueba  ( const std::vector <IA> & B )
{   //---------------------------- begin --------------------------------
	double duracion ;
	time_point start, finish;
	std::vector <IA> A ( B);
    std::less<IA>  comp ;

    A = B ;
    cout<<"std::sort                    : ";
    start= now() ;
    std::sort (A.begin() , A.end()  );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
    VERIFY(A);

    A = B ;
    cout<<"Boost introsort              : ";
    start= now() ;
    boost::sort::parallel::introsort (A.begin() , A.end() );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
    VERIFY(A);

    A = B ;
    cout<<"std::stable_sort             : ";
    start= now() ;
    std::stable_sort (A.begin() , A.end()  );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
    VERIFY(A);

    A = B ;
    cout<<"Boost smart_merge_sort       : ";
    start= now() ;
    boost::sort::parallel::smart_merge_sort (A.begin() , A.end()  );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
    VERIFY(A);

    A = B ;
    cout<<"Timsort                      : ";
    start= now() ;
    gfx::timsort (A.begin() , A.end()  );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
    VERIFY(A);

#ifdef SORT_HAS_TBB
    A = B ;
    cout<<"TBB parallel_sort            : ";
    start= now() ;
    tbb::parallel_sort (A.begin() , A.end()  );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
    VERIFY(A);
#endif

    A = B ;
    cout<<"Boost parallel introsort     : ";
    start= now() ;
    bsp::parallel_introsort (A.begin() , A.end() );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
    VERIFY(A);

    A = B ;
    cout<<"Boost parallel stable sort   : ";
    start= now() ;
    bsp::parallel_stable_sort (A.begin() , A.end() ,NThread() );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
    VERIFY(A);

    A = B ;
    cout<<"Boost sample sort            : ";
    start= now() ;
    bsp::sample_sort (A.begin() , A.end() ,NThread() );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
    VERIFY(A);

#ifdef SORT_HAS_TBB
    A = B ;
    cout<<"TBB parallel stable sort     : ";
    start= now() ;
    pss::parallel_stable_sort (A.begin() , A.end(),comp );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
    VERIFY(A);
#endif

#ifdef SORT_HAS_HPX
    A = B ;
    using hpx::util::placeholders::_1;
    using hpx::util::placeholders::_2;
    hpx::util::function_nonser<int(int, char**)> callback = hpx::util::bind(&hpx_test<IA>, A, _1, _2);
    hpx::init(callback, "dummyname", hpx_argc, hpx_argv, hpx::runtime_mode_default );
#endif
    return 0 ;
};

template <class IA>
int Prueba_spreadsort  ( const std::vector <IA> & B )
{   //---------------------------- begin --------------------------------
	double duracion ;
	time_point start, finish;
	std::vector <IA> A ( B);

	//--------------------------------------------------------------------
	A = B ;
    cout<<"Boost spreadsort             : ";
    start= now() ;
    boost::sort::spreadsort::spreadsort (A.begin() , A.end()  );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
    return 0;
};
