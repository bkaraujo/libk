#ifndef __LIBK_MEMORY__
#define __LIBK_MEMORY__

#include "libk/libk.h"

void* k_memory_alloc(u64 size);
void k_memory_free(void *block);
void k_memory_set(void *target, i32 value, u64 size);
void k_memory_copy(void *target, const void *source, u64 size);

#if defined(K_PLATFORM_WINDDOWS)
void* k_memory_heap_create(u64 size);
void* k_memory_heap_alloc(u64 size);
void k_memory_heap_free(void* block);
void k_memory_heap_destroy(void* heap);
#endif

#endif
