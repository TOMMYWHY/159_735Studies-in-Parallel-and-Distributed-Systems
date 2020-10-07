 
 mac:

```shell 

mpic++ ass_2.cpp -o out
 mpirun -n 4 out 16

```

mighty:

```shell
mpic++ ass_2.cpp -o sort
qsub sort.pbs

```
