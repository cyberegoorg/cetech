//==============================================================================
// Based on bitsquid foundation.
// git+web: https://bitbucket.org/bitsquid/foundation
//==============================================================================

#ifndef CETECH_QUEUE_H
#define CETECH_QUEUE_H

#include <memory.h>
#include "array.h"

//==============================================================================
// Interface macros
//==============================================================================

#define QUEUE_T(N)                 struct queue_##N

#define QUEUE_INIT(N, a, alloc)    queue_init_##N(a, alloc)
#define QUEUE_DESTROY(N, a)        queue_destroy_##N(a)

#define QUEUE_SIZE(N, q)                queue_size_##N(q)
#define QUEUE_SPACE(N, q)               queue_space_##N( q)
#define QUEUE_RESERVE(N, q, size)       queue_reserve_##N(q)
#define QUEUE_PUSH_BACK(N, q, item)     queue_push_back_##N( q, item)
#define QUEUE_POP_BACK(N, q)            queue_pop_back_##N(q)
#define QUEUE_PUSH_FRONT(N, q, item)    queue_push_front_##N( q, item)
#define QUEUE_POP_FRONT(N, q)           queue_pop_front_##N(q)
#define QUEUE_CONSUME(N, q)             queue_consume_##N( q, n)
#define QUEUE_PUSH(N, q, items, n)      queue_push_##N(q, items, n)
#define QUEUE_BEGIN_FRONT(N, q)         queue_begin_front_##N(q)
#define QUEUE_END_FRONT(N, q)           queue_end_front_##N(q)

#define QUEUE_AT(q, i) ARRAY_AT(&(q)->_data, (i + (q)->_offset) % ARRAY_SIZE(&(q)->_data))

//==============================================================================
// Prototypes macro
//==============================================================================

#define QUEUE_PROTOTYPE(T) QUEUE_PROTOTYPE_N(T, T)
#define QUEUE_PROTOTYPE_N(T, N)                                                                             \
    struct queue_##N {                                                                                      \
        ARRAY_T(N) _data;                                                                                   \
        u32 _size;                                                                                          \
        u32 _offset;                                                                                        \
    };                                                                                                      \
    static inline void queue_init_##N(QUEUE_T(N) *q,                                                        \
                                      struct allocator* allocator) {                                        \
        CE_ASSERT("queue_"#T, q != NULL);                                                                   \
        CE_ASSERT("queue_"#T, allocator != NULL);                                                           \
        ARRAY_INIT(N, &q->_data, allocator);                                                                \
        q->_size = 0;                                                                                       \
        q->_offset = 0;                                                                                     \
    }                                                                                                       \
                                                                                                            \
    static inline uint32_t queue_size_##N(const  QUEUE_T(N) *q) {                                           \
        return q->_size;                                                                                    \
    }                                                                                                       \
                                                                                                            \
                                                                                                            \
    static inline  void queue_destroy_##N(QUEUE_T(N) *q) {                                                  \
        CE_ASSERT("queue_"#T, q != NULL);                                                                   \
        ARRAY_DESTROY(N, &q->_data);                                                                        \
        q->_size = 0;                                                                                       \
        q->_offset = 0;                                                                                     \
    }                                                                                                       \
                                                                                                            \
    static inline void queue_increase_capacity_##N( QUEUE_T(N) *q, u32 new_capacity) {                      \
        uint32_t end = ARRAY_SIZE(&q->_data);                                                               \
                                                                                                            \
        ARRAY_RESIZE(N, &q->_data, new_capacity);                                                           \
        if (q->_offset + q->_size > end) {                                                                  \
            uint32_t end_items = end - q->_offset;                                                          \
            memmove(ARRAY_BEGIN(&q->_data) + new_capacity - end_items, ARRAY_BEGIN(&q->_data) + q->_offset, \
                    end_items * sizeof(T));                                                                 \
            q->_offset += new_capacity - end;                                                               \
        }                                                                                                   \
    }                                                                                                       \
                                                                                                            \
    static inline void queue_grow_##N( QUEUE_T(N) *q, uint32_t min_capacity) {                              \
        uint32_t new_capacity = ARRAY_SIZE(&q->_data) * 2 + 8;                                              \
        if (new_capacity < min_capacity)                                                                    \
            new_capacity = min_capacity;                                                                    \
        queue_increase_capacity_##N(q, new_capacity);                                                       \
    }                                                                                                       \
                                                                                                            \
                                                                                                            \
                                                                                                            \
    static inline uint32_t queue_space_##N(const  QUEUE_T(N) *q) {                                          \
        return ARRAY_SIZE(&q->_data) - q->_size;                                                            \
    }                                                                                                       \
                                                                                                            \
    static inline void queue_reserve_##N( QUEUE_T(N) *q, uint32_t size) {                                   \
        if (size > q->_size)                                                                                \
            queue_increase_capacity_##N(q, size);                                                           \
    }                                                                                                       \
                                                                                                            \
    static inline void queue_push_back_##N( QUEUE_T(N) *q, T item) {                                        \
        if (!queue_space_##N(q))                                                                            \
            queue_grow_##N(q, 0);                                                                           \
        q->_data.data[q->_size++] = item;                                                                   \
    }                                                                                                       \
                                                                                                            \
    static inline void queue_pop_back_##N( QUEUE_T(N) *q) {                                                 \
        --q->_size;                                                                                         \
    }                                                                                                       \
                                                                                                            \
    static inline void queue_push_front_##N( QUEUE_T(N) *q, T item) {                                       \
        if (!queue_space_##N(q))                                                                            \
            queue_grow_##N(q, 0);                                                                           \
        q->_offset = (q->_offset - 1 + ARRAY_SIZE(&q->_data)) % ARRAY_SIZE(&q->_data);                      \
        ++q->_size;                                                                                         \
        q->_data.data[0] = item;                                                                            \
    }                                                                                                       \
                                                                                                            \
    static inline void queue_pop_front_##N( QUEUE_T(N) *q) {                                                \
        q->_offset = (q->_offset + 1) % ARRAY_SIZE(&q->_data);                                              \
        --q->_size;                                                                                         \
    }                                                                                                       \
                                                                                                            \
    inline void queue_consume_##N( QUEUE_T(N) *q, uint32_t n) {                                             \
        q->_offset = (q->_offset + n) % ARRAY_SIZE(&q->_data);                                              \
        q->_size -= n;                                                                                      \
    }                                                                                                       \
                                                                                                            \
    static inline void queue_push_##N( QUEUE_T(N) *q, T *items, uint32_t n) {                               \
        if (queue_space_##N(q) < n) {                                                                       \
            queue_grow_##N(q, ARRAY_SIZE(&q->_data) + n);                                                   \
        }                                                                                                   \
                                                                                                            \
        const uint32_t size = ARRAY_SIZE(&q->_data);                                                        \
        const uint32_t insert = (q->_offset + q->_size) % size;                                             \
                                                                                                            \
        uint32_t to_insert = n;                                                                             \
        if (insert + to_insert > size) {                                                                    \
            to_insert = size - insert;                                                                      \
        }                                                                                                   \
                                                                                                            \
        memcpy(ARRAY_BEGIN(& q->_data) + insert, items, to_insert * sizeof(T));                             \
        q->_size += to_insert;                                                                              \
        items += to_insert;                                                                                 \
        n -= to_insert;                                                                                     \
                                                                                                            \
        memcpy(ARRAY_BEGIN(& q->_data), items, n * sizeof(T));                                              \
        q->_size += n;                                                                                      \
    }                                                                                                       \
                                                                                                            \
    static inline T *queue_begin_front_##N( QUEUE_T(N) *q) {                                                \
        return ARRAY_BEGIN(& q->_data) + q->_offset;                                                        \
    }                                                                                                       \
                                                                                                            \
    static inline T *queue_end_front_##N( QUEUE_T(N) *q) {                                                  \
        uint32_t end = q->_offset + q->_size;                                                               \
        return end > ARRAY_SIZE(&q->_data) ? ARRAY_END(&q->_data) : ARRAY_BEGIN(& q->_data) + end;          \
    }                                                                                                       \

QUEUE_PROTOTYPE_N(void*, void)

QUEUE_PROTOTYPE_N(char*, pchar)

QUEUE_PROTOTYPE(char)

QUEUE_PROTOTYPE(int)

QUEUE_PROTOTYPE(u8)

QUEUE_PROTOTYPE(u16)

QUEUE_PROTOTYPE(u32)

QUEUE_PROTOTYPE(u64)

QUEUE_PROTOTYPE(i8)

QUEUE_PROTOTYPE(i16)

QUEUE_PROTOTYPE(i32)

QUEUE_PROTOTYPE(i64)

#endif //CETECH_QUEUE_H
