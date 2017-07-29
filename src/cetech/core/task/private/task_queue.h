#ifndef CETECH_QUEUE_MPMC_H
#define CETECH_QUEUE_MPMC_H

//==============================================================================
// Includes
//==============================================================================

#include <stdatomic.h>
#include <cetech/core/os/errors.h>
#include "celib/allocator.h"


//==============================================================================
// Implementation
//==============================================================================

typedef char cacheline_pad_t[64];
struct task_queue {
    uint32_t *_data;
    cacheline_pad_t _pad1;
    atomic_int *_sequences;
    cacheline_pad_t _pad2;
    uint32_t _capacity;
    cacheline_pad_t _pad3;
    atomic_int _dequeuePos;
    cacheline_pad_t _pad4;
    atomic_int _enqueuePos;
    cacheline_pad_t _pad5;
    int _capacityMask;
    cacheline_pad_t _pad6;
    struct cel_alloc *allocator;
};


void queue_task_init(struct task_queue *q,
                     uint32_t capacity,
                     struct cel_alloc *allocator) {
    *q = (struct task_queue) {};

    q->_capacityMask = capacity - 1;
    q->allocator = allocator;

    // capacity must be power of two
    CETECH_ASSERT("QUEUEMPC", 0 == (capacity & q->_capacityMask));

    q->_capacity = capacity;
    q->_data = CEL_ALLOCATE(allocator, uint32_t,
                            sizeof(uint32_t) * capacity);

    q->_sequences = CEL_ALLOCATE(allocator, atomic_int,
                                 sizeof(atomic_int) * capacity);

    for (uint32_t i = 0; i < capacity; ++i) {
        atomic_init(q->_sequences + i, i);
    }

    atomic_init(&q->_enqueuePos, 0);
    atomic_init(&q->_dequeuePos, 0);
}

void queue_task_destroy(struct task_queue *q) {
    CEL_FREE(q->allocator, q->_data);
    CEL_FREE(q->allocator, q->_sequences);
}

uint32_t queue_task_size(struct task_queue *q) {
    uint32_t e = atomic_load(&q->_enqueuePos) & q->_capacityMask;
    uint32_t d = atomic_load(&q->_dequeuePos) & q->_capacityMask;

    return e > d ? e - d : d - e;
}

int queue_task_push(struct task_queue *q,
                    uint32_t value) {
    int pos = atomic_load_explicit(&q->_enqueuePos, memory_order_relaxed);

    for (;;) {
        int seq = atomic_load_explicit(q->_sequences + (pos & q->_capacityMask),
                                       memory_order_acquire);

        intptr_t dif = (intptr_t) seq - (intptr_t) pos;

        if (dif == 0) {
            if (atomic_compare_exchange_weak_explicit(&q->_enqueuePos, &pos,
                                                      pos + 1,
                                                      memory_order_relaxed,
                                                      memory_order_relaxed)) {
                break;
            }
        } else if (dif < 0) {
            return 0;
        } else {
            pos = atomic_load_explicit(&q->_enqueuePos, memory_order_relaxed);
        }
    }

    q->_data[pos & q->_capacityMask] = value;
    atomic_store_explicit(&q->_sequences[pos & q->_capacityMask], pos + 1,
                          memory_order_release);

    return 1;
}

int queue_task_pop(struct task_queue *q,
                   uint32_t *value,
                   uint32_t defaultt) {
    int pos = atomic_load_explicit(&q->_dequeuePos, memory_order_relaxed);

    for (;;) {
        int seq = atomic_load_explicit(q->_sequences + (pos & q->_capacityMask),
                                       memory_order_acquire);

        intptr_t dif = (intptr_t) seq - (intptr_t) (pos + 1);

        if (dif == 0) {
            if (atomic_compare_exchange_weak_explicit(&q->_dequeuePos, &pos,
                                                      pos + 1,
                                                      memory_order_relaxed,
                                                      memory_order_relaxed)) {
                break;
            }
        } else if (dif < 0) {
            *value = defaultt;
            return 0;
        } else {
            pos = atomic_load_explicit(&q->_dequeuePos, memory_order_relaxed);
        }
    }

    *value = q->_data[pos & q->_capacityMask];
    atomic_store_explicit(&q->_sequences[pos & q->_capacityMask],
                          pos + q->_capacityMask + 1, memory_order_release);
    return 1;
}

#endif //CETECH_QUEUE_MPMC_H
