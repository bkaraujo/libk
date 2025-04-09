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

typedef enum {
  K_MEMORY_ALLOCATOR_LINEAR,
  K_MEMORY_ALLOCATOR_DYNAMIC
} KAllocatorType;

typedef struct {
  KAllocatorType type;
} KAllocator;

KAllocator* k_memory_allocator_create(KAllocatorType type, u64 size);
void* k_memory_allocator_alloc(KAllocator* allocator, u64 size, u8 tag);
void k_memory_allocator_free(KAllocator* allocator, void* pointer);
void k_memory_allocator_reset(KAllocator* allocator);
void k_memory_allocator_destroy(KAllocator* allocator);

#endif
