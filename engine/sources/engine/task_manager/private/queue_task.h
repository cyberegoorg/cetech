#ifndef CETECH_QUEUE_MPMC_H
#define CETECH_QUEUE_MPMC_H

//==============================================================================
// Includes
//==============================================================================

#include <stdatomic.h>

#include "celib/types.h"
#include "celib/errors/errors.h"
#include "celib/memory/memory.h"

//==============================================================================
// Implementation
//==============================================================================

typedef char cacheline_pad_t[64];
struct queue_task {
    u32 *_data;
    cacheline_pad_t _pad1;
    atomic_int *_sequences;
    cacheline_pad_t _pad2;
    u32 _capacity;
    cacheline_pad_t _pad3;
    atomic_int _dequeuePos;
    cacheline_pad_t _pad4;
    atomic_int _enqueuePos;
    cacheline_pad_t _pad5;
    int _capacityMask;
    cacheline_pad_t _pad6;
    struct allocator *allocator;
};


void queue_task_init(struct queue_task *q, u32 capacity, struct allocator *allocator) {
    *q = (struct queue_task) {0};

    q->_capacityMask = capacity - 1;
    q->allocator = allocator;

    CE_ASSERT_MSG("QUEUEMPC", 0 == (capacity & q->_capacityMask), "capacity must be power of two");

    q->_capacity = capacity;
    q->_data = CE_ALLOCATE(allocator, u32, capacity);

    q->_sequences = CE_ALLOCATE(allocator, atomic_int, capacity);

    for (u32 i = 0; i < capacity; ++i) {
        atomic_init(q->_sequences + i, i);
    }

    atomic_init(&q->_enqueuePos, 0);
    atomic_init(&q->_dequeuePos, 0);
}

void queue_task_destroy(struct queue_task *q) {
    CE_DEALLOCATE(q->allocator, q->_data);
    CE_DEALLOCATE(q->allocator, q->_sequences);
}

//u32 queue_mpmc_size(struct queue_task* q) {
//    u32 e = atomic_load(&q->_enqueuePos) & q->_capacityMask;
//    u32 d = atomic_load(&q->_dequeuePos) & q->_capacityMask;
//
//    return e > d ? e - d : d - e;
//}

int queue_task_push(struct queue_task *q, u32 value) {
    int pos = atomic_load_explicit(&q->_enqueuePos, memory_order_relaxed);

    for (;;) {
        int seq = atomic_load_explicit(q->_sequences + (pos & q->_capacityMask), memory_order_acquire);

        intptr_t dif = (intptr_t) seq - (intptr_t) pos;

        if (dif == 0) {
            if (atomic_compare_exchange_weak_explicit(&q->_enqueuePos, &pos, pos + 1, memory_order_relaxed,
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
    atomic_store_explicit(&q->_sequences[pos & q->_capacityMask], pos + 1, memory_order_release);

    return 1;
}

int queue_task_pop(struct queue_task *q, u32 *value, u32 defaultt) {
    int pos = atomic_load_explicit(&q->_dequeuePos, memory_order_relaxed);

    for (;;) {
        int seq = atomic_load_explicit(q->_sequences + (pos & q->_capacityMask), memory_order_acquire);

        intptr_t dif = (intptr_t) seq - (intptr_t) (pos + 1);

        if (dif == 0) {
            if (atomic_compare_exchange_weak_explicit(&q->_dequeuePos, &pos, pos + 1, memory_order_relaxed,
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
    atomic_store_explicit(&q->_sequences[pos & q->_capacityMask], pos + q->_capacityMask + 1, memory_order_release);
    return 1;
}

#endif //CETECH_QUEUE_MPMC_H
