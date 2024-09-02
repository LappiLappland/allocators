#include "stackAllocator.h"
#include "commonAllocator.h"

#ifdef LL_ALLOC_DEBUG
#include <stdio.h>
#endif

StackAllocator initStackAllocator(size_t size) {
    StackAllocator allocator = {size, NULL, 0};

    StackHeader* start = malloc(size);
    start->offset = 0;
    allocator.start = start;
    allocator.current = start;

    return allocator;
}

void* mallocStackAllocator(StackAllocator* allocator, size_t size, size_t alignment) {
    if (size == 0) return NULL;

    StackHeader* dataAddress = (void*)allocator->current + sizeof(StackHeader);
    void* dataAddressAligned = (void*)alignMemory((word_t)dataAddress, alignment);

    if (dataAddressAligned + size > (void*)allocator->start + allocator->size) {
        return NULL; // Out of memory
    } 

    // Move header right before actual data for easy access on free
    StackHeader* actualHeader = dataAddressAligned - sizeof(StackHeader);
    // Length of empty space after alignment
    actualHeader->offset = (void*)actualHeader - (void*)allocator->current;

    allocator->current = dataAddressAligned + size;

    return dataAddressAligned;
}

void freeStackAllocator(StackAllocator* allocator, void* pointer) {
    if (pointer == NULL) return; // Null pointer

    if (pointer < (void*)allocator->current || pointer > (void*)allocator->current + allocator->size) {
        return; // Out of bounds
    }

    StackHeader* header = pointer - sizeof(StackHeader);
    allocator->current = (void*)header - header->offset;
}

void resetStackAllocator(StackAllocator* allocator) {
    allocator->current = allocator->start;
    allocator->current->offset = 0;
}