rm -rf timer.o mpiwrapper.o wrapper_f.o
mpicc timer.c -c
mpicc mpiwrapper.c -c -finstrument-functions -g 
mpicc wrapper_f.c -c -finstrument-functions -g 
ar -r libperf.a timer.o  mpiwrapper.o wrapper_f.o 
