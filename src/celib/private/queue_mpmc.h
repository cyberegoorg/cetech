#ifndef CE_QUEUE_MPMC_H
#define CE_QUEUE_MPMC_H

//==============================================================================
// Includes
//==============================================================================

#include <stdatomic.h>
#include <celib/os.h>
#include <celib/macros.h>
#include "celib/allocator.h"


//==============================================================================
// Implementation
//==============================================================================

typedef char cacheline_pad_t[64];
struct queue_mpmc {
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
    struct ce_alloc *allocator;
};


void queue_task_init(struct queue_mpmc *q,
                     uint32_t capacity,
                     struct ce_alloc *allocator) {
    *q = (struct queue_mpmc) {};

    q->_capacityMask = capacity - 1;
    q->allocator = allocator;

    // capacity must be power of two
    CE_ASSERT("QUEUEMPC", 0 == (capacity & q->_capacityMask));

    q->_capacity = capacity;
    q->_data = CE_ALLOC(allocator, uint32_t,
                        sizeof(uint32_t) * capacity);

    q->_sequences = CE_ALLOC(allocator, atomic_int,
                             sizeof(atomic_int) * capacity);

    for (uint32_t i = 0; i < capacity; ++i) {
        atomic_init(q->_sequences + i, i);
    }

    atomic_init(&q->_enqueuePos, 0);
    atomic_init(&q->_dequeuePos, 0);
}

void queue_task_destroy(struct queue_mpmc *q) {
    CE_FREE(q->allocator, q->_data);
    CE_FREE(q->allocator, q->_sequences);
}

uint32_t queue_task_size(struct queue_mpmc *q) {
    uint32_t e = atomic_load(&q->_enqueuePos) & q->_capacityMask;
    uint32_t d = atomic_load(&q->_dequeuePos) & q->_capacityMask;

    return e > d ? e - d : d - e;
}

int queue_task_push(struct queue_mpmc *q,
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

int queue_task_pop(struct queue_mpmc *q,
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

#endif //CE_QUEUE_MPMC_H
