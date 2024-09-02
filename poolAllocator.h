#ifndef LL_ALLOC_POOL
#define LL_ALLOC_POOL

#include <stdbool.h>
#include <stdlib.h>

#include "commonAllocator.h"

// Pool allocator initialization takes linear time
// Because we have to initialize all blocks in linked list
// Lazy mode initializes more blocks as needed
//#define LL_ALLOC_POOL_LAZY
#define LL_ALLOC_POOL_BLOCKS_PER_REQUEST 6

typedef struct _PoolHeader {
    //Behaves like linked list node
    struct _PoolHeader* next;
} PoolHeader;

typedef struct {
    size_t size;
    size_t blockSize;
    void* bufferStart; // Actual buffer start
    void* blocksStart; // Aligned buffer start
#ifdef LL_ALLOC_POOL_LAZY
    void* bufferCurrent; // Behaves like stack top
#endif

    PoolHeader* freeBlocks; // Linked list head
} PoolAllocator;

PoolAllocator initPoolAllocator(size_t size, size_t blockSize, size_t alignment);
void* mallocPoolAllocator(PoolAllocator* allocator, size_t size);
void freePoolAllocator(PoolAllocator* allocator, void* pointer);
void resetPoolAllocator(PoolAllocator* allocator);

#endif

