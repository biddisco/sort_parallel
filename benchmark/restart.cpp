// standard library
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <random>
#include <vector>
#include <file_vector.hpp>

#include <hpx/parallel/algorithms/sort.hpp>

#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>

// we need a callback for the HPX runtime to call for this benchmark
// as we start stop the HPX runtime for each test.
// when hpx::init() is called, the hpx_main function is called
// we set a calback to the actual hpx test each time and call it from here

template <typename IA>
void my_sort(const std::vector<IA> & B) {
  std::vector <IA> A (B);
//  std::sort(A.begin() , A.end());
  hpx::parallel::algorithms::parallel_introsort (A.begin() , A.end() );
}

template <typename IA>
int Prueba_hpx ( )
{
    //std::vector< std::vector<IA> > vec_list;
    //
    for (int i=0; i<200; ++i) {
        uint64_t N = 10000000 + (rand() % (int)(200000 - 100000 + 1));
        std::vector<IA> Z(N, 0) ;
        std::generate(Z.begin(), Z.end(), std::rand);
//        std::cout << N << " elements of size " << sizeof (IA) << " randomly filled \n";
        hpx::async( &my_sort<IA>, std::move(Z));
    }
    return hpx::finalize();
}

template <class IA>
int hpx_test(int argc, char ** argv)
{
    int result = Prueba_hpx<IA>();
    return hpx::finalize();
}
// we need these for the init function
int    hpx_argc = 0;
char **hpx_argv = NULL;

template <class IA>
void Generator ( )
{
    using hpx::util::placeholders::_1;
    using hpx::util::placeholders::_2;
    hpx::util::function_nonser<int(int, char**)> callback = hpx::util::bind(&hpx_test<IA>, _1, _2);
    hpx::init(callback, "dummy", hpx_argc, hpx_argv, hpx::runtime_mode_default );
};

int main (int argc, char *argv[] )
{
    hpx_argc = argc;
    hpx_argv = argv;

    Generator< uint8_t  > (  );
    Generator< uint16_t > (  );
    Generator< uint32_t > (  );
    Generator< uint64_t > (  );
    Generator< float >    (  );
    Generator< double >   (  );
    return 0 ;
}
