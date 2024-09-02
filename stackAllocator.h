#ifndef LL_ALLOC_STACK
#define LL_ALLOC_STACK

#include <stdbool.h>
#include <stdlib.h>

#include "commonAllocator.h"

typedef struct _StackHeader {
    // Maybe uint8_t could work too
    uint32_t offset;
} StackHeader;

typedef struct {
    size_t size;
    StackHeader* start;
    StackHeader* current;
} StackAllocator;

StackAllocator initStackAllocator(size_t size);
void* mallocStackAllocator(StackAllocator* allocator, size_t size, size_t alignment);
void freeStackAllocator(StackAllocator* allocator, void* pointer);
void resetStackAllocator(StackAllocator* allocator);

#endif

