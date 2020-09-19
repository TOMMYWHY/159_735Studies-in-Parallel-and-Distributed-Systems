#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "mpi.h"
#include <fstream>
#include <iomanip>


#define MATRIX 65536
#define TWO_32  4294967296
#define RANDOM_SEED 11111

#define Small_a  1664525
#define Small_c  1013904223

using  namespace std;
unsigned long A_C[40][2] = {
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

unsigned long get_in_circle_amount(unsigned long n0, unsigned long count, unsigned long A, unsigned long C);
int is_in_circle(unsigned long n);
unsigned long get_n_k(unsigned long n, unsigned long A, unsigned long C);

void test_comm_time(int myid, int numproc);

int main(int argc, char *argv[]){
    int numproc, myid, namelen;
    unsigned long total, in_circles, n0, n1, i;
    unsigned long N, A, C, per_processor_tasks;
    double  work_time_start, work_time, total_time_start, total_time;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Status Stat;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Get_processor_name(processor_name, &namelen);

    A = A_C[numproc-1][0];
    C = A_C[numproc-1][1];
//    N = atoi(argv[1]);
    N =300000000;

/*    if (N == 0)
    {
        test_comm_time(myid,numproc);
        MPI_Finalize();
        return 0;
    }*/




    per_processor_tasks = N / (numproc);
    total = 0;
    if (myid == 0) // if master
    {
        cout << "Total Processors: " << numproc<<endl;
        cout << "Master Processor name:: " << processor_name<<endl;

        total_time_start = MPI_Wtime();
        n0 = RANDOM_SEED;
        // Master sends N to all the slave processes
        for  (i=1; i<numproc; i++)
        {
            n1 = get_n_k(n0, Small_a, Small_c); //k is 1
            MPI_Send(&n1, 1, MPI_LONG, i, 0, MPI_COMM_WORLD);
            cout <<"send "<<n1 <<" to slave "<< i <<endl;
            n0 = n1;
        }

        // Master does its own task
        work_time_start = MPI_Wtime();
        total = get_in_circle_amount(RANDOM_SEED, per_processor_tasks, A, C);
        work_time = MPI_Wtime() - work_time_start;
        cout <<"Master processor "<<myid <<": in_circle amount "<< total << "; total is "<< per_processor_tasks <<endl;
        cout <<"Master processor "<<myid <<" work time spend time: "<< work_time << " s " <<endl;

        //receive all slaves
        for (i=1;i<numproc;i++)
        {
            MPI_Recv(&in_circles, 1, MPI_LONG, i, 0, MPI_COMM_WORLD, &Stat);
            total += in_circles;
        }
        total_time = MPI_Wtime() - total_time_start;
        cout << "----------------------------"<<endl;

        cout <<"Master total time:  "<<total_time <<  " s " <<endl;
        cout << "Communication time: " <<total_time-work_time << "s"<< endl;
        //result
        cout <<"The "<<total <<" of "<< N << " points falls into the circle " <<endl;
        cout <<endl;
        long double pi =total*4.0/N;
        cout<<"***** PI is "<<fixed<<setprecision(8)<<pi<<endl;

        cout << "#############################################"<<endl;



    }else{
        /*slaves*/
        total_time_start = MPI_Wtime();
        MPI_Recv(&n0, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD, &Stat);
        in_circles = get_in_circle_amount(n0, per_processor_tasks, A, C);
        MPI_Send(&in_circles, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD);
        total_time = MPI_Wtime() - total_time_start;

        cout <<"Slave processor "<<myid <<"  in_circle amount:  "<< in_circles << " total is: " << per_processor_tasks  <<endl;
        cout <<"Slave processor "<<myid <<" total spend time:: "<< total_time << " s " <<endl;
        cout <<endl;
    }

    MPI_Finalize();
}

unsigned long get_in_circle_amount(unsigned long n0, unsigned long count, unsigned long A, unsigned long C){
    unsigned long n_circle_amount, n1,  i;
    n_circle_amount = is_in_circle(n0);
    for(i=1; i<count; i++){
        n1 = get_n_k(n0, A, C);
        n_circle_amount += is_in_circle(n1);
        n0 = n1;
    }
    return n_circle_amount;
}

int is_in_circle(unsigned long n){
    //using n to calculate (x, y)
    double x, y;
//    y = ceil(n / MATRIX ); // 0-Matrix
    y = (n / MATRIX ); // 0-Matrix
//    x = n - ( y * MATRIX );
    x = n%( MATRIX );
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

unsigned long get_n_k(unsigned long n, unsigned long A, unsigned long C){
    return (A*n+C) % TWO_32;
}

/*
void test_comm_time(int myid, int numproc){
    int i;

    unsigned long data_send, data_recv;
    double time_start, time_spend;
    MPI_Status Stat;
    if (myid == 0)
    {
        data_send = 4294967296;

        for ( i=1; i<numproc; i++)
        {
            time_start = MPI_Wtime();
            MPI_Send(&data_send, 1, MPI_LONG, i, 0, MPI_COMM_WORLD);
            MPI_Recv(&data_recv, 1, MPI_LONG, i, 0, MPI_COMM_WORLD, &Stat);
            time_spend = MPI_Wtime() - time_start;
            cout <<"Master 0 spends "<< time_spend <<" to communicate with salve:" << i <<endl;
        }
    }
    else
    {
        time_start = MPI_Wtime();
        MPI_Recv(&data_recv, 1, MPI_LONG, i, 0, MPI_COMM_WORLD, &Stat);
        MPI_Send(&data_recv, 1, MPI_LONG, i, 0, MPI_COMM_WORLD);
        time_spend = MPI_Wtime() - time_start;
    }

}*/
