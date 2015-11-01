#pragma once

#include <cstring>
#include <atomic>

namespace cetech {
    // Based on http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue (thanks Dmitry Vyukov =))

    template < typename T, int SIZE >
    struct TaskQueue {
        enum {
            MASK = SIZE - 1u,
            CACHE_SIZE = 64
        };
        typedef char cacheline_pad_t[CACHE_SIZE];

        T _task_ids[SIZE];
        cacheline_pad_t pad0_;
        std::atomic < size_t > sequences[SIZE];
        cacheline_pad_t pad1_;
        std::atomic < size_t > enqueue_pos;
        cacheline_pad_t pad2_;
        std::atomic < size_t > dequeue_pos;
        cacheline_pad_t pad3_;

        TaskQueue() {
            memset(_task_ids, 0, sizeof(uint32_t) * SIZE);

            for (size_t i = 0; i != SIZE; i += 1) {
                sequences[i].store(i, std::memory_order_relaxed);
            }

            enqueue_pos.store(0, std::memory_order_relaxed);
            dequeue_pos.store(0, std::memory_order_relaxed);
        }
    };

    namespace taskqueue {
        template < typename T, int SIZE >
        size_t size(TaskQueue < T, SIZE >& q) {
            //(((B)->end + 1) % (B)->length - (B)->start - 1)
            
            size_t e = q.enqueue_pos & TaskQueue < T, SIZE >::MASK;
            size_t d = q.dequeue_pos & TaskQueue < T, SIZE >::MASK;

            return (e > d) ? (e - d) : (d - e);
        }
        
        template < typename T, int SIZE >
        void push(TaskQueue < T, SIZE >& q, T task) {
            size_t pos = q.enqueue_pos.load(std::memory_order_relaxed);

            for (;; ) {
                size_t seq = q.sequences[pos & TaskQueue < T, SIZE > ::MASK].load(std::memory_order_acquire);

                intptr_t dif = (intptr_t)seq - (intptr_t)pos;

                if (dif == 0) {
                    if (q.enqueue_pos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                        break;
                    }
                } else if (dif < 0) {
                    return /*false*/;

                } else {
                    pos = q.enqueue_pos.load(std::memory_order_relaxed);
                }
            }

            q._task_ids[pos & TaskQueue < T, SIZE > ::MASK] = task;
            q.sequences[pos & TaskQueue < T, SIZE > ::MASK].store(pos + 1, std::memory_order_release);
        }

        template < typename T, int SIZE >
        T pop(TaskQueue < T, SIZE >& q) {
            size_t pos = q.dequeue_pos.load(std::memory_order_relaxed);

            for (;; ) {
                size_t seq = q.sequences[pos & TaskQueue < T, SIZE > ::MASK].load(std::memory_order_acquire);

                intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
                if (dif == 0) {
                    if (q.dequeue_pos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                        break;
                    }
                } else if (dif < 0) {
                    return 0;

                } else {
                    pos = q.dequeue_pos.load(std::memory_order_relaxed);
                }
            }

            T ret = q._task_ids[pos & TaskQueue < T, SIZE > ::MASK];

            q.sequences[pos & TaskQueue < T, SIZE > ::MASK].store(pos + TaskQueue < T,
                                                                  SIZE > ::MASK + 1,
                                                                  std::memory_order_release);

            return ret;
        }
    }
}
