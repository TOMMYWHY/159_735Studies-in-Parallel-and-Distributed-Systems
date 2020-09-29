#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <omp.h>
#include "arrayff.hxx"
#include "draw.hxx"

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

    std::cout << "It takes " << omp_get_wtime() - t_start << " seconds\n";
    dump_array<float, 2>(h, "plate1.fit");
    std::cout << "Required " << iter << " iterations" << std::endl;
}

int main(int argc, char* argv[])
{
    calc_seq();

    const float tol = 0.00001;
    const int ITMAX = 1000000;
    const int npix = atoi(argv[1]);
    const int npixx = npix;
    const int npixy = npix;

    const int nprocs = omp_get_num_procs();
    std::cout << "there are " << nprocs << " processors." << std::endl;

    Array<float, 2> h(npixy, npixy), g(npixy, npixx);
    fix_boundaries2(h);
//    dump_array<float, 2>(h, "plate0.fit");

    //no need to check the 4 boundariess
    const int nrequired = (npixx - 2)*(npixy - 2);
    int nconverged = 0;
    omp_set_dynamic(0);
    omp_set_num_threads(nprocs);
    int iter = 0;
    double t_start = omp_get_wtime();
    for (iter = 0; iter < ITMAX; iter++)
    {
#pragma omp parallel
        {
            int nThreads = omp_get_num_threads();
            int myId = omp_get_thread_num();
            //split the plate into nThreads parts and each thread is working on one part
            int step = npixy / nThreads;
            int yStart = myId * step;
            int yEnd = yStart + step;
            if (yStart == 0)
                yStart = 1;
            if (myId == nThreads - 1)
                yEnd = npixy - 1;

#pragma omp parallel for
            for (int y = yStart; y < yEnd; y++)
            {
                for (int x = 1; x < npixx - 1; x++)
                {
                    g(y, x) = 0.25 * (h(y, x - 1) + h(y, x + 1) + h(y - 1, x) + h(y + 1, x));
                }
            }

#pragma omp barrier
#pragma omp single //'single' implies a barrier at the end of the block, 'master' won't.
            {
                fix_boundaries2(g);
                nconverged = 0;
            }

#pragma omp parallel for reduction (+:nconverged) //calculate the converged cells
            for (int y = yStart; y < yEnd; y++)
            {
                for (int x = 1; x < npixx - 1; x++)
                {
                    float dhg = fabs(g(y, x) - h(y, x));
                    if (dhg < tol)
                        nconverged += 1;
                    h(y, x) = g(y, x);
                }
            }
        }
        if (nconverged == nrequired)
            break;
    }
    dump_array<float, 2>(h, "plate1.fit");
    std::cout << "It takes " << omp_get_wtime() - t_start << " seconds\n";
    std::cout << "Executed " << iter << " iterations to converge." << std::endl;
}
