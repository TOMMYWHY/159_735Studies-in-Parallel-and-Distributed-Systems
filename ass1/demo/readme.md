mpicc first.c -o first
mpirun -n 4 first
emacs first.pbs &
qsub first.pbs