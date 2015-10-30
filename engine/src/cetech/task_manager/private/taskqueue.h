#pragma once

#include <cstring>
#include <atomic>

namespace cetech {
    // Based on http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue (thanks Dmitry Vyukov =))

    template<int SIZE>
    struct TaskQueue {
        enum {
            MASK = SIZE - 1u
        };

        std::atomic<size_t> enqueue_pos;
        std::atomic<size_t> dequeue_pos;
        std::atomic<size_t> sequences[SIZE];
        uint32_t _task_ids[SIZE];

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
        template<int SIZE>
        void push(TaskQueue<SIZE>& q, uint32_t task) {
            size_t pos = q.enqueue_pos.load(std::memory_order_relaxed);

            for (;;) {
              size_t seq = q.sequences[pos & TaskQueue<SIZE> ::MASK].load(std::memory_order_acquire);

              intptr_t dif = (intptr_t)seq - (intptr_t)pos;

              if (dif == 0) {
                if (q.enqueue_pos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                  break;

              } else if (dif < 0) {
                return /*false*/;

              } else {
                pos = q.enqueue_pos.load(std::memory_order_relaxed);
              }
            }

            q._task_ids[pos & TaskQueue<SIZE> ::MASK] = task + 1;
            q.sequences[pos & TaskQueue<SIZE> ::MASK].store(pos + 1, std::memory_order_release);
        }

        template<int SIZE>
        uint32_t pop(TaskQueue<SIZE>& q) {
            size_t pos = q.dequeue_pos.load(std::memory_order_relaxed);

            for (;;) {
              size_t seq = q.sequences[pos & TaskQueue<SIZE> ::MASK].load(std::memory_order_acquire);

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

            uint32_t ret = q._task_ids[pos & TaskQueue<SIZE> ::MASK];

            q.sequences[pos & TaskQueue<SIZE>::MASK].store(pos + TaskQueue<SIZE> ::MASK + 1, std::memory_order_release);

            return ret;
        }
    }
}
