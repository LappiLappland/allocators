#ifndef LL_ALLOC_COMMON
#define LL_ALLOC_COMMON

#include <stddef.h>
#include <stdint.h>

#define LL_ALLOC_DEBUG

typedef intptr_t word_t;

static inline word_t alignMemory(word_t n, size_t alignment) {
  return (n + alignment - 1) & ~(alignment - 1);
}

#endif