#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <omp.h>
#include "arrayff.hxx"
#include "draw.hxx"
#include <fstream>
#include <iostream>
using namespace std;


double seq( int npix)
{
    const float tol = 0.00001;
    int npixx = npix;
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
    cout << "nconverged:"<<nconverged <<endl;
    std::cout << "It takes " << omp_get_wtime() - t_start << " seconds\n";
    dump_array<float, 2>(h, "plate1.fit");
    std::cout << "Required " << iter << " iterations" << std::endl;

    return omp_get_wtime() - t_start;

}

int main(int argc, char* argv[])
{

    const int npix = atoi(argv[1]);
    const int img_size = atoi(argv[1]);
    const int nprocs = atoi(argv[2]);

    double seq_time_cost =  seq(img_size);
    cout << "-----------------parallel-------------"<<endl;
    const float tol = 0.00001;
    const int ITMAX = 1000000;
    const int npixx = npix;
    const int npixy = npix;
    const int nprocs_has = omp_get_num_procs();

    std::cout << "image size: "<<img_size<<"*"<<img_size<<
              "; total has:"<<nprocs_has << " processors ; using " << nprocs << " threads." << std::endl;

    Array<float, 2> h(npixy, npixy), g(npixy, npixx);
    fix_boundaries2(h);
    //no need to check the 4 boundariess
//    const int nrequired = (npixx - 2)*(npixy - 2);
    const int nrequired = (npixx )*(npixy );
    int nconverged = 0;
//    omp_set_dynamic(0);
    omp_set_num_threads(nprocs);
    int iter = 0;
    double t_start = omp_get_wtime();
#pragma omp parallel
    {
        int i, id,nthrds, p_start1,p_end1,p_start2,p_end2 , step,istart,iend;
        id = omp_get_thread_num();
        nthrds = omp_get_num_threads();

        step =  (npixy-1) / nthrds;
        istart = id * step;
        iend = (id + 1 ) * step;
        if(id == nthrds-1){
            iend = npixy-1;
        }

        do {

            for (int y = istart; y < iend; y++) {
                for (int x = 1; x < npixx - 1; ++x) {
                    g(y, x) = 0.25 * (h(y, x - 1) + h(y, x + 1) + h(y - 1, x) + h(y + 1, x));
                }
            }
#pragma omp barrier
#pragma omp single
            {
                fix_boundaries2(g);
                nconverged = 0;
            }
#pragma omp for  reduction(+:nconverged)

            for (int y = 0; y < npixx; y++) {
                for (int x = 0; x < npixx; ++x) {
                    float dhg = std::fabs(g(y, x) - h(y, x));
                    if (dhg < tol) ++nconverged;
                    h(y, x) = g(y, x);
                }
            }
#pragma omp single
            {
                ++iter;
            }

        } while (nconverged < nrequired && iter < ITMAX);
    }

    cout<< "nconverged:" <<nconverged <<endl;

    dump_array<float, 2>(h, "plate11.fit");
    std::cout << "It takes " << omp_get_wtime() - t_start << " seconds  "<< std::endl;
    std::cout << "Executed " << iter << " iterations to converge." << std::endl;

    ofstream file("report.txt",ios::app);
    if (file.is_open())
    {
        file <<"image_size:"<< to_string(img_size)<<
        "; seq:"<<to_string(seq_time_cost);
        file << "; par:total_pro "<<to_string(nprocs_has)<< ", use " <<to_string(nprocs)
        << ", time " << to_string( omp_get_wtime() - t_start)<< "\n";
        file.close();
    }
    cout << "result recorded in report.txt~!"<<endl;
}
