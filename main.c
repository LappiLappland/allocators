#include "freeListAllocator.h"

#include <stdint.h>
#include <stdio.h>

//256 mb
#define LL_ALLOC_SIZE (100)

int main(){

    FreeListAllocator allocator = initFreeListAllocator(LL_ALLOC_SIZE);

    int32_t* a = mallocFreeListAllocator(&allocator, sizeof(int32_t), 8, BestFit);
    *a = 31;
    int32_t* b = mallocFreeListAllocator(&allocator, sizeof(int32_t), 8, BestFit);
    *b = 15;
    int32_t* c = mallocFreeListAllocator(&allocator, sizeof(int32_t), 8, BestFit);
    *c = 562;

    freeFreeListAllocator(&allocator, b);
    freeFreeListAllocator(&allocator, c);
    int32_t* d = mallocFreeListAllocator(&allocator, sizeof(int32_t), 8, BestFit);
    *d = 56;

    printf("-> %d -> %d ->%d\n", *a, *b, *c);

    return 0;
}