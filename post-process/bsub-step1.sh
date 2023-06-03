bsub -I -b -q q_sw_expr -n 1 -cgsp 64 -share_size 6500 -host_stack 1024   ./a.out1 $1  
