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
 #include <fstream>
 
 using namespace std;
 const float WL  = 2.0;
 const float XL1 = -WL;
 const float XL2 =  WL;
 const float YL1 = -WL;
 const float YL2 =  WL;
 
 // kernel
 __global__ void cuda_shoot(float* lensim, int xsize, int ysize, float lens_scale, float* xlens, float* ylens, float* eps, int nlenses)
 {
   const float xsrc = 0.0;      
   const float ysrc = 0.0;
   const float rsrc = 0.1;     
   const float rsrc2 = rsrc * rsrc;
   const float ldc  = 0.5;    
 
   int index = threadIdx.x + blockIdx.x * blockDim.x;
   if (index >= xsize*ysize)
     return;
 
   int iy = index / xsize;
   int ix = index % xsize;
 
   float yl = -2.0 + iy * lens_scale;  
   float xl = -2.0 + ix * lens_scale;  
   float xs = xl;
   float ys = yl;
 
   for (int p = 0; p < nlenses; ++p) {
     float dx = xl - xlens[p];
     float dy = yl - ylens[p];
     float dr = dx * dx + dy * dy;
     xs -= eps[p] * dx / dr;
     ys -= eps[p] * dy / dr;
   }
 
   float xd = xs - xsrc;
   float yd = ys - ysrc;
   float sep2 = xd * xd + yd * yd;
   if (sep2 < rsrc2) {
     float mu = sqrt(1 - sep2 / rsrc2);
     lensim[index] = 1.0 - ldc * (1 - mu);
   }
 }

 double diffclock(clock_t clock1,clock_t clock2)
 {
   double diffticks = clock1 - clock2;
   double diffms = (diffticks * 1000) / CLOCKS_PER_SEC;
   return diffms; // Time difference in milliseconds
 }
 
 double seq(int len_num);

 int main(int argc, char* argv[]) 
 {
     const int len_num = atoi(argv[1]);

    //  cout << "-------------------------"<<endl;
     float* xlens;
     float* ylens;
     float* eps;
 //    const int nlenses = set_example_3(&xlens, &ylens, &eps);
     const int nlenses = set_example_n(len_num, &xlens, &ylens, &eps);
 
    //  std::cout << "# Simulating " << nlenses << " lens system" << std::endl;
 
   const float lens_scale = 0.0025;
   const int npixx = static_cast<int>(floor((XL2 - XL1) / lens_scale)) + 1;
   const int npixy = static_cast<int>(floor((YL2 - YL1) / lens_scale)) + 1;
  //  std::cout << "# Building " << npixx << "X" << npixy << " lens image" << std::endl;
 
   Array<float, 2> lensim(npixy, npixx);
  //  cout << "lensim.ntotal: "<<lensim.ntotal <<endl;
   //copy data to GPU space

   float* d_xlens  ,*d_ylens  ,*d_eps    ,*d_lensim;

   cudaMalloc(&d_xlens, nlenses*sizeof(float));
   cudaMalloc(&d_ylens, nlenses*sizeof(float));
   cudaMalloc(&d_eps, nlenses*sizeof(float));
   cudaMalloc(&d_lensim, lensim.ntotal*sizeof(float));
   
  //  // Copy vectors from host to device memory
   cudaMemcpy(d_xlens, xlens, nlenses*sizeof(float), cudaMemcpyHostToDevice);
   cudaMemcpy(d_ylens, ylens, nlenses*sizeof(float), cudaMemcpyHostToDevice);
   cudaMemcpy(d_eps, eps, nlenses*sizeof(float), cudaMemcpyHostToDevice);
   cudaMemcpy(d_lensim, lensim.buffer,lensim.ntotal*sizeof(float), cudaMemcpyHostToDevice);
 
     
 
   clock_t tstart = clock();
 
   int threadsPerBlock = 256;
  int blocksPerGrid = lensim.ntotal / threadsPerBlock + 1;
 
  //  int blocksPerGrid = (lensim.ntotal+ threadsPerBlock-1)/ threadsPerBlock ;
   std::cout << "Launching a grid of " << blocksPerGrid
         << " "
         << threadsPerBlock * blocksPerGrid
         << " threads" << std::endl;
   cuda_shoot<<<blocksPerGrid, threadsPerBlock>>>(d_lensim, npixx, npixy, lens_scale, d_xlens, d_ylens, d_eps, nlenses);
   
   clock_t tend = clock();
   double tms = diffclock(tend, tstart);
   std::cout << "# Time cost: " << tms << " ms;--- " << tms/1000 << "s" << std::endl;
 
   //copy the lens image to host
   cudaMemcpy(lensim.buffer, d_lensim, sizeof(float)*lensim.ntotal, cudaMemcpyDeviceToHost);
 
   dump_array<float, 2>(lensim, "lens_gpu.fit");
  /*----------------------*/
   double seq_cost= seq( len_num);


   ofstream file("report.txt",ios::app);
    if (file.is_open()){
        file <<"nlenses:"<< to_string(nlenses)<<
        "; seq:"<<to_string(seq_cost);
        file << "; cuda use " <<to_string(tms) << "\n";
        file.close();
    }
 
   cudaFree(d_xlens);
   cudaFree(d_ylens);
   cudaFree(d_eps);
   cudaFree(d_lensim);
 
   delete[] xlens;
   delete[] ylens;
   delete[] eps;
 }
 
/***************************************************/
 double seq(int len_num){
  float* xlens;
  float* ylens;
  float* eps;
  const int nlenses = set_example_n(len_num, &xlens, &ylens, &eps);
  std::cout << "# Simulating " << nlenses << " lens system" << std::endl;

  const float rsrc = 0.1;      // radius
  const float ldc  = 0.5;      // limb darkening coefficient
  const float xsrc = 0.0;      // x and y centre on the map
  const float ysrc = 0.0;

  const float lens_scale = 0.0025;

  const int npixx = static_cast<int>(floor((XL2 - XL1) / lens_scale)) + 1;
  const int npixy = static_cast<int>(floor((YL2 - YL1) / lens_scale)) + 1;
  Array<float, 2> lensim(npixy, npixx);

  clock_t tstart = clock();

  const float rsrc2 = rsrc * rsrc;
  float xl, yl, xs, ys, sep2, mu;
  float xd, yd;
  int numuse = 0;
  for (int iy = 0; iy < npixy; ++iy)
      for (int ix = 0; ix < npixx; ++ix) {
          yl = YL1 + iy * lens_scale;
          xl = XL1 + ix * lens_scale;

          shoot(xs, ys, xl, yl, xlens, ylens, eps, nlenses);

          xd = xs - xsrc;
          yd = ys - ysrc;
          sep2 = xd * xd + yd * yd;
          if (sep2 < rsrc2) {
              mu = sqrt(1 - sep2 / rsrc2);
              lensim(iy, ix) = 1.0 - ldc * (1 - mu);
          }
      }

  clock_t tend = clock();
  double tms = diffclock(tend, tstart);
  std::cout << "# seq Time cost: " << tms << " ms;--- " << tms/1000 << "s" << std::endl;

  // dump_array<float, 2>(lensim, "lens_seq.fit");
  
  return tms;
  
  delete[] xlens;
  delete[] ylens;
  delete[] eps;
 }
 
