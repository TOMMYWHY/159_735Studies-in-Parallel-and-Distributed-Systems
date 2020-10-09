#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <omp.h>
#include "../arrayff.hxx"
#include "../draw.hxx"

using namespace std;

void calc_seq( int npix)
{
    const float tol = 0.00001;
//    const int npix = npix;
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
}

int main(int argc, char* argv[])
{
     int npix = atoi(argv[1]);

    calc_seq(npix);

    const float tol = 0.00001;
    const int ITMAX = 1000000;
//     int npix = atoi(argv[1]);
    const int npixx = npix;
    const int npixy = npix;

    const int nprocs_has = omp_get_num_procs();
    //const int nprocs =4  ;
    const int img_size = atoi(argv[1]);
    const int nprocs = atoi(argv[2]);
    std::cout << "image size: "<<img_size<<"*"<<img_size<<
      "; total has:"<<nprocs_has << " processors ; using " << nprocs << " threads." << std::endl;

    Array<float, 2> h(npixy, npixy), g(npixy, npixx);
    fix_boundaries2(h);
//    dump_array<float, 2>(h, "plate0.fit");

    //no need to check the 4 boundariess
    const int nrequired = (npixx - 2)*(npixy - 2);
//    int nconverged = 0;
    std::vector<int> iter(nprocs) ;
    std::vector<int> nconverged(nprocs) ;

//    omp_set_dynamic(0);
    omp_set_num_threads(nprocs);
    int iter = 0;
    double t_start = omp_get_wtime();

#pragma omp parallel // private(nconverged)

//    for (iter = 0; iter < ITMAX; iter++)
    {
//#pragma omp parallel
        {
            int nThreads = omp_get_num_threads();
            int myId = omp_get_thread_num();
            //split the plate into nThreads parts and each thread is working on one part
            int step = npixy / nThreads;
            int id,yStart_loop1,yStart_loop2,yEnd_loop1,yEnd_loop2;
//            int yStart_loop1 = myId * step;
//            int yEnd = yStart + step;
            if (nThreads == 1){
                yStart_loop1 = 1;
                yStart_loop2 = 0;
                yEnd_loop1 = npixy - 1;
                yEnd_loop2 = npixy;
            }else{
                if(myId==0){
                    yStart_loop1 = 1 ;
                    yStart_loop2 = 0 ;
                    yEnd_loop1 = step;
                    yEnd_loop2 = step;
                }
                else if (myId == nThreads - 1){
                    yStart_loop1 = myId * step ;
                    yStart_loop2 = myId * step ;
                    yEnd_loop1 = npixy - 1;
                    yEnd_loop2 = npixy;
                } else{
                    yStart_loop1 = myId * step ;
                    yStart_loop2 =myId * step;
                    yEnd_loop1 = (myId + 1) * step;
                    yEnd_loop2 =  (myId + 1) * step;
                }
            }


//#pragma omp  for collapse(2)
            for (int y = yStart_loop1; y < yEnd_loop1; y++)
            {
                for (int x = 1; x < npixx - 1; x++)
                {
                    g(y, x) = 0.25 * (h(y, x - 1) + h(y, x + 1) + h(y - 1, x) + h(y + 1, x));
                }
            }

#pragma omp barrier
#pragma omp single
            {
                fix_boundaries2(g);
//                nconverged = 0;
                nconverged[id] = 0;
            }
//#pragma omp barrier

//#pragma omp for collapse(2) reduction (+:nconverged)
            for (int y = yEnd_loop2; y <  yEnd_loop2 ; y++)
            {
                for (int x = 1; x < npixx - 1; x++)
                {
                    float dhg = std::fabs(g(y, x) - h(y, x));

                    if (dhg < tol){
//                        nconverged += 1;
                        ++nconverged[id];
                    }
                    h(y, x) = g(y, x);
                }
            }
//            ++iter[id];
#pragma omp barrier
            int result = 0 ;
            for (int i = 0 ; i < nprocs; i++)
                result += nconverged[i];
            if(result < nrequired ){
                break;
            }
//#pragma omp barrier
        }
//        cout<< "nconverged:" <<nconverged <<endl;
        /*if (nconverged == nrequired)
            break;*/
    }
//    cout<< "nconverged:" <<nconverged <<endl;

    dump_array<float, 2>(h, "plate1.fit");
    std::cout << "It takes " << omp_get_wtime() - t_start << " seconds  "<< std::endl;
//    std::cout << "Executed " << iter << " iterations to converge." << std::endl;
}
