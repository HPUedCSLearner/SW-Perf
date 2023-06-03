// #pragma once
#ifndef __PERF_COUNTER_H__
#define __PERF_COUNTER_H__

#include <athread.h>
 
uint64_t perf_counter();


uint64_t perf_counter()
{
    return athread_time_cycle();
}
#endif
