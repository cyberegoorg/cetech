//
//                      **SPSC queue**
//
// based on: https://github.com/mstump/queues/blob/master/include/spsc-bounded-queue.hpp

#ifndef CE_SPSC_H
#define CE_SPSC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"
#include <stdatomic.h>

// # Struct
typedef struct ce_spsc_queue_t0 {
    cache_line_pad_t _pad0;
    size_t size;
    size_t itemsize;
    size_t mask;
    uint8_t *buffer;

    cache_line_pad_t _pad1;
    atomic_size_t head;

    cache_line_pad_t _pad2;
    atomic_size_t tail;
} ce_spsc_queue_t0;

static inline void ce_spsc_init(ce_spsc_queue_t0 *queue,
                                size_t size,
                                size_t itemsize,
                                ce_alloc_t0 *alloc) {
    CE_ASSERT("", (size != 0) && ((size & (~size + 1)) == size));

    *queue = (ce_spsc_queue_t0) {
            .size = size,
            .itemsize = itemsize,
            .mask = size - 1,
            .buffer = CE_ALLOC(alloc, uint8_t, size * (itemsize + 1)),
    };
}

static inline void ce_spsc_free(ce_spsc_queue_t0 *queue,
                                ce_alloc_t0 *alloc) {
    CE_FREE(alloc, queue->buffer);

    *queue = (ce_spsc_queue_t0) {};
}

static inline bool ce_spsc_enqueue(ce_spsc_queue_t0 *queue,
                                   void *item) {
    const size_t head = atomic_load_explicit(&queue->head, memory_order_relaxed);

    if (((atomic_load_explicit(&queue->tail, memory_order_acquire) - (head + 1)) & queue->mask) >=
        1) {
        memcpy(queue->buffer + (queue->itemsize * (head & queue->mask)), item,
               queue->itemsize);
        atomic_store_explicit(&queue->head, queue->head + 1, memory_order_release);
        return true;
    }
    return false;
}

static inline bool ce_spsc_dequeue(ce_spsc_queue_t0 *queue,
                                   void *item) {
    const size_t tail = atomic_load_explicit(&queue->tail, memory_order_relaxed);

    if (((atomic_load_explicit(&queue->head, memory_order_acquire) - tail) & queue->mask) >= 1) {
        memcpy(item, queue->buffer + (queue->itemsize * (tail & queue->mask)),
               queue->itemsize);
        atomic_store_explicit(&queue->tail, queue->tail + 1, memory_order_release);
        return true;
    }
    return false;
}

#ifdef __cplusplus
};
#endif

#endif //CE_SPSC_H
