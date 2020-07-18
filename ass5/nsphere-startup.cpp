// 159.735 Semester 1, 2015
// Start-up program for Assignment 5
//
#include <cstdlib>
#include <cmath>

#include <iostream>
#include <string>

#include <vector>

const long MAXDIM = 7;
const double RMIN = 2.0;
const double RMAX = 8.0;

long count_in(long ndim, double radius)
/* 
   Given:

   ndim   -> number of dimensions of the hypersphere
   radius -> radius of the hypersphere

   count the number of integer points that lie wholly within the
   hypersphere, assuming it is centred on the origin.

*/

{
  long count = 0;

  // Provide your implementation to count the number of integer points
  // inside an N-sphere of given radius

  return count;
}

int main(int argc, char* argv[]) 
{
  const long ntrials = 10;

  long ndone = 0;

  for (long n = 0; n < ntrials; ++n) {
    const double r = drand48() * (RMAX - RMIN) + RMIN;
    const long  nd = lrand48() % (MAXDIM - 1) + 1;
    //double r = 1.5; 
    //long nd = 3;

    std::cout << "### " << ndone << " " << n << " " << r << " " << nd 
	      << " ... " << std::endl;

    const long num = count_in(nd, r);

    std::cout << " -> " << num << " " << std::endl;
    ++ndone;
  }
}

