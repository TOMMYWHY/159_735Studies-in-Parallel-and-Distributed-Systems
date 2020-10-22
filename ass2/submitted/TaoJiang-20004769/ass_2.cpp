#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;


int is_sorted(float *data, int size);
int compare(const void* x1, const void* x2);

int main(int argc, char* argv[]){
  int numproc, myid, N, i;
  const float xmin = 1.0;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numproc);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

//  N = atoi(argv[1]);
//    N = 16;
    N = numproc*1000000;



  const float xmax = 100000 ; //the range is big enough
  int pre_proc_recv_amount = N/numproc; // pre_proc_num
  float* send_all_data = new float [N]();
  float* recv_proc_data = new float [pre_proc_recv_amount]();

  if (myid == 0)
  {
    cout << "Generate " <<N <<" numbers to be sorted on " << numproc<<" processors." <<endl;
    cout << "Each processor get " <<pre_proc_recv_amount << " numbers."<<endl;
//    cout <<"unsorted numbers: " ;
      for (i=0; i<N; i++)
    {
      send_all_data[i] = drand48()*(xmax-xmin-1)+xmin; //generates N*numproc random numbers
//        cout <<send_all_data[i] <<" , ";
    }
//      cout << "\n"<<endl;
      cout << "-------*******---------"<< endl;
  }

  double start_time = MPI_Wtime();
  //scatter the data to each processors
  MPI_Scatter(send_all_data, pre_proc_recv_amount, MPI_FLOAT, recv_proc_data, pre_proc_recv_amount, MPI_FLOAT, 0, MPI_COMM_WORLD);
  //void* send_all_data: 存储在0号进程的数据，array ；即 全部 N
  //int pre_proc_recv_amount:具体需要给每个进程发送的数据的个数 10
  //void*  recv_proc_data:接收缓存，缓存 recv_count个数据

/*  for (i=0; i<pre_proc_recv_amount; i++)
  {
    cout <<recv_proc_data[i] <<",";// 每个processor收到的数据
  }
    cout<<endl;*/

  double bucket_start_time = MPI_Wtime();
  int nbuckets = numproc;
      float* bucket=new float[nbuckets*pre_proc_recv_amount]();//  定义一个大桶，其中，有4个小桶
      int * nitems = new int [nbuckets]();//the number of items thrown into each bucket 每个小桶中
  float step = (xmax-xmin)/nbuckets;
  for (i=0; i<N/numproc; i++)
  {
    int bktno = (int)((recv_proc_data[i] - xmin)/step); //recv_proc_data[i] 所在桶的编号
    int index = bktno * pre_proc_recv_amount + nitems[bktno];
//      cout << i <<",  num: " << recv_proc_data[i]<<" , buckets num:"<<bktno<<" , index:" <<index <<endl;
      bucket[index] = recv_proc_data[i];
    nitems[bktno]++;
  }
  double bucket_end_time = MPI_Wtime() - bucket_start_time; // partitioning bucket
  int * recvCount = new int [nbuckets]();// = (int*)calloc(nbuckets, sizeof(int)); //buckets
  MPI_Alltoall(nitems, 1, MPI_INT, recvCount, 1, MPI_INT, MPI_COMM_WORLD);
  int* sdispls = new int [nbuckets]();
  int* rdispls = new int [nbuckets]();
  for (i=1; i<nbuckets; i++){
    sdispls[i] = i*pre_proc_recv_amount;
//    cout << "sdispls"<<i<<" "<<sdispls[i] <<" ";
    rdispls[i] = rdispls[i-1]+recvCount[i-1];
//    cout << "rdispls"<<i<<" "<<rdispls[i] <<" ; ";

  }
  float* big_bucket = new float[N]();// void *recvbuf,
  MPI_Alltoallv(bucket, nitems, sdispls, MPI_FLOAT, big_bucket, recvCount, rdispls, MPI_FLOAT, MPI_COMM_WORLD);
// nitems : const int *sendcounts,
//recvCount: const int *recvcounts,

  int  totalCount = 0;
  for (i=0; i<nbuckets; i++){
//    cout <<"recvCount:"<<i << " num:"<<recvCount[i] <<endl;
      totalCount += recvCount[i];
  }
  double sort_start_time = MPI_Wtime();
  //now we have all data in the big bucket, sort
  qsort(big_bucket, totalCount, sizeof(float), compare);
  double sort_end_time = MPI_Wtime()-sort_start_time;
  memset(recvCount, 0, nbuckets*sizeof(int));
  MPI_Gather(&totalCount, 1, MPI_INT, recvCount, 1, MPI_INT,0, MPI_COMM_WORLD);
  rdispls[0] = 0;
  for (i=1; i<nbuckets; i++){
      rdispls[i] = rdispls[i-1] +  recvCount[i-1];
  }

  MPI_Gatherv(big_bucket, totalCount, MPI_FLOAT, send_all_data, recvCount, rdispls, MPI_FLOAT, 0, MPI_COMM_WORLD);
    if (myid == 0 ){
        if( is_sorted(send_all_data, N)){
            cout << "Generate " <<N <<" numbers to be sorted on " << numproc<<" processors." <<endl;
            cout << "Each processor get " <<pre_proc_recv_amount << " numbers."<<endl;
            double total_time = MPI_Wtime()-start_time;
            double parallel_time =  bucket_end_time+sort_end_time;
            double serial_time = total_time - parallel_time;
            cout << "Sort total time :" << total_time<<" s"<< endl;
            cout << "serial time :" << serial_time<<" s"<< endl;
            cout << "Parallel partition time:"<< parallel_time<<" s"<<endl;

            /*    cout<<"sorted result:" <<endl;
                for(i=0;i<N;i++){
                    cout << send_all_data[i] << " , ";
                }
                cout <<endl;
        */
            cout<< "Total sort numbers: "<< N << ", " << " and the data sorted from"<<send_all_data[0] << " to " << send_all_data[N-1]<<endl;

        }else{
            cout <<"sorted fail...."<<endl;
        }


  }
    cout<<"=========================================="<<endl;

    MPI_Finalize();
  return 0;
}
/********************************************************************/

int compare(const void* x1, const void* x2) {
    const float* f1 = (float *)x1;
    const float* f2 =(float *) x2;
    float diff = *f1 - *f2;

    return (diff < 0) ? -1 : 1;
}

int is_sorted(float *data, int size)
{
    int i;
    for (i=0; i<size; i++){
        if ( data[i] < data[i-1]){
            return 0;
        }
    }
    return 1;
}

