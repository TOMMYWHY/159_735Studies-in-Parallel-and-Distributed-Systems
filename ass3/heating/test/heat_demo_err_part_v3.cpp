/******************************************************************************

Start up demo program for 159735 Assignment 3 Semester 1 2013

All this does is initialize the image and write it to a file.

To compile:

make heat_demo

To run (for example to make a 100X100 pixel image):

./heat_demo 100


******************************************************************************/
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <omp.h>
#include "arrayff.hxx"
#include "draw.hxx"


using namespace std;

int main(int argc, char* argv[]) {

    const float tol = 0.00001;
    const int max_iter = 1000000;
    const int npix = atoi(argv[1]);
    const int npixx = npix;
    const int npixy = npix;

    const int num_thds = 4;

//    const int nprocs = omp_get_num_procs();
//    std::cout << "there are " << nprocs << " processors." << std::endl;

    Array<float, 2> h(npixy, npixy), g(npixy, npixx);
    const int nrequired = npixx * npixy;
    const int ITMAX = 1000000;

//    std::vector<int> iter(NUM_THREADS) ;
//    std::vector<int> nconverged(NUM_THREADS) ;

    int iter = 0;
    int nconverged = 0;

    fix_boundaries2(h);
//    dump_array<float, 2>(h, "plate0.fit");
    int NUM_THREADS = 10;
    omp_set_num_threads(NUM_THREADS);

    //no need to check the 4 boundariess
    const int required_cells = (npixx - 2) * (npixy - 2);
    int converged_cells = 0;
//    omp_set_dynamic(0);
//    omp_set_num_threads(num_thds);
//    double t_start = omp_get_wtime();
    for (iter = 0; iter < max_iter; iter++) {
#pragma omp parallel
        {
//            int nThreads = omp_get_num_threads();
//            int myId = omp_get_thread_num();
            //split the plate into nThreads parts and each thread is working on one part
            std :: cout << " Hello     " << std:: endl;

//#pragma omp barrier


        }
    }
}
