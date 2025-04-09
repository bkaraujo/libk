#include "libk/libk.h"

typedef struct BKMemory {
    void *pointer;
    struct BKMemory *next;
} KMemory;

static KMemory head;

void* k_memory_alloc(const u64 size) {
    K_FRAME_PUSH_WITH("%ull", size)
    // =======================================================================
    // Find the stop to create the KMemory
    // =======================================================================
    KMemory *node = NULL;
    if (head.next == NULL) { node = &head; }
    else {
        KMemory *current = head.next;
        while (current->next != NULL) current = current->next;
        node = current;
    }
    // =======================================================================
    // Allocate the memory a record stack frame of the caller
    // =======================================================================
    node->next = malloc(sizeof(KMemory));
    if (node->next == NULL) KFATAL("Failed to allocate %llu bytes", sizeof(KMemory))
    memset(node->next, 0, sizeof(KMemory));

    node->next->pointer = malloc(size);
    if (node->next->pointer == NULL) KFATAL("Failed to allocate %llu bytes", size)
    memset(node->next->pointer, 0, size);
    // =======================================================================
    // Deliver the result
    // =======================================================================
    KTRACE("Allocated 0x%p with %llu bytes", node->next->pointer, size)
    K_FRAME_POP_WITH(node->next->pointer)
}

void k_memory_free(void *block) {
    K_FRAME_PUSH_WITH("0x%p", block)

    if (block == NULL) KFATAL("block is NULL")
    if (head.next == NULL) KFATAL("Unknown block 0x%p", block)

    KMemory *node = NULL;
    if (head.next->pointer == block) {
        node = head.next;
        head.next = node->next;
    } else {
        KMemory *s = head.next;
        while (s != NULL) {
            KMemory *current = s->next;
            if (current == NULL) break;
            if (current->pointer == block) {
                s->next = current->next;
                node = current;
                break;
            }

            s = current;
        }
    }

    if (node == NULL) KFATAL("Unknown block 0x%p", block)

    free(node->pointer);
    free(node);

    KTRACE("Deallocated 0x%p", block)
    K_FRAME_POP
}

void k_memory_set(void *target, const i32 value, const u64 size) {
    K_FRAME_PUSH_WITH("0x%p, %d, llu", target, value, size)

    if (target == NULL) KFATAL("target is NULL")
    if (size == 0) KFATAL("size is 0")
    memset(target, value, size);

    K_FRAME_POP
}

void k_memory_copy(void *target, const void *source, const u64 size) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p, %llu", target, source, size)
    memcpy(target, source, size);
    K_FRAME_POP
}
