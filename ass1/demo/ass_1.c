#include "mpi.h"
#include <stdio.h>


#define MATRIX 65536
#define TWO_32  4294967296
#define RANDOM_SEED 10

#define Small_a  1664525
#define Small_c  1013904223


unsigned long AC_Table[40][2] = {
        {   1664525,     1013904223},
        { 389569705,     1196435762},
        {2940799637,     3519870697},
        { 158984081,     2868466484},
        {2862450781,     1649599747},
        {3211393721,     2670642822},
        {1851289957,     1476291629},
        {3934847009,     2748932008},
        {2184914861,     2180890343},
        { 246739401,     2498801434},
        {1948736821,     3421909937},
        {2941245873,     3167820124},
        {4195587069,     2636375307},
        {4088025561,     3801544430},
        { 980655621,       28987765},
        {2001863745,     2210837584},
        { 657792333,     3039689583},
        {  65284841,     1338634754},
        {1282409429,     1649346937},
        {3808694225,     2768872580},
        {2968195997,     2254235155},
        {2417331449,     2326606934},
        {2878627493,     1719328701},
        { 307989601,     1061592568},
        { 504219373,       53332215},
        {1897564169,     1140036074},
        {2574089845,     4224358465},
        {3294562801,     2629538988},
        {3478292285,     1946028059},
        {2651335705,      573775550},
        {2523738949,     1473591045},
        { 666245249,       95141024},
        {4137395341,     1592739711},
        {2604435753,     1618554578},
        {1706708245,     4257218569},
        {3963176977,     2685635028},
        {3678957277,     2617994019},
        {3530469177,      740185638},
        {3858799589,     4194465613},
        { 629287073,     2426187848},
};

/*
int in_circles_circle (n)unsigned long n;
{
    //calculate (x, y) for n
    double x, y;
    unsigned long ix, iy;
    ix = n % MATRIX;
    iy = n / MATRIX;
    x = ix*1.0 / SHORT_1_VALUE - 1;
    y = iy*1.0 / SHORT_1_VALUE - 1;
    return  (x*x + y*y <= 1) ? 1:0;
}
*/

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


unsigned long get_n_k(n, A, C)unsigned long n;unsigned long A;unsigned long C;
{
    return (A*n+C) % TWO_32;
}


unsigned long get_in_circle_amount(n0, count, A, C)unsigned long n0;unsigned long count;unsigned long A;unsigned long C;
{
    unsigned long n_circle_amount, n1,  i;
    if (count < 1 )
    {
        return 0L;
    }

    n_circle_amount = is_in_circle(n0);
    for(i=1; i<count; i++){
        n1 = get_n_k(n0, A, C);
        n_circle_amount += is_in_circle(n1);
        n0 = n1;
    }
    return n_circle_amount;
}

int main(argc,argv)int argc;char *argv[];
{
    int numproc, myid, namelen;
    unsigned long total, in_circles, n0, n1, i;
    unsigned long N, A, C, per_processor_tasks;
    double work_time_start, work_time, total_time_start, total_time;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Status Stat;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);

    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Get_processor_name(processor_name, &namelen);

    A = AC_Table[numproc-1][0];
    C = AC_Table[numproc-1][1];
//    N = atoi(argv[1]);
    N =100000000;


    per_processor_tasks = N / (numproc);

    total = 0;
    if (myid == 0) // if master
    {
        fprintf(stdout, "Total Processors: %d \n", numproc);
        fprintf(stdout, "Master Processor name: %s\n", processor_name);

        fprintf(stdout, "N=%ld, A=%ld, C=%ld\n", N, A, C);
        total_time_start = MPI_Wtime();
        n0 = RANDOM_SEED;
        // Master sends N to all the slave processes
        for  (i=1; i<numproc; i++)
        {
            n1 = get_n_k(n0, Small_a, Small_c); //k is 1
            MPI_Send(&n1, 1, MPI_LONG, i, 0, MPI_COMM_WORLD);
            n0 = n1;
        }

        // Master does its own task
        work_time_start = MPI_Wtime();
        total = get_in_circle_amount(RANDOM_SEED, per_processor_tasks, A, C);
        work_time = MPI_Wtime() - work_time_start;
        fprintf(stdout, "Master processor %ld in_circle amount:%ld ; total is %ld\n", myid, total, per_processor_tasks);
        fprintf(stdout, "Master processor  %ld  work time spend time: %f s\n", myid, work_time);

        //receive all slaves
        for (i=1;i<numproc;i++)
        {
            MPI_Recv(&in_circles, 1, MPI_LONG, i, 0, MPI_COMM_WORLD, &Stat);
            total += in_circles;
        }
        total_time = MPI_Wtime() - total_time_start;
        fprintf(stdout, "------------\n");
        fprintf(stdout,"Master total time:   %f \n", total_time );

        //result
        fprintf(stdout,"\n");
        fprintf(stdout,"The %ld of %ld points falls into the circle\n", total, N);
        fprintf(stdout,"\n");

        fprintf(stdout,"***** PI is %f\n", total*4.0/N);
        fprintf(stdout,"\n");
        fprintf(stdout,"############################# \n");


    }else{
        /*slaves*/
        total_time_start = MPI_Wtime();
        MPI_Recv(&n0, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD, &Stat);
        in_circles = get_in_circle_amount(n0, per_processor_tasks, A, C);
        MPI_Send(&in_circles, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD);
        total_time = MPI_Wtime() - total_time_start;
        fprintf(stdout, "\n");
        fprintf(stdout, "Slave processor %ld in_circle amount:%ld; total is %ld\n", myid, in_circles, per_processor_tasks);

//        fprintf(stdout, "Slave processor %ld work spend time: %f s\n", myid, work_time);
        fprintf(stdout, "Slave processor %ld total spend time: %f s\n", myid, total_time);

        fprintf(stdout, "\n");
    }

    MPI_Finalize();
}
