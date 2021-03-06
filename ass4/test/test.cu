/* 
   159735 Parallel Programming

   Startup program for sequential implementation of simulation by ray
   tracing of gravitational lensing.
 */
 #include <ctime>

 #include <iostream>
 #include <string>
 
 #include <cmath>
 #include <cuda.h>
 
 #include "lenses.h"
 #include "arrayff.hxx"
 
 using namespace std;
 // Global variables! Not nice style, but we'll get away with it here.
 // Boundaries in physical units on the lens plane
 const float WL  = 2.0;
 const float XL1 = -WL;
 const float XL2 =  WL;
 const float YL1 = -WL;
 const float YL2 =  WL;
 
 //device
 __device__ void shoot_device(float& xs, float& ys, float xl, float yl, 
   float* xlens, float* ylens, float* eps, int nlenses)
 {
 float dx, dy, dr;
 xs = xl;
 ys = yl;
 for (int p = 0; p < nlenses; ++p) {
  dx = xl - xlens[p];
  dy = yl - ylens[p];
  dr = dx * dx + dy * dy;
  xs -= eps[p] * dx / dr;
  ys -= eps[p] * dy / dr;
 }
 
 }
 // kernel
 __global__ void cuda_shoot(float* lensim, int xsize, int ysize, float lens_scale, float* xlens, float* ylens, float* eps, int nlenses)
 {
   // Source star parameters. You can adjust these if you like - it is
   // interesting to look at the different lens images that result
   const float xsrc = 0.0;      // x and y centre on the map
   const float ysrc = 0.0;
   const float rsrc = 0.1;      // radius
   const float rsrc2 = rsrc * rsrc;
   const float ldc  = 0.5;      // limb darkening coefficient
 //printf("11111111");
   int index = threadIdx.x + blockIdx.x * blockDim.x;
   if (index >= xsize*ysize)
     return;
     
   int iy = index / xsize;
   int ix = index % xsize;
 
   float yl = -2.0 + iy * lens_scale;  //-2.0 is YL1
   float xl = -2.0 + ix * lens_scale;  //-2.0 is XL1
 
   float xs, ys;
 
   shoot_device(xs, ys, xl, yl, xlens, ylens, eps, nlenses);
 
   float xd = xs - xsrc;
   float yd = ys - ysrc;
   float sep2 = xd * xd + yd * yd;
   if (sep2 < rsrc2) {
     float mu = sqrt(1 - sep2 / rsrc2);
     lensim[index] = 1.0 - ldc * (1 - mu);
     printf("%d ,", lensim[index]);
     // lensim(iy, ix) = 1.0 - ldc * (1 - mu);
   }
 }
 
 // float* copy_array_to_device(float* data, int size)
 // {
 //   float *d_buffer;
 //   cudaMalloc(&d_buffer, sizeof(float)*size);
 //   cudaMemcpy(d_buffer, data, sizeof(float)*size, cudaMemcpyHostToDevice);
 //   return d_buffer;
 // }
 
 
 // Used to time code. OK for single threaded programs but not for
 // multithreaded programs. See other demos for hints at timing CUDA
 // code.
 double diffclock(clock_t clock1,clock_t clock2)
 {
   double diffticks = clock1 - clock2;
   double diffms = (diffticks * 1000) / CLOCKS_PER_SEC;
   return diffms; // Time difference in milliseconds
 }
 
 int main(int argc, char* argv[]) 
 {
     const int len_num = atoi(argv[1]);
 
     // Set up lensing system configuration - call example_1, _2, _3 or
   // _n as you wish. The positions and mass fractions of the lenses
   // are stored in these arrays
     float* xlens;
     float* ylens;
     float* eps;
 //    const int nlenses = set_example_3(&xlens, &ylens, &eps);
     const int nlenses = set_example_n(len_num, &xlens, &ylens, &eps);
 
     std::cout << "# Simulating " << nlenses << " lens system" << std::endl;
 
   // Pixel size in physical units of the lens image. You can try finer
   // lens scale which will result in larger images (and take more
   // time).
   const float lens_scale = 0.0025;
   // Size of the lens image
   const int npixx = static_cast<int>(floor((XL2 - XL1) / lens_scale)) + 1;
   const int npixy = static_cast<int>(floor((YL2 - YL1) / lens_scale)) + 1;
   std::cout << "# Building " << npixx << "X" << npixy << " lens image" << std::endl;
 
   // Put the lens image in this array
   Array<float, 2> lensim(npixy, npixx);
   cout << "lensim.ntotal: "<<lensim.ntotal <<endl;
   // for (int i = 0; i<lensim.ntotal; i ++ ){
   //   cout << lensim.buffer[i]<<"; ";
   // }
 
   // Allocate in DEVICE memory
   float* d_xlens  ,*d_ylens  ,*d_eps    ,*d_lensim;
 
   cudaMalloc(&d_xlens, nlenses);
   cudaMalloc(&d_ylens, nlenses);
   cudaMalloc(&d_eps, nlenses);
   cudaMalloc(&d_lensim, sizeof(float)*lensim.ntotal);
   
   // Copy vectors from host to device memory
   cudaMemcpy(d_xlens, xlens, nlenses, cudaMemcpyHostToDevice);
   cudaMemcpy(d_xlens, ylens, nlenses, cudaMemcpyHostToDevice);
   cudaMemcpy(d_eps, eps, nlenses, cudaMemcpyHostToDevice);
   cudaMemcpy(d_lensim, lensim.buffer,sizeof(float)*lensim.ntotal, cudaMemcpyHostToDevice);
 
 
 
   clock_t tstart = clock();
 
   // Invoke kernel to draw the lensing image map here. 
   int threadsPerBlock = 256;
   int blocksPerGrid = lensim.ntotal / threadsPerBlock + 1;
 
//    int blocksPerGrid = (lensim.ntotal+ threadsPerBlock-1)/ threadsPerBlock ;

 
//    printf("222222");
 
   std::cout << "Launching a grid of " << blocksPerGrid
         << " "
         << threadsPerBlock * blocksPerGrid
         << " threads" << std::endl;
   cuda_shoot<<<blocksPerGrid, threadsPerBlock>>>(d_lensim, npixx, npixy, lens_scale, d_xlens, d_ylens, d_eps, nlenses);
   
   clock_t tend = clock();
   double tms = diffclock(tend, tstart);
   std::cout << "# Time elapsed: " << tms << " ms;--- " << tms/1000 << "s" << std::endl;
 
   //copy the lens image to host
   cudaMemcpy(lensim.buffer, d_lensim, sizeof(float)*lensim.ntotal, cudaMemcpyDeviceToHost);
   printf("33333333");
 
   cout << "lensim.ntotal: "<<lensim.ntotal <<endl;
   // for (int i = 0; i<lensim.ntotal; i ++ ){
   //   cout << lensim.buffer[i]<<"; ";
   // }
     cout <<"lensim test:"<< lensim(998,998) << endl ;
   // Write the lens image to a FITS formatted file. You can view this
   // image file using ds9
   dump_array<float, 2>(lensim, "lens_gpu.fit");
 
   cudaFree(d_xlens);
   cudaFree(d_ylens);
   cudaFree(d_eps);
   cudaFree(d_lensim);
 
   delete[] xlens;
   delete[] ylens;
   delete[] eps;
 }
 
 