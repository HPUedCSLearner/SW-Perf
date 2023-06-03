#include <stdio.h>
#include <limits.h>
#include <athread.h>

#include "stack.h"
#include "perf_counter.h"
#include "get_pid_time.h"

// #define OPEN_GET_BASIC_INFO_SWITH

// must pre-kown the depth is lower 20 
const char *indent[20] = {
    "",                                 // indent 0
    " ",
    "  ",
    "   ",
    "    ",
    "     ",
    "      ",
    "       ",
    "        ",
    "         ",
    "          ",                       // indent 10
    "           ",
    "            ",
    "             ",
    "              ",
    "               ",
    "                ",
    "                 ",
    "                  ",
    "                   "               // indent 19
};

typedef unsigned long long ULL;

ULL func_start_time = 0;
ULL func_end_time = 0;
ULL this_func_time = 0;

ULL depth = 0;
ULL max_call_depth = 0;
int depth_is_zero_num = 0;

int __profile__rank = INT_MAX;        //记录当前处在哪个rank中（多进程）
int __profile_common_size = INT_MAX;  // record  mpi size

ULL call_start_probe_times = 0;
ULL call_end_probe_times = 0;

Stack time_stk;
Stack func_addr_stk;


static FILE *fp_trace = NULL;

void __attribute__((constructor)) traceBegin(void);
void __attribute__((destructor)) traceEnd(void);
void __cyg_profile_func_enter(void *func, void *caller);
void __cyg_profile_func_exit(void *func, void *caller);

void get_basic_info();

const char* outfile_path = "./out/";
const char* outfile_postfix = "factrace.txt";
char outfilename[BUF_SIZE] = {0};

void __attribute__((constructor, no_instrument_function)) traceBegin(void) {

    get_file_name(outfilename, outfile_path, outfile_postfix);
    fp_trace = fopen(outfilename, "w");
    if(fp_trace == NULL) {
        printf("ERROR: Failed to open file %s\n", outfilename);
    }

    stack_init(&time_stk);
    stack_init(&func_addr_stk);
    printf("[basic info]: __traceBegin__\n");
}

void __attribute__((destructor, no_instrument_function)) traceEnd(void) {
#ifdef OPEN_GET_BASIC_INFO_SWITH
    if (__profile__rank == INT_MAX || __profile__rank == 0) {
        char outfilename[BUF_SIZE] = {0};
        get_file_name(outfilename, outfile_path, outfile_postfix);
    
        printf("*************************TraceEnd  Basic Info*************************\n");
        printf("[basic info]: depth_is_zero_num: \t%d\n", depth_is_zero_num);
        printf("[basic info]: max_call_depth: \t\t%lld\n", max_call_depth);
        printf("[basic info]: call_start_probe_times: \t%lld\n", call_start_probe_times);
        printf("[basic info]: call_end_probe_times: \t%lld\n", call_end_probe_times);
        printf("[basic info]: mpi rank: \t\t%d\n", __profile__rank);
        printf("[basic info]: mpi common size: \t\t%d\n", __profile_common_size);
        printf("[basic info]: sampling out file: \t%s\n", outfilename);
        printf("[basic info]: __traceEnd__\n");
        printf("***********************************************************************\n");
    }
#endif

    printf("[basic info]: sampling out file: \t%s\n", outfilename);
    printf("[basic info]: __traceEnd__\n");

    if (fp_trace != NULL) {
        fclose(fp_trace);
    }
}

void __cyg_profile_func_enter(void *func, void *caller) {
    // printf("come here\n");

#ifdef OPEN_GET_BASIC_INFO_SWITH
    get_basic_info();
#endif

    fprintf(fp_trace, "%s", indent[depth]);
    fprintf(fp_trace, "ENTER\t%p\tDepth\t%lld\n", func, depth);
    func_start_time = athread_time_cycle();
    stack_push(&time_stk, func_start_time);
    depth++;

#ifdef OPEN_GET_BASIC_INFO_SWITH
    call_start_probe_times++;

    if (depth > max_call_depth) {
        max_call_depth = depth;
    }
#endif

    // printf("entry %p %p\n", func, caller);
}


void __cyg_profile_func_exit(void *func, void *caller) {

    depth--;

    func_end_time = athread_time_cycle();
    this_func_time = func_end_time - get_stack_top_data(&time_stk);
    stack_pop(&time_stk);

    fprintf(fp_trace, "%s", indent[depth]);
    fprintf(fp_trace, "EXIT\t%p\tDepth\t%lld\tCostTime\t%lld\n", func, depth, this_func_time);

#ifdef OPEN_GET_BASIC_INFO_SWITH
    call_end_probe_times++;
#endif

    // printf("exit %p %p\n", func, caller);
}



void get_basic_info()
{
    if (depth == 0) {
        depth_is_zero_num++;
    }
    if (depth > max_call_depth) {
        max_call_depth = depth;
    }
}