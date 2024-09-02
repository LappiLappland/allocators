#include "freeListAllocator.h"
#include "commonAllocator.h"

#ifdef LL_ALLOC_DEBUG
#include <stdio.h>
#endif

FreeListAllocator initFreeListAllocator(size_t size) {
    FreeListAllocator allocator = {size, NULL, 0};

    FreeListHeader* start = malloc(size);
    allocator.start = start;
    allocator.freeBlocks = start;
    start->next = NULL;
    start->size = size;

    return allocator;
}

static void insertBlock(FreeListAllocator* allocator, FreeListHeader* inserted, FreeListHeader* previous) {
    if (previous == NULL) {
        if (allocator->freeBlocks == NULL) {
            allocator->freeBlocks = inserted;
            inserted->next = NULL;
        } else {
            inserted->next = allocator->freeBlocks;
            allocator->freeBlocks = inserted;
        }
    } else {
        if (previous->next == NULL) {
            previous->next = inserted;
            inserted->next = NULL;
        } else {
            inserted->next = previous->next;
            previous->next = inserted;
        }
    }
}

static void removeBlock(FreeListAllocator* allocator, FreeListHeader* removed, FreeListHeader* previous) {
    if (previous) {
        previous->next = removed->next;
    } else {
        allocator->freeBlocks = removed->next;
    }
}

static void coalesceBlocks(FreeListAllocator* allocator, FreeListHeader* current, FreeListHeader* previous) {
    // Check next block
    if (current->next != NULL && (void*)current + current->size == current->next) {
        current->size += current->next->size;
        removeBlock(allocator, current->next, current);
    }

    // Check previous block
    if (previous != NULL && (void*)previous + previous->size == current) {
        previous->size += current->size;
        removeBlock(allocator, current, previous);
    }
}

static FreeListHeader* findFirst(FreeListAllocator* allocator, size_t size, size_t alignment, void** alignedAddress, FreeListHeader** previousBlock ) {

    FreeListHeader* current = allocator->freeBlocks;
    void* dataAddressAligned = NULL;
    FreeListHeader* previous = NULL;
    while (current != NULL) {
        void* dataAddress = (void*)current + sizeof(FreeListBusyHeader);

        dataAddressAligned = (void*)alignMemory((word_t)dataAddress, alignment);
        size_t requiredSize = dataAddressAligned + size - (void*)current;

        if (current->size >= requiredSize) {
            break;
        }

        previous = current;
        current = current->next;
    }

    *alignedAddress = dataAddressAligned;
    *previousBlock = previous;

    return current;
}

static FreeListHeader* findBest(FreeListAllocator* allocator, size_t size, size_t alignment, void** alignedAddress, FreeListHeader** previousBlock ) {
    FreeListHeader* current = allocator->freeBlocks;
    FreeListHeader* best = current;
    void* dataAddressAligned = NULL;
    FreeListHeader* previous = NULL;
    FreeListHeader* bestPrevious = NULL;
    while (current != NULL) {
        void* dataAddress = (void*)current + sizeof(FreeListBusyHeader);

        dataAddressAligned = (void*)alignMemory((word_t)dataAddress, alignment);
        size_t requiredSize = dataAddressAligned + size - (void*)current;

        if (current->size >= requiredSize && current->size < best->size) {
            best = current;
            bestPrevious = previous;
            //If size fits almost perfectly, we can stop searching
            if (best->size - requiredSize < LL_ALLOC_FREELIST_BEST_DIFF) {
                break;
            }
        }

        previous = current;
        current = current->next;
    }

    //Have to find aligned address again, but for the best option
    void* dataAddress = (void*)best + sizeof(FreeListBusyHeader);
    dataAddressAligned = (void*)alignMemory((word_t)dataAddress, alignment);

    *alignedAddress = dataAddressAligned;
    *previousBlock = bestPrevious;

    return best;
}

void* mallocFreeListAllocator(FreeListAllocator* allocator, size_t size, size_t alignment, FreeListSearchMode mode) {
    
    void* blockData = 0;
    FreeListHeader* previousBlock = 0;

    FreeListHeader* block;
    switch (mode) {
        case FirstFit:
            block = findFirst(allocator, size, alignment, &blockData, &previousBlock);
            break;
        case BestFit:
            block = findBest(allocator, size, alignment, &blockData, &previousBlock);
            break;
    }

    size_t usedSize = blockData + size - (void*)block;
    size_t dataSize = size;
    size_t remainingSize = block->size - usedSize;

    //Split block
    if (remainingSize > sizeof(FreeListHeader)) {
        FreeListHeader* splittedBlock = (void*)block + usedSize;
        splittedBlock->size = remainingSize;
        insertBlock(allocator, splittedBlock, block);
    } else {
        //Not enough space for splitting, give that extra to data
        dataSize += remainingSize;
    }

    removeBlock(allocator, block, previousBlock);

    FreeListBusyHeader* busyHeader = blockData - sizeof(FreeListBusyHeader);
    busyHeader->size = dataSize;
    busyHeader->offset = (void*)busyHeader - (void*)block;

    return blockData;
}

void freeFreeListAllocator(FreeListAllocator* allocator, void* pointer) {
    FreeListBusyHeader* header = pointer - sizeof(FreeListBusyHeader);

    FreeListHeader* blockHeader = (void*)header - header->offset;
    blockHeader->size = (pointer + header->size) - (void*)blockHeader;
    blockHeader->next = NULL;

    FreeListHeader* current = allocator->freeBlocks;
    FreeListHeader* previous = NULL;
    while (current != NULL) {
        if (pointer < (void*)current) {
            break;
        }

        previous = current;
        current = current->next;
    }
    insertBlock(allocator, blockHeader, previous);

    coalesceBlocks(allocator, blockHeader, previous);
}

void resetFreeListAllocator(FreeListAllocator* allocator) {
    allocator->freeBlocks = allocator->start;
    allocator->freeBlocks->size = allocator->size;
}