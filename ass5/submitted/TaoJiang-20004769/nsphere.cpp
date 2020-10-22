/* 159.735 Semester 2, 2016.  Ian Bond, 3/10/2016
 
 Sequential version of the N-sphere counting problem for Assignment
 5. Two alternative algorithms are presented.

 Note: a rethink will be needed when implementing a GPU version of
 this. You can't just cut and paste code.

 To compile: g++ -O3 -o nsphere nsphere.cpp
 (you will get slightly better performance with the O3 optimization flag)
*/
#include <cstdlib>
#include <cmath>

#include <iostream>
#include <string>

#include <vector>
#include <fstream>
using namespace std;

const long MAXDIM = 10;
const double RMIN = 2.0;
const double RMAX = 8.0;

long powlong(long n, long k)
/* Evaluate n**k where both are long integers */
{
  long p = 1;
  for (long i = 0; i < k; ++i) p *= n;
  return p;
}

/*----------------------------------------------------------------------------*/

void convert(long num, long base, std::vector<long>& index)
/* Convert a decimal number into another base system - the individual
   digits in the new base are stored in the index array. */
{
  const long ndim = index.size();
  for (long i = 0; i < ndim; ++i) {
    index[i] = 0;
  }
  long idx = 0;
  while (num != 0) {
    long rem = num % base;
    // cout << "rem:"<<rem<<",";
    num = num / base;
      // cout << "num:"<<num<<",";
    index[idx] = rem;
    ++idx;
  }
  // cout <<endl;
}

long count_in_v1(long ndim, double radius)
/* 
   Version 1 of the counting algorithm. Given:

   ndim   -> number of dimensions of the hypersphere
   radius -> radius of the hypersphere

   count the number of integer points that lie wholly within the
   hypersphere, assuming it is centred on the origin.
*/
{
  const long halfb = static_cast<long>(floor(radius));
  const long base = 2 * halfb + 1;
  const double rsquare = radius * radius;

  // This is the total number of points we will need to test.
  const long ntotal = powlong(base, ndim);
  cout  <<"ntotal:"<<ntotal 
      <<" halfb:"<<halfb
      <<" base:"<<base
      <<" rsquare:"<<rsquare
      <<endl;
  long count = 0;

  // Indices in x,y,z,.... 
  std::vector<long> index(ndim, 0);

  // Loop over the total number of points. For each visit of the loop,
  // we covert n to its equivalent in a number system of given "base".
    // for (int i = 0; i < index.size(); i++)
    // {
    //   cout << index[i] << ", ";
    // }
    // cout <<endl;
    
    // convert(10, base, index);
    cout<< "---" <<endl;
  // for (int i = 0; i < index.size(); i++)
  //     {
  //       cout << index[i]<<" ";
  //     }
  //     cout <<endl;

  for (long n = 0; n < ntotal; ++n) {
    convert(n, base, index);
    double rtestsq = 0;
    for (long k = 0; k < ndim; ++k) {
      // cout<<"index[k]:"<<index[k]<<"; ";
      double xk = index[k] - halfb;

      rtestsq += xk * xk; 
      // cout<< "xk:"<<xk  << "; rtestsq:"<<rtestsq<<"; ";

    }
    // cout << endl;
    if (rtestsq < rsquare) ++count;
  }

  return count;
}

/*----------------------------------------------------------------------------*/

int main(int argc, char* argv[]) 
{
  // You can make this larger if you want
//  const long ntrials = 20;
  const long ntrials = 1;

  for (long n = 0; n < ntrials; ++n) {

    // Get a random value for the hypersphere radius between the two limits
//    const double r = drand48() * (RMAX - RMIN) + RMIN;

    // Get a random value for the number of dimensions between 1 and
    // MAXDIM inclusive
//    const long  nd = lrand48() % (MAXDIM - 1) + 1;
      double r = 2;
      long nd = 3;

      std::cout << "### n:" << n << ";  r:" << r << "; nd:" << nd << "; ... " << std::endl;

    const long num1 = count_in_v1(nd, r);
    std::cout << " -> " << num1 << " " << std::endl;
    
    

  }
  return 0;
}

