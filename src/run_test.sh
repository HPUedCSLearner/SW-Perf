mpicc -finstrument-functions -g -c test.c
mpicc test.o mpiwrapper.o timer.o -o a.out
bsub -I -b -q q_sw_expr -n 1  -cgsp 64 -share_size 4096 -host_stack 128 ./a.out
