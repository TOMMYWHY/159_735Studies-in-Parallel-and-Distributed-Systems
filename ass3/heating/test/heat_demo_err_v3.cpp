#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <omp.h>
#include "../arrayff.hxx"
#include "../draw.hxx"

using namespace std;

void calc_seq()
{
    const float tol = 0.00001;
    const int npix = 100;
    const int npixx = npix;
    const int npixy = npix;
    const int ntotal = npixx * npixy;

    Array<float, 2> h(npixy, npixy), g(npixy, npixx);

    const int nrequired = npixx * npixy;
    const int ITMAX = 1000000;

    int iter = 0;
    int nconverged = 0;

    fix_boundaries2(h);
    dump_array<float, 2>(h, "plate0.fit");
    double t_start = omp_get_wtime();
    do {

        for (int y = 1; y < npixy - 1; ++y) {
            for (int x = 1; x < npixx - 1; ++x) {
                g(y, x) = 0.25 * (h(y, x - 1) + h(y, x + 1) + h(y - 1, x) + h(y + 1, x));
            }
        }

        fix_boundaries2(g);

        nconverged = 0;
        for (int y = 0; y < npixy; ++y) {
            for (int x = 0; x < npixx; ++x) {
                float dhg = std::fabs(g(y, x) - h(y, x));
                if (dhg < tol) ++nconverged;
                h(y, x) = g(y, x);
            }
        }
        ++iter;

    } while (nconverged < nrequired && iter < ITMAX);

    dump_array<float, 2>(h, "plate1.fit");
    std::cout << "The Serial spend: " << omp_get_wtime() - t_start << " s\n";
    std::cout << "Need : " << iter << " iterations to converge." << std::endl;

}

int main(int argc, char* argv[])
{
    calc_seq();

    const float tol = 0.00001;
    const int max_iter = 1000000;
    const int npix = atoi(argv[1]);
    const int npixx = npix;
    const int npixy = npix;

    const int nprocs = omp_get_num_procs();
    std::cout << "Total: " << nprocs << " processors." << std::endl;

    Array<float, 2> h(npixy, npixy), g(npixy, npixx);
    fix_boundaries2(h);
    dump_array<float, 2>(h, "plate0.fit");

    //no need to check the 4 boundariess
    const int converge_edge = (npixx - 2)*(npixy - 2);
    int nconverged = 0;
    omp_set_dynamic(0);
    omp_set_num_threads(nprocs);
    int iter = 0;
    double t_start = omp_get_wtime();
    for (iter = 0; iter < max_iter; iter++)
    {
#pragma omp parallel
        {
            int nThreads = omp_get_num_threads();
            int myId = omp_get_thread_num();


#pragma omp parallel for  collapse(2)
            for (int y = 1; y < npixy-1; ++y) {
                for (int x = 1; x < npixx - 1; ++x) {
                    g(y, x) = 0.25 * (h(y, x - 1) + h(y, x + 1) + h(y - 1, x) + h(y + 1, x));
                }
//                printf("y = %d, I am Thread %d\n", y, omp_get_thread_num());
            }

    #pragma omp barrier
    //#pragma omp master
    #pragma omp single

            {
                fix_boundaries2(g);
                nconverged = 0;
//                printf("I am Thread %d\n", omp_get_thread_num());
            }

#pragma omp parallel for reduction (+:nconverged) //calculate the converged cells
//    #pragma omp  for
            for (int y = 1; y < npixy-1; ++y)
            {
                for (int x = 1; x < npixx - 1; x++)
                {
                    float delta = fabs(g(y, x) - h(y, x));
                    if (delta < tol)
                        nconverged += 1;
                    h(y, x) = g(y, x);
                }
//                printf("y = %d, I am part2 Thread %d\n", y, omp_get_thread_num());

            }
        }
        if (nconverged == converge_edge)
            break;
    }

    dump_array<float, 2>(h, "plate1.fit");
    std::cout << "Use "<<nprocs<< " threads spend: " << omp_get_wtime() - t_start << "s \n";
    std::cout << "Need : " << iter/nprocs << " iterations to converge." << std::endl;
}
