#ifndef LL_ALLOC_FREELIST
#define LL_ALLOC_FREELIST

#include <stdbool.h>
#include <stdlib.h>

#include "commonAllocator.h"

#define LL_ALLOC_FREELIST_BEST_DIFF 8

typedef struct _FreeListHeader {
    size_t size;
    struct _FreeListHeader* next;
} FreeListHeader;

typedef struct _FreeListBusyHeader {
    size_t size;
    // Maybe uint8_t could work too
    uint32_t offset;
} FreeListBusyHeader;

typedef struct {
    size_t size;
    FreeListHeader* start;
    FreeListHeader* freeBlocks;
} FreeListAllocator;

typedef enum {
    FirstFit,
    BestFit,
} FreeListSearchMode;

FreeListAllocator initFreeListAllocator(size_t size);
void* mallocFreeListAllocator(FreeListAllocator* allocator, size_t size, size_t alignment, FreeListSearchMode mode);
void freeFreeListAllocator(FreeListAllocator* allocator, void* pointer);
void resetFreeListAllocator(FreeListAllocator* allocator);

#endif

