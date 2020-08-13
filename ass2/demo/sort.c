#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
/********************************************************************/

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

int compare(const void* x1, const void* x2) {
  const float* f1 = x1;
  const float* f2 = x2;
  float diff = *f1 - *f2;

  return (diff < 0) ? -1 : 1;
}

/********************************************************************/
int main(argc, argv)int argc; char* argv[];
{
  int numproc, myid, N, i;
  float *dSend, *dRecv;
  const float xmin = 1.0;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numproc);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  
  N = atoi(argv[1]);
//  const float xmax = N * 10; //the range is big enough //todo
  const float xmax = N ; //the range is big enough //todo
  int nrecv = N/numproc; // pre_proc_num
  dSend = (float*)malloc(N*sizeof(float)); // N_container
  dRecv = (float*)malloc(nrecv*sizeof(float)); // pre_proc_get_count_container

  //the master processor generates N*numproc random numbers and makes sure they are not sorted.
  if (myid == 0)
  {
    fprintf(stdout, "Generating %d numbers to be sorted on %d processors\n", N, numproc);
      printf("each proc get %d \n",nrecv);

      for (i=0; i<N; i++)
    {
      dSend[i] = drand48()*(xmax-xmin-1)+xmin;
    }          
  }

  double total_s = MPI_Wtime();
  //scatter the data to each processors
  //fprintf(stdout, "It's processor %d\n\n", myid);
  MPI_Scatter(dSend, nrecv, MPI_FLOAT, dRecv, nrecv, MPI_FLOAT, 0, MPI_COMM_WORLD);
  //void*  dSend:存储在0号进程的数据，array ；即 全部 N
  //int nrecv:具体需要给每个进程发送的数据的个数 10
  //void*  dRecv:接收缓存，缓存 recv_count个数据

  for (i=0; i<N/numproc; i++)
  {
    fprintf(stdout, "%f ", dRecv[i]); // 每个processor收到的数据
  }
  fprintf(stdout, "\n");
  fprintf(stdout, "\n");

  //create numproc buckets and put the numbers into correct buckets, memory size is numproc*nrecv
  double bucketing_s = MPI_Wtime();
  int nbuckets = numproc;
  float* bucket = calloc(nbuckets*nrecv, sizeof(float)); // 定义一个大桶，其中，有4个小桶
  int* nitems = calloc(nbuckets, sizeof(int)); //the number of items thrown into each bucket 每个小桶中
  float step = (xmax-xmin)/nbuckets;
  for (i=0; i<N/numproc; i++)
  {
    int bktno = (int)((dRecv[i] - xmin)/step); //dRecv[i] 所在桶的编号
    int index = bktno * nrecv + nitems[bktno];//todo
      printf("DATA %d %f %d %d\n", i, dRecv[i], bktno, index);

      bucket[index] = dRecv[i];
    ++nitems[bktno];
  }
  double bucketing_t = MPI_Wtime() - bucketing_s; // partitioning bucket

  //now empty the small buckets to large buckets
  //first we need to let each processor know how many number it needs to receive
  int* recvCount = (int*)calloc(nbuckets, sizeof(int)); //buckets
  MPI_Alltoall(nitems, 1, MPI_INT, recvCount, 1, MPI_INT, MPI_COMM_WORLD);

  //send and receive displacement //todo
  int* sdispls = (int*)calloc(nbuckets, sizeof(int)); 
  int* rdispls = (int*)calloc(nbuckets, sizeof(int)); 
  for (i=1; i<nbuckets; i++){
    sdispls[i] = i*nrecv;
//    printf("sdispls[%d] %d \n", i, sdispls[i]);

    rdispls[i] = rdispls[i-1]+recvCount[i-1];
//    printf("rdispls[%d] %d \n", i, rdispls[i]);

  }

  float* big_bucket = calloc(N, sizeof(float));// void *recvbuf,
  MPI_Alltoallv(bucket, nitems, sdispls, MPI_FLOAT, big_bucket, recvCount, rdispls, MPI_FLOAT, MPI_COMM_WORLD);
// nitems : const int *sendcounts,
//recvCount: const int *recvcounts,

  int  totalCount = 0;
  for (i=0; i<nbuckets; i++){
//      printf("recvCount[%d] %d \n", i, recvCount[i]);
      totalCount += recvCount[i];
  }
  double sorting_s = MPI_Wtime();
  //now we have all data in the big bucket, sort  
  qsort(big_bucket, totalCount, sizeof(float), compare);
  double sorting_t = MPI_Wtime()-sorting_s;

  //gather the result
  memset(recvCount, 0, nbuckets*sizeof(int));
  MPI_Gather(&totalCount, 1, MPI_INT, recvCount, 1, MPI_INT,0, MPI_COMM_WORLD);
  rdispls[0] = 0;
  for (i=1; i<nbuckets; i++){
      rdispls[i] = rdispls[i-1] +  recvCount[i-1];
  }

  MPI_Gatherv(big_bucket, totalCount, MPI_FLOAT, dSend, recvCount, rdispls, MPI_FLOAT, 0, MPI_COMM_WORLD);
    printf("------------------ \n");

    if (myid == 0 && is_sorted(dSend, N)){
      fprintf(stdout, "total time: %f, parallel: %f\n", MPI_Wtime()-total_s, bucketing_t+sorting_t);
      fprintf(stdout, "The data array is sorted, from %f to %f\n", dSend[0], dSend[N-1]);
      for(i=0;i<N;i++){
          printf("%f, ",dSend[i]);
      }
      printf("\n");

  }
    printf("------------------ \n");

    MPI_Finalize();
  return 0;
}
