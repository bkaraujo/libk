#include "libk/libk.h"

typedef struct {
    u64 index;
    char *payload;
} KMemoryPage;

typedef struct {
    KAllocator allocator;
    u64 allocated;
    u64 tagged_count[U8_MAX];
    u64 tagged_size[U8_MAX];
    u64 page_size;
    KMemoryPage page[U8_MAX];
} KAllocatorLinear;

typedef struct {
    KAllocator allocator;
} KAllocatorDynamic;

#if ! defined(K_MEMORY_ALLOCATOR_MAXIMUM)
#   define K_MEMORY_ALLOCATOR_MAXIMUM U8_MAX
#endif

static KAllocator *allocators[K_MEMORY_ALLOCATOR_MAXIMUM];

static KAllocator *k_linear_create(const u64 size) {
    K_FRAME_PUSH_WITH("%d", size)

    KAllocatorLinear *allocator = k_memory_alloc(sizeof(KAllocatorLinear));
    allocator->allocator.type = K_MEMORY_ALLOCATOR_LINEAR;
    allocator->page_size = size;

    KDEBUG("KAllocator 0x%p created with page length of %d bytes", allocator, allocator->page_size)
    K_FRAME_POP_WITH((KAllocator*) allocator);
}

static KAllocator *k_dynamic_create(const u64 size) {
    K_FRAME_PUSH_WITH("%llu", size)
    KFATAL("Unsupported allocator type");
}

KAllocator* k_memory_allocator_create(const KAllocatorType type, const u64 size) {
    K_FRAME_PUSH_WITH("%d, %llu", type, size)
    for (u8 i = 0 ; i < K_MEMORY_ALLOCATOR_MAXIMUM ; ++i) {
        if (allocators[i] != NULL) continue;

        if (type == K_MEMORY_ALLOCATOR_LINEAR) {
            allocators[i] = k_linear_create(size);
            K_FRAME_POP_WITH(allocators[i]);
        }

        if (type == K_MEMORY_ALLOCATOR_DYNAMIC) {
            allocators[i] = k_dynamic_create(size);
            K_FRAME_POP_WITH(allocators[i]);

        }
    }

    KFATAL("Unsupported allocator type");
}

static void* k_linear_alloc(KAllocatorLinear *allocator, const u64 size, const u8 tag) {
    K_FRAME_PUSH_WITH("0x%p, %llu, %d", allocator, size, tag)
    // -------------------------------------------------
    // Ensure that the allocator can hold the desired length
    // -------------------------------------------------
    if (size == 0) {
        KFATAL("KAllocator 0x%p allocation length must be greater then 0", allocator)
    }

    if (size > allocator->page_size) {
        KFATAL("KAllocator with page length of %d bytes. It cannot allocate %d bytes", allocator, allocator->page_size, size)
    }
    // -------------------------------------------------
    // Find a suitable TLMemoryPage within the allocator
    // -------------------------------------------------
    u8 found = U8_MAX;
    for (u8 i = 0; i < U8_MAX ; ++i) {

        // Initialize a new TLMemoryPage
        if (allocator->page[i].payload == NULL) {
            allocator->page[i].payload = k_memory_alloc(allocator->page_size);
            KTRACE("KAllocator 0x%p initializing page 0x%p", allocator, allocator->page[i].payload)

            found = i;
            break;
        }

        // check if the page support the desired length
        if (allocator->page[i].index + size <= allocator->page_size) {
            found = i;
            break;
        }
    }

    if (found == U8_MAX) {
        KWARN("KAllocator 0x%p no suitable TLMemoryPage", allocator)
        K_FRAME_POP_WITH(NULL)
    }
    // -------------------------------------------------
    // Adjust the KAllocator internal state
    // -------------------------------------------------
    allocator->allocated += size;
    allocator->tagged_count[tag] += 1;
    allocator->tagged_size[tag] += size;
    // -------------------------------------------------
    // Adjust the TLMemoryPage internal state
    // -------------------------------------------------
    void* address = allocator->page[found].payload + allocator->page[found].index;
    allocator->page[found].index += size;
    // -------------------------------------------------
    // Hand out the memory pointer
    // -------------------------------------------------
    K_FRAME_POP_WITH(address)
}

static void* k_dynamic_alloc(KAllocatorDynamic *allocator, const u64 size, const u8 tag) {
    K_FRAME_PUSH_WITH("0x%p, %llu, %d", allocator, size, tag)
    KFATAL("Unsupported allocator type")
}

void* k_memory_allocator_alloc(KAllocator* allocator, const u64 size, const u8 tag) {
    K_FRAME_PUSH_WITH("0x%p, %llu, %d", allocator, size, tag)

    void* allocated;

    switch (allocator->type) {
        default: KFATAL("Unsupported allocator type")
        case K_MEMORY_ALLOCATOR_LINEAR : allocated = k_linear_alloc((KAllocatorLinear*) allocator, size, tag); break;
        case K_MEMORY_ALLOCATOR_DYNAMIC: allocated = k_dynamic_alloc((KAllocatorDynamic*) allocator, size, tag); break;
    }

    K_FRAME_POP_WITH(allocated)
}

void k_memory_allocator_free(KAllocator* allocator, void* pointer) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", allocator, pointer)

    switch (allocator->type) {
        default: KFATAL("Unsupported allocator type")
        case K_MEMORY_ALLOCATOR_LINEAR : KFATAL("Unsupported allocator type")
        case K_MEMORY_ALLOCATOR_DYNAMIC: {
            KFATAL("Implementation missing")
        } break;
    }

    K_FRAME_POP
}

void k_memory_allocator_reset(KAllocator* allocator) {
    K_FRAME_PUSH_WITH("0x%p", allocator)

    switch (allocator->type) {
        default: KFATAL("Unsupported allocator type");
        case K_MEMORY_ALLOCATOR_DYNAMIC: KFATAL("Unsupported allocator type");
        case K_MEMORY_ALLOCATOR_LINEAR : {
            KAllocatorLinear* linear = (KAllocatorLinear*) allocator;
            for (u32 i = 0 ; i < U8_MAX ; ++i) {
                if (linear->page[i].payload == NULL) break;

                linear->page[i].index = 0;
                k_memory_set(linear->page[i].payload, 0, linear->page_size);
            }
        } break;
    }

    K_FRAME_POP
}

static void k_linear_destroy(const u8 index) {
    K_FRAME_PUSH_WITH("%d", index)

    KAllocatorLinear *linear = (KAllocatorLinear*) allocators[index];

    for (u32 i = 0 ; i < U8_MAX ; ++i) {
        if (linear->page[i].payload != NULL) {
            KDEBUG("KAllocator 0x%p releasing page 0x%p", linear, linear->page[i].payload)
            k_memory_free(linear->page[i].payload);
            linear->page[i].payload = NULL;
        }
    }

    for (u32 i = 0 ; i < U8_MAX ; ++i) {
        if (allocators[i] == NULL || allocators[i]->type == K_MEMORY_ALLOCATOR_LINEAR) continue;
        if ( ((KAllocatorLinear*) allocators[index])->tagged_size[i] != 0) {
            KVERBOSE("KAllocator 0x%p at %3d: [%03d] %llu bytes", linear, i, linear->tagged_count[i], linear->tagged_size[i]);
        }
    }

    k_memory_free(linear);
    allocators[index] = NULL;

    K_FRAME_POP
}

static void k_dynamic_destroy(const u8 index) {
    K_FRAME_PUSH_WITH("%d", index)
    KFATAL("Implementation missing")
}

K_INLINE static u16 k_memory_allocator_get_index(const KAllocator *desired) {
    K_FRAME_PUSH_WITH("0x%p", desired)

    if (desired == NULL) {
        KWARN("KAllocator is NULL")
        K_FRAME_POP_WITH(false)
    }

    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (allocators[i] == NULL) continue;
        if (allocators[i] == desired) {
            K_FRAME_POP_WITH(i)
        }
    }

    K_FRAME_POP_WITH(U16_MAX)
}

void k_memory_allocator_destroy(KAllocator* allocator) {
    K_FRAME_PUSH_WITH("0x%p", allocator)

    const u16 index = k_memory_allocator_get_index(allocator);
    if (index > U8_MAX) KFATAL("Unknown allocator 0x%p", allocator)

    switch (allocator->type) {
        default: KFATAL("Unsupported allocator type")
        case K_MEMORY_ALLOCATOR_LINEAR : k_linear_destroy(index); break;
        case K_MEMORY_ALLOCATOR_DYNAMIC: k_dynamic_destroy(index); break;
    }

    K_FRAME_POP
}