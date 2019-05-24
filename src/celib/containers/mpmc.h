//
//                      **MPMC queue**
//
// based on: http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue

#ifndef CE_MPMC_H
#define CE_MPMC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"
#include <stdatomic.h>

// # Struct
typedef struct ce_mpmc_queue_t0 {
    uint8_t *buffer;
    cache_line_pad_t _pad1;

    atomic_int *sequences;
    cache_line_pad_t _pad2;

    size_t capacity;
    cache_line_pad_t _pad3;

    atomic_int dequeue_pos;
    cache_line_pad_t _pad4;

    atomic_int enqueue_pos;
    cache_line_pad_t _pad5;

    uint32_t capacity_mask;
    cache_line_pad_t _pad6;

    ce_alloc_t0 *allocator;
    size_t itemsize;
} ce_mpmc_queue_t0;


static inline void ce_mpmc_init(ce_mpmc_queue_t0 *q,
                                size_t capacity,
                                size_t itemsize,
                                struct ce_alloc_t0 *allocator) {
    // capacity must be power of two
    CE_ASSERT("QUEUEMPC", 0 == (capacity & (capacity - 1)));

    *q = (struct ce_mpmc_queue_t0) {
            .itemsize = itemsize,
            .allocator = allocator,
            .capacity_mask = capacity - 1,
            .capacity = capacity,
            .buffer = CE_ALLOC(allocator, uint8_t, itemsize * capacity),
            .sequences = CE_ALLOC(allocator, atomic_int, sizeof(atomic_int) * capacity),
    };

    for (uint32_t i = 0; i < capacity; ++i) {
        atomic_init(q->sequences + i, i);
    }

    atomic_init(&q->enqueue_pos, 0);
    atomic_init(&q->dequeue_pos, 0);
}

static inline void ce_mpmc_clean(ce_mpmc_queue_t0 *q) {
    for (uint32_t i = 0; i < q->capacity; ++i) {
        atomic_init(q->sequences + i, i);
    }

    atomic_init(&q->enqueue_pos, 0);
    atomic_init(&q->dequeue_pos, 0);
}

static inline void ce_mpmc_free(ce_mpmc_queue_t0 *q) {
    CE_FREE(q->allocator, q->buffer);
    CE_FREE(q->allocator, q->sequences);
}

static inline uint32_t ce_mpmc_size(ce_mpmc_queue_t0 *q) {
    uint32_t e = atomic_load(&q->enqueue_pos) & q->capacity_mask;
    uint32_t d = atomic_load(&q->dequeue_pos) & q->capacity_mask;

    return e > d ? e - d : d - e;
}

static inline bool ce_mpmc_enqueue(ce_mpmc_queue_t0 *q,
                                   void *value) {
    int pos = atomic_load_explicit(&q->enqueue_pos, memory_order_relaxed);

    for (;;) {
        int seq = atomic_load_explicit(q->sequences + (pos & q->capacity_mask),
                                       memory_order_acquire);

        intptr_t dif = (intptr_t) seq - (intptr_t) pos;

        if (dif == 0) {
            if (atomic_compare_exchange_weak_explicit(&q->enqueue_pos, &pos,
                                                      pos + 1,
                                                      memory_order_relaxed,
                                                      memory_order_relaxed)) {
                break;
            }
        } else if (dif < 0) {
            return false;
        } else {
            pos = atomic_load_explicit(&q->enqueue_pos, memory_order_relaxed);
        }
    }

    memcpy(q->buffer + ((pos & q->capacity_mask) * q->itemsize), value, q->itemsize);

    atomic_store_explicit(&q->sequences[pos & q->capacity_mask], pos + 1, memory_order_release);

    return true;
}

static inline bool ce_mpmc_dequeue(ce_mpmc_queue_t0 *q,
                                   void *value) {
    int pos = atomic_load_explicit(&q->dequeue_pos, memory_order_relaxed);

    for (;;) {
        int seq = atomic_load_explicit(q->sequences + (pos & q->capacity_mask),
                                       memory_order_acquire);

        intptr_t dif = (intptr_t) seq - (intptr_t) (pos + 1);

        if (dif == 0) {
            if (atomic_compare_exchange_weak_explicit(&q->dequeue_pos, &pos,
                                                      pos + 1,
                                                      memory_order_relaxed,
                                                      memory_order_relaxed)) {
                break;
            }
        } else if (dif < 0) {
            return false;
        } else {
            pos = atomic_load_explicit(&q->dequeue_pos, memory_order_relaxed);
        }
    }

    memcpy(value, q->buffer + ((pos & q->capacity_mask) * q->itemsize), q->itemsize);

    atomic_store_explicit(&q->sequences[pos & q->capacity_mask],
                          pos + q->capacity_mask + 1, memory_order_release);
    return true;
}


#ifdef __cplusplus
};
#endif

#endif //CE_MPMC_H
