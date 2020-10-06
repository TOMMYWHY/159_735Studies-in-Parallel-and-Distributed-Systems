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

int main(int argc, char* argv[])
{
    const float tol = 0.00001;

    // X and Y dimensions. Force it to be a square.
    const int npix = atoi(argv[1]);
    const int npixx = npix;
    const int npixy = npix;
    const int ntotal = npixx * npixy;

    // Images as 2D arrays: h is the current image, g is the updated
    // image. To access individual pixel elements, use the () operator.
    // Note: that y is the first index (to reflect row major
    // order). Eg: h(y, x) = fubar
    Array<float, 2> h(npixy, npixx), g(npixy, npixx);
    const int nrequired = npixx * npixy;
    const int ITMAX = 1000000;
    int iter = 0;
    int nconverged = 0;

    // Draw the printed circuit components
    fix_boundaries2<float>(h);

    // This is the initial value image where the boundaries and printed
    // circuit components have been fixed
//  dump_array<float, 2>(h, "plate0.fit");

    // Complete the sequential version to compute the heat transfer,
    // then make a parallel version of it

    do {

        for (int y = 1; y < npixy-1; ++y) {
            for (int x = 1; x < npixx-1; ++x) {
                g(y, x) = 0.25 * (h(y, x-1) + h(y, x+1) + h(y-1, x) + h(y+1,x));
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
    std::cout << "Required " << iter << " iterations" << std::endl;
}
