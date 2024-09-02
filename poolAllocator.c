#include "poolAllocator.h"
#include "commonAllocator.h"

#ifdef LL_ALLOC_DEBUG
#include <stdio.h>
#endif

PoolAllocator initPoolAllocator(size_t size, size_t blockSize, size_t alignment) {
    PoolAllocator allocator;
    
    void* start = malloc(size);
    
    //Have to align starting address
    PoolHeader* alignedStart = (void*)alignMemory((word_t)start, alignment);
    //If we align block size, each next block would end up aligned too
    size_t blockSizeAligned = alignMemory(blockSize, alignment);

    allocator.size = size;
    allocator.blockSize = blockSizeAligned;
    allocator.bufferStart = start;
    allocator.blocksStart = alignedStart;
    allocator.freeBlocks = NULL;
#ifdef LL_ALLOC_POOL_LAZY
    allocator.bufferCurrent = allocator.blocksStart;
#endif

#ifndef LL_ALLOC_POOL_LAZY
    resetPoolAllocator(&allocator);
#endif

    return allocator;
}

#ifdef LL_ALLOC_POOL_LAZY
static void requestMoreBlocks(PoolAllocator* allocator) {
    size_t freeLeft =  ((allocator->blocksStart + allocator->size) - allocator->bufferCurrent) / allocator->blockSize;

    size_t amount = freeLeft < LL_ALLOC_POOL_BLOCKS_PER_REQUEST ? freeLeft : LL_ALLOC_POOL_BLOCKS_PER_REQUEST;

    for (size_t i = 0; i < amount; i++) {
        PoolHeader* block = allocator->bufferCurrent;
        block->next = allocator->freeBlocks;
        allocator->freeBlocks = block;
        allocator->bufferCurrent += allocator->blockSize;
    }
}
#endif

void* mallocPoolAllocator(PoolAllocator* allocator, size_t size) {
    if (size == 0) return NULL;

    PoolHeader* block = allocator->freeBlocks;

#ifdef LL_ALLOC_POOL_LAZY
    if (block == NULL) {
        requestMoreBlocks(allocator);
        block = allocator->freeBlocks;
    }
#endif

    if (block == NULL) {
        return NULL; // Out of memory
    }

    allocator->freeBlocks = block->next;

    // We won't need header anymore, so user can overwrite it
    return block;
}

void freePoolAllocator(PoolAllocator* allocator, void* pointer) {
    if (pointer == NULL) return; // Null pointer

    if (pointer < allocator->blocksStart || pointer > allocator->bufferStart + allocator->size) {
        return; // Out of bounds;
    }

    // Since block is freed, we can swap data to header
    PoolHeader* block = pointer;
    block->next = allocator->freeBlocks;
    allocator->freeBlocks = block;
}

void resetPoolAllocator(PoolAllocator* allocator) {
    size_t length = (allocator->size - (allocator->blocksStart - allocator->bufferStart)) / allocator->blockSize;
    
    for (size_t i = 0; i < length; i++) {
        PoolHeader* block = allocator->blocksStart + i * allocator->blockSize;

        block->next = allocator->freeBlocks;
        allocator->freeBlocks = block;
    }
}