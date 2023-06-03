#include<stdio.h>

void __wrap_zgemm_( void* a, void* b, void* c, void * d,
                    void* e, void* f, void* g, void * h,
                    void* i, void* j, void* k, void * l,
                    void* m)
{
    __real_zgemm_(a, b, c, d, e, f, g, h, i, j, k, l, m);
}

void __wrap_zcopy_( void* a, void* b, void* c, void * d)
{
    __real_zcopy_(a, b, c, d);
}

void __wrap_zgetrf_( void* a, void* b, void* c, void * d,
                     void* e, void* f)
{
    __real_zgetrf_(a, b, c, d, e, f);
}

void __wrap_zgetrs_( void* a, void* b, void* c, void* d,
                     void* e, void* f, void* g, void* h,
                     void* i)                     
           
{
    __real_zgetrs_(a, b, c, d, e, f, g, h, i);
}

void __wrap_ztrtrs_( void* a, void* b, void* c, void* d,
                     void* e, void* f, void* g, void* h,
                     void* i, void* j)                     
           
{
    __real_ztrtrs_(a, b, c, d, e, f, g, h, i, j);
}

void __wrap_zgemv_( void* a, void* b, void* c, void* d,
                     void* e, void* f, void* g, void* h,
                     void* i, void* j, void* k)                     
           
{
    __real_zgemv_(a, b, c, d, e, f, g, h, i, j, k);
}

// -L/home/export/online1/mdt00/shisuan/swzjsys/wys/must/sw-main/timer/ -lperf
// -Wl,--wrap=zgemm_  -Wl,--wrap=zcopy_  
// -Wl,--wrap=zgetrs_  -Wl,--wrap=zgetrf_ 
// -Wl,--wrap=ztrtrs_  -Wl,--wrap=zgemv_ 
// -L/usr/sw/yyzlib/xMath-SACA/   -lswblas  -lswlapack