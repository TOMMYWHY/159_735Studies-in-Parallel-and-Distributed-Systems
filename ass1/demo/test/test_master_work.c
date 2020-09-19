/*
qsub: error while loading shared libraries: libstdc++.so.6: wrong ELF class: ELFCLASS32

*/


#include "mpi.h"
#include <stdio.h>
#include <math.h>

#define MATRIX 65536
#define A  1664525
#define C  1013904223
#define TWO_32  4294967296


int is_in_circle (n)unsigned long n;
{
    //using n to calculate (x, y)  
    double x, y;
    y = ceil(n / MATRIX ); // 0-Matrix
    x = n - ( y * MATRIX );
    x = x /MATRIX -0.5; // 0-1 => -0.5 - 0.5  => R = 0.5
    y = y /MATRIX -0.5;
    // R**R = 0.25
    if(x*x + y*y <= 0.25){
        return 1;
    }else
    {
        return 0;
    }
}

long int generator (n0,k)unsigned long int n0;int k;
{
    // AA
    long int AA,CC, n1;
    // k = myid;
    AA=1; // pow(a,k)%m
    for (int e = 0; e < k; e++)
    {
        AA=(AA*A)%TWO_32;
    }

    //CC

    int a_k,a_0, a_k_ms; //pow(a,k); pow(a,0); Sum( pow(a,k)mod m );
        a_0=A;
        for (int j = 0; j < k; j++)
        {
            a_k=(a_k*a_0)%TWO_32; // pow(a,k)
            a_k_ms+=a_k%TWO_32;  // Sum( pow(a,k)mod m )
            a_0=a_k;
        }
    CC = (a_k_ms%TWO_32 *C)%TWO_32;
    n1 = (AA*n0+CC) % TWO_32; // 
    return n1;
}


int main(argc,argv)int argc;char *argv[];
{
    int numproc, myid, namelen;
    unsigned long total, hit,in_circles, n0, n1, i;
    unsigned long N, per_processor_tasks;
    double  total_time_start, total_time;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Status Stat;

    MPI_Init(&argc,&argv);//INITIALIZE
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);

    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Get_processor_name(processor_name, &namelen);

    // A = 1664525;
    // C = 1013904223;
    // two_32 = 4294967296;
    //    N = atoi(argv[1]);
    N =100000000;

    // per_processor_tasks = N / (numproc-1);
    per_processor_tasks = N / (numproc);
    total = 0;
    if (myid == 0) //  
    {
        /*master*/
        fprintf(stdout, "testing: %d \n", (MATRIX/2));
        fprintf(stdout, "Total Processors: %d \n", numproc);

        fprintf(stdout, "Master Processor name: %s\n", processor_name);
        total_time_start = MPI_Wtime();
        n0 = 2;
        // Master sends N to all the slave processes
        for  (i=1; i<numproc; i++)
        {
            n1 = (A*n0+C) % TWO_32;
            // fprintf(stdout, "***** %d\n",n0);
            fprintf(stdout, "send %ld to slave %d\n",n1,i);
            MPI_Send(&n1, 1, MPI_LONG, i, 0, MPI_COMM_WORLD);
            n0 = n1;
        }
        int n_m0=1,n_m1;
        fprintf(stdout, " n0: %d\n",n_m0);
        n_m1 = (A*n_m0+C) % TWO_32;
        
        fprintf(stdout, "Master processor get n0: %d\n",n_m1);
        total_time_start = MPI_Wtime();
        in_circles = is_in_circle(n_m1);
        for(i=1; i<per_processor_tasks; i++){
            // n1 = (AA*n0+CC) % TWO_32; // TODO
            n1 = generator(n_m1,numproc-1);
            in_circles += is_in_circle(n1);
            n_m1 = n1;
        }
        total = in_circles;
        total_time = MPI_Wtime() - total_time_start;
        fprintf(stdout, "Master processor %ld : in_circle amount:%ld ; total is %ld\n", myid, in_circles, per_processor_tasks);
        fprintf(stdout, "Master processor %ld spend time: %f s\n", myid, total_time);



        //receive all slaves
        for (i=1;i<numproc;i++)
        {
            MPI_Recv(&in_circles, 1, MPI_LONG, i, 0, MPI_COMM_WORLD, &Stat);
            total += in_circles;
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
        total_time_start = MPI_Wtime();
        MPI_Recv(&n0, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD, &Stat);
        in_circles = is_in_circle(n0);

        for(i=1; i<per_processor_tasks; i++){
            // n1 = (AA*n0+CC) % TWO_32; // TODO
            n1 = generator(n0,numproc-1);
            in_circles += is_in_circle(n1);
            n0 = n1;
        }
        MPI_Send(&in_circles, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD);
        total_time = MPI_Wtime() - total_time_start;
        fprintf(stdout, "------------\n");
        fprintf(stdout, "Slave processor %ld : in_circle amount:%ld ; total is %ld\n", myid, in_circles, per_processor_tasks);
        fprintf(stdout, "Slave processor %ld spend time: %f s\n", myid, total_time);
//        fprintf(stdout, "------------\n");
    }

    MPI_Finalize();
}