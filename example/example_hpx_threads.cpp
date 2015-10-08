//----------------------------------------------------------------------------
/// @file example_default_threads.cpp
/// @brief this program is simple example of the parallel_introsort and
///        sample_sort with the thread parameter by default of the new
///        hpx::parallel::sort library
///
/// @author Copyright (c) 2010 2015 Francisco Jos?????? Tapia (fjtapia@gmail.com )\n
///         Distributed under the Boost Software License, Version 1.0.\n
///         ( See accompanyingfile LICENSE_1_0.txt or copy at
///           http://www.boost.org/LICENSE_1_0.txt  )
/// @version 0.1
///
/// @remarks
//-----------------------------------------------------------------------------
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>

#include <iostream>
#include <vector>
#include <random>
#include <stdlib.h>
#include <algorithm>
#include <random>

#include <hpx/parallel/algorithms/sort.hpp>

//#include <parallel_stable_sort/tbb-lowlevel/parallel_stable_sort.h>

//#include <boost/sort/spreadsort/spreadsort.hpp>
#include <boost/sort/parallel/util/time_measure.hpp>
#include "../benchmark/include/int_array.hpp"
#include "../benchmark/include/file_vector.hpp"

#define NELEM 25000000
#define NMAXSTRING 2000000
using namespace std ;
namespace bsp = hpx::parallel::sort;
namespace bsp_util = boost::sort::parallel::util ;

using bsp_util::time_point ;
using bsp_util::now;
using bsp_util::subtract_time ;
using bsp_util::fill_vector_uint64;
using bsp_util::write_file_uint64;

void Generator_sorted(void );
void Generator_uint64(void );
void Generator_string(void) ;

template <class IA>
void Generator (uint64_t N );

template <class IA>
int Prueba  ( const std::vector <IA> & B);

//template <class IA>
//int Prueba_spreadsort  ( const std::vector <IA> & B );

int benchmark()
{
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
//    Prueba_spreadsort( Z);
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
//    Prueba_spreadsort( Z);
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
//    Prueba_spreadsort( Z);
    cout<<std::endl ;
};


template <class IA>
void Generator (uint64_t N )
{   //------------------------------- begin ----------------------------------
    bsp_util::uint64_file_generator gen ( "input.bin");
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

/*
    A = B ;
    cout<<"Boost introsort              : ";
    start= now() ;
    boost::sort::parallel::introsort (A.begin() , A.end() );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";

    A = B ;
    cout<<"std::stable_sort             : ";
    start= now() ;
    std::stable_sort (A.begin() , A.end()  );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";

    A = B ;
    cout<<"Boost smart_merge_sort       : ";
    start= now() ;
    boost::sort::parallel::smart_merge_sort (A.begin() , A.end()  );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";

    A = B ;
    cout<<"Timsort                      : ";
    start= now() ;
    gfx::timsort (A.begin() , A.end()  );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
*/

    A = B ;
    cout<<"HPX parallel introsort       : ";
    start= now() ;
    bsp::parallel_introsort (A.begin() , A.end() );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
/*
    A = B ;
    cout<<"Boost parallel stable sort   : ";
    start= now() ;
    bsp::parallel_stable_sort (A.begin() , A.end() ,NThread() );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";

    A = B ;
    cout<<"Boost sample sort            : ";
    start= now() ;
    bsp::sample_sort (A.begin() , A.end() ,NThread() );
    finish = now() ;
    duracion = subtract_time(finish ,start) ;
    cout<<duracion<<" secs\n";
*/
    return 0 ;
};

int hpx_main(boost::program_options::variables_map& vm)
{
    hpx::id_type                    here = hpx::find_here();
    uint64_t                        rank = hpx::naming::get_locality_id_from_id(here);
    std::string                     name = hpx::get_locality_name();
    uint64_t                      nranks = hpx::get_num_localities().get();
    std::size_t                  current = hpx::get_worker_thread_num();
    std::vector<hpx::id_type>    remotes = hpx::find_remote_localities();
    std::vector<hpx::id_type> localities = hpx::find_all_localities();
    uint64_t                     threads = hpx::get_os_thread_count();
    //
    char const* msg = "hello world from OS-thread %1% of %2% on locality %3% rank %4% hostname %5%";
    std::cout << (boost::format(msg) % current % threads % hpx::get_locality_id() % rank % name.c_str()) << std::endl;
    std::mt19937_64 my_rand(0);

    const uint32_t NMAX = 10000000 ;
    std::vector <uint64_t> A , B ;
    for ( uint32_t i =0 ; i < NMAX ; ++i)
        A.push_back( my_rand() );
    B = A ;
    //------------------------------------------------------------------------
    // if the thread parameter is not specified, the number of thread used
    // is the number of HW threads of the machine where the program is running.
    // This number is calculate in each execution of the code
    //------------------------------------------------------------------------
    bsp::parallel_introsort ( A.begin(), A.end());
//    bsp::sample_sort ( B.begin() , B.end());
    std::sort ( B.begin() , B.end());

    bool error = false;
    for ( uint32_t i =0 ; i < NMAX ; ++i ) {
        if ( A[i] != B[i]) {
          error = true;
          break;
        }
    }
    if (!error) {
      std::cout << "Sort completed without errors " << std::endl;
    }
    else {
        std::cout<<"Error in the sorting process\n";
    }

    benchmark();

    return hpx::finalize();
}
/*
//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    // Configure application-specific options
    boost::program_options::options_description
       desc_commandline("Usage: " HPX_APPLICATION_STRING " [options]");

    // Initialize and run HPX, this test requires to run hpx_main on all localities
    std::vector<std::string> cfg;
    cfg.push_back("hpx.run_hpx_main!=1");

    return hpx::init(desc_commandline, argc, argv, cfg);
}
*/
