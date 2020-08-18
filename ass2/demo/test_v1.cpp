//
// Created by Tommy on 2020/8/13.
//
//#include "mpi.h"
//#include <stdio.h>
//#include <stdlib.h>
#include <iostream>
using namespace std;

int main(){
//    int numproc, myid, N, i;
//    MPI_Init(&argc, &argv);
//    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
//    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

/*
//    N = atoi(argv[1]);
    N = 20;

    const float min_n = 1.0;
    const float max_n = N; //the range is big enough //todo
//    const float max_n = N*N;
    int pre_proc_num = N/numproc; // pre_proc_num
//    float *send_data, *recv_data;
    float * send_all_data = (float*)malloc(N*sizeof(float)); // all_data
    float * recv_data_proc = (float*)malloc(pre_proc_num*sizeof(float)); // pre_proc_get_data

    *//*generate data*//*
    cout <<"send_all_data:"  <<endl;

    for (int i = 0; i <N; i++) {
        send_all_data[i] = drand48() * (max_n - min_n - 1) + min_n;
        cout << send_all_data[i] <<"," ;
    }
    cout << endl;*/


    cout << "testing"<<endl;
    return 0;
}