#include "mpi.h"
#include <stdio.h>
#include <math.h>

//2**15
#define SHORT_1_VALUE 32768
// 2**16
#define SHORT_VALUE 65536


int hit_circle (n)unsigned long n;
{
    //calculate (x, y) for n
    double x, y;
    unsigned long ix, iy;
    ix = n % SHORT_VALUE;
    iy = n / SHORT_VALUE;
    x = ix*1.0 / SHORT_1_VALUE - 1;
    y = iy*1.0 / SHORT_1_VALUE - 1;
    return  (x*x + y*y <= 1) ? 1:0;
}

int main(argc,argv)int argc;char *argv[];
{
    int numproc, myid, namelen;
    unsigned long total, hit, n0, n1, i;
    unsigned long N, A, C,two_32, per_processor_tasks;
    double  total_time_start, total_time;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Status Stat;

    MPI_Init(&argc,&argv);//INITIALIZE
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);

    // todo  leap-frog need more numproc???
    if ( numproc <=1  ) {
        fprintf(stdout, " Need more Processors ~! \n" );
        MPI_Finalize();
        return 0;
    }
    if ( numproc > 32 ) {
        fprintf(stdout, " Need Processors %d is too much~! \n", numproc);
        MPI_Finalize();
        return 0;
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Get_processor_name(processor_name, &namelen);

      A = 1664525;
      C = 1013904223;
      two_32 = 4294967296;
        N = atoi(argv[1]);
//      N =10000000;

    per_processor_tasks = N / (numproc-1);
    total = 0;
    if (myid == 0) // if master
    {
        fprintf(stdout, "Total Processors: %d \n", numproc);
        fprintf(stdout, "Master Processor name: %s\n", processor_name);
        total_time_start = MPI_Wtime();
        n0 = 1;//seed
        for  (i=1; i<N; i++)
        {
            n1 = (A*n0+C) % two_32;
//            fprintf(stdout, "***** %d\n",n0);
            fprintf(stdout, "* send number %d to slave %d \n",n1,i%(numpro+1));

            MPI_Send(&n1, 1, MPI_LONG, N%(numproc-1), 0, MPI_COMM_WORLD);
            n0 = n1;
        }
        fprintf(stdout, "master send done \n");

        //receive all slaves
        for (i=1;i<N;i++)
        {
            MPI_Recv(&hit, 1, MPI_LONG, i, 0, MPI_COMM_WORLD, &Stat);
            total += hit;
        }
        total_time = MPI_Wtime() - total_time_start;
        fprintf(stdout, "------------\n");
        fprintf(stdout,"Master total time:  %f s\n", total_time);

        //result
        fprintf(stdout,"\n");
        fprintf(stdout,"Total points are %ld. There are %ld points in the circle.  \n", N, total);
        fprintf(stdout,"PI result is %f\n", total*4.0/N);
        fprintf(stdout,"====================================\n");

    }
    else
    {
        /*slaves*/
        MPI_Recv(&n0, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD, &Stat);
        fprintf(stdout, "***** slave %ld receive n %d\n",myid,n0);

//        hit = hit_circle(n0);
//        /*for(i=1; i<per_processor_tasks; i++){
//            n1 = (A*n1+C) % two_32;
//            hit += hit_circle(n1);
//            n0 = n1;
//        }*/
//
//
//        MPI_Send(&hit, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD);
//        fprintf(stdout, "------------\n");
//        fprintf(stdout, "Slave processor %ld : hit/total is %ld/%ld\n", myid, hit, per_processor_tasks);
////        fprintf(stdout, "------------\n");
    }

    MPI_Finalize();
}
