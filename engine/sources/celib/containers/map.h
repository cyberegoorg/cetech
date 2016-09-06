//==============================================================================
// Based on bitsquid foundation.
// git+web: https://bitbucket.org/bitsquid/foundation
//==============================================================================

#ifndef CETECH_CONTAINERS_MAP_H
#define CETECH_CONTAINERS_MAP_H

//==============================================================================
// Includes
//==============================================================================

#include "array.h"

//==============================================================================
// defines
//==============================================================================

#define _MAP_END_OF_LIST 0xffffffffu

//==============================================================================
// Shared macros
//==============================================================================

#define MAP_T(N) struct map_##N
#define MAP_ENTRY_T(N) struct map_entry_##N

#define MAP_INIT(N, h, a) map_init_##N(h, a)
#define MAP_DESTROY(N, h) map_destroy_##N(h)

//==============================================================================
// Map interface macros
//==============================================================================

#define MAP_HAS(N, h, key)                map_has_##N(h, key)
#define MAP_GET(N, h, key, deffault)      map_get_##N(h, key, deffault)
#define MAP_SET(N, h, key, value)         map_set_##N(h, key, (value))
#define MAP_REMOVE(N, h, key)             map_remove_##N(h, key)
#define MAP_RESERVE(N, h, size)           map_reserve_##N(h, size)
#define MAP_CLEAR(N, h)                   map_clear_##N(h)
#define MAP_BEGIN(N, h)                   map_begin_##N(h)
#define MAP_END(N, h)                     map_end_##N(h)

//==============================================================================
// Multi-Map interface macros
//==============================================================================

#define MULTIMAP_FIND_FIRST(N, h, key)    multimap_find_first_##N(h, key)
#define MULTIMAP_FIND_NEXT(N, h, e)       multimap_find_next_##N(h, e)
#define MULTIMAP_COUNT(N, h, key)         multimap_count_##N(h, key)
#define MULTIMAP_GET(N, h, key, items)    multimap_get_##N(h, key, items)
#define MULTIMAP_INSERT(N, h, key, value) multimap_insert_##N(h, key, value)
#define MULTIMAP_REMOVE(N, h, key, e)     multimap_remove_##N(h, e)
#define MULTIMAP_REMOVE_ALL(N, h, key)    multimap_remove_all_##N(h, key)

//==============================================================================
// Structs
//==============================================================================

struct map_find_result {
    uint32_t map_i;
    uint32_t data_prev;
    uint32_t data_i;
};

#define MAP_PROTOTYPE(T) MAP_PROTOTYPE_N(T, T)
#define MAP_PROTOTYPE_N(T, N)                                                  \
                                                                               \
struct map_entry_##N {                                                         \
    u64 key;                                                                   \
    u32 next;                                                                  \
    T value;                                                                   \
};                                                                             \
                                                                               \
typedef struct map_entry_##N map_entry_t_##N;                                  \
ARRAY_PROTOTYPE(map_entry_t_##N);                                              \
struct map_##N {                                                               \
    ARRAY_T(u32) _hash;                                                        \
    ARRAY_T(map_entry_t_##N) _data;                                            \
};                                                                             \
                                                                               \
static void map_init_##N(MAP_T(N) *h, struct allocator *allocator) {           \
    ARRAY_INIT(u32, &h->_hash, allocator);                                     \
    ARRAY_INIT(map_entry_t_##N, &h->_data, allocator);                         \
}                                                                              \
                                                                               \
static void map_destroy_##N(MAP_T(N) *h) {                                     \
    ARRAY_DESTROY(u32, &h->_hash);                                             \
    ARRAY_DESTROY(map_entry_t_##N, &h->_data);                                 \
}                                                                              \
                                                                               \
                                                                               \
static uint32_t _map_add_entry_##N(MAP_T(N) *h, uint64_t key) {                \
    map_entry_t_##N e;                                                         \
    e.key = key;                                                               \
    e.next = _MAP_END_OF_LIST;                                                 \
                                                                               \
    uint32_t ei = ARRAY_SIZE(&h->_data);                                       \
    ARRAY_PUSH_BACK(map_entry_t_##N, &h->_data, e);                            \
                                                                               \
    return ei;                                                                 \
}                                                                              \
                                                                               \
static struct map_find_result _map_find_key_##N(MAP_T(N) *h, uint64_t key) {   \
    struct map_find_result fr;                                                 \
    fr.map_i = _MAP_END_OF_LIST;                                               \
    fr.data_prev = _MAP_END_OF_LIST;                                           \
    fr.data_i = _MAP_END_OF_LIST;                                              \
                                                                               \
    if (ARRAY_SIZE(&h->_hash) == 0)                                            \
        return fr;                                                             \
                                                                               \
    fr.map_i = key % ARRAY_SIZE(&h->_hash);                                    \
    fr.data_i = ARRAY_AT(&h->_hash, fr.map_i);                                 \
    while (fr.data_i != _MAP_END_OF_LIST) {                                    \
        if (ARRAY_AT(&h->_data, fr.data_i).key == key)                         \
            return fr;                                                         \
                                                                               \
        fr.data_prev = fr.data_i;                                              \
        fr.data_i = ARRAY_AT(&h->_data, fr.data_i).next;                       \
    }                                                                          \
                                                                               \
    return fr;                                                                 \
}                                                                              \
                                                                               \
static void _map_erase_##N(MAP_T(N) *h, const struct map_find_result *fr) {    \
    if (fr->data_prev == _MAP_END_OF_LIST)                                     \
        ARRAY_AT(&h->_hash, fr->map_i) = ARRAY_AT(&h->_data, fr->data_i).next; \
    else                                                                       \
        ARRAY_AT(&h->_data, fr->data_prev).next = ARRAY_AT(&h->_data,          \
                                                           fr->data_i).next;   \
                                                                               \
    if (fr->data_i == ARRAY_SIZE(&h->_data) - 1) {                             \
        ARRAY_POP_BACK(map_entry_t_##N, &h->_data);                            \
        return;                                                                \
    }                                                                          \
                                                                               \
    ARRAY_AT(&h->_data, fr->data_i) = ARRAY_AT(&h->_data,                      \
                                               ARRAY_SIZE(&h->_data) - 1);     \
                                                                               \
    struct map_find_result last = _map_find_key_##N(h, ARRAY_AT(&h->_data, fr->data_i).key);\
                                                                               \
    if (last.data_prev != _MAP_END_OF_LIST)                                    \
        ARRAY_AT(&h->_data, last.data_prev).next = fr->data_i;                 \
    else                                                                       \
        ARRAY_AT(&h->_hash, last.map_i) = fr->data_i;                          \
}                                                                              \
                                                                               \
static struct map_find_result _map_find_entry_##N(                             \
    MAP_T(N) *h,                                                               \
    const map_entry_t_##N *e                                                   \
) {                                                                            \
    struct map_find_result fr;                                                 \
    fr.map_i = _MAP_END_OF_LIST;                                               \
    fr.data_prev = _MAP_END_OF_LIST;                                           \
    fr.data_i = _MAP_END_OF_LIST;                                              \
                                                                               \
    if (ARRAY_SIZE(&h->_hash) == 0)                                            \
        return fr;                                                             \
                                                                               \
    fr.map_i = e->key % ARRAY_SIZE(&h->_hash);                                 \
    fr.data_i = ARRAY_AT(&h->_hash, fr.map_i);                                 \
    while (fr.data_i != _MAP_END_OF_LIST) {                                    \
        if (&ARRAY_AT(&h->_data, fr.data_i) == e)                              \
            return fr;                                                         \
                                                                               \
        fr.data_prev = fr.data_i;                                              \
        fr.data_i = ARRAY_AT(&h->_data, fr.data_i).next;                       \
    }                                                                          \
    return fr;                                                                 \
}                                                                              \
                                                                               \
static uint32_t _map_find_or_fail_##N(MAP_T(N) *h, uint64_t key) {             \
    return _map_find_key_##N(h, key).data_i;                                   \
}                                                                              \
                                                                               \
static uint32_t _map_find_or_make_##N(MAP_T(N) *h, uint64_t key) {             \
    const struct map_find_result fr = _map_find_key_##N(h, key);               \
    if (fr.data_i != _MAP_END_OF_LIST)                                         \
        return fr.data_i;                                                      \
                                                                               \
    uint32_t i = _map_add_entry_##N(h, key);                                   \
    if (fr.data_prev == _MAP_END_OF_LIST)                                      \
        ARRAY_AT(&h->_hash, fr.map_i) = i;                                     \
    else                                                                       \
        ARRAY_AT(&h->_data, fr.data_prev).next = i;                            \
                                                                               \
    return i;                                                                  \
}                                                                              \
                                                                               \
static uint32_t _map_make_##N(MAP_T(N) *h, uint64_t key) {                     \
    const struct map_find_result fr = _map_find_key_##N(h, key);               \
    const uint32_t i = _map_add_entry_##N(h, key);                             \
                                                                               \
    if (fr.data_prev == _MAP_END_OF_LIST)                                      \
        ARRAY_AT(&h->_hash, fr.map_i) = i;                                     \
                                                                               \
    else                                                                       \
        ARRAY_AT(&h->_data, fr.data_prev).next = i;                            \
                                                                               \
    ARRAY_AT(&h->_data, i).next = fr.data_i;                                   \
                                                                               \
    return i;                                                                  \
}                                                                              \
                                                                               \
static void _map_find_and_erase_##N(MAP_T(N) *h, uint64_t key) {               \
    const struct map_find_result fr = _map_find_key_##N(h, key);               \
    if (fr.data_i != _MAP_END_OF_LIST)                                         \
        _map_erase_##N(h, &fr);                                                \
}                                                                              \
                                                                               \
static void multimap_insert_##N(MAP_T(N) *h, uint64_t key, T value);           \
                                                                               \
static void _map_remap_##N(MAP_T(N) *h, uint32_t new_size) {                   \
    MAP_T(N) nh;                                                               \
    map_init_##N(&nh, h->_hash.allocator);                                     \
                                                                               \
    ARRAY_RESIZE(u32, &nh._hash, new_size);                                    \
    ARRAY_RESERVE(map_entry_t_##N, &nh._data, ARRAY_SIZE(&h->_data));          \
                                                                               \
    for (uint32_t i = 0; i < new_size; ++i)                                    \
        ARRAY_AT(&nh._hash, i) = _MAP_END_OF_LIST;                             \
                                                                               \
    for (uint32_t i = 0; i < ARRAY_SIZE(&h->_data); ++i) {                     \
        const map_entry_t_##N e = ARRAY_AT(&h->_data, i);                      \
                                                                               \
        multimap_insert_##N(&nh, e.key, e.value);                              \
    }                                                                          \
                                                                               \
    MAP_T(N) empty;                                                            \
    map_init_##N(&empty, h->_hash.allocator);                                  \
                                                                               \
    map_destroy_##N(h);                                                        \
    memory_copy(h, &nh, sizeof(MAP_T(N)));                                     \
    memory_copy(&nh, &empty, sizeof(MAP_T(N)));                                \
}                                                                              \
                                                                               \
static int _map_full_##N(MAP_T(N) *h) {                                        \
    const float max_load_factor = 0.7f;                                        \
    return ARRAY_SIZE(&h->_data) >= ARRAY_SIZE(&h->_hash) * max_load_factor;   \
}                                                                              \
                                                                               \
static void _map_grow_##N(MAP_T(N) *h) {                                       \
    const uint32_t new_size = ARRAY_SIZE(&h->_data) * 2 + 10;                  \
    _map_remap_##N(h, new_size);                                               \
}                                                                              \
                                                                               \
static int map_has_##N(MAP_T(N) *h, uint64_t key) {                            \
    return _map_find_or_fail_##N(h, key) != _MAP_END_OF_LIST;                  \
}                                                                              \
                                                                               \
static T map_get_##N(MAP_T(N) *h, uint64_t key, T deffault) {                  \
    const uint32_t i = _map_find_or_fail_##N(h, key);                          \
    return i == _MAP_END_OF_LIST ? deffault : ARRAY_AT(&h->_data, i).value;    \
}                                                                              \
                                                                               \
static void map_set_##N(MAP_T(N) *h, uint64_t key, T value) {                  \
    if (ARRAY_SIZE(&h->_hash) == 0)                                            \
        _map_grow_##N(h);                                                      \
                                                                               \
    const uint32_t i = _map_find_or_make_##N(h, key);                          \
    ARRAY_AT(&h->_data, i).value = value;                                      \
                                                                               \
    if (_map_full_##N(h))                                                      \
        _map_grow_##N(h);                                                      \
}                                                                              \
                                                                               \
static void map_remove_##N(MAP_T(N) *h, uint64_t key) {                        \
    _map_find_and_erase_##N(h, key);                                           \
}                                                                              \
                                                                               \
static void map_reserve_##N(MAP_T(N) *h, uint32_t size) {                      \
    _map_remap_##N(h, size);                                                   \
}                                                                              \
                                                                               \
static void map_clear_##N(MAP_T(N) *h) {                                       \
    ARRAY_RESIZE(u32, &h->_hash, 0);                                           \
    ARRAY_RESIZE(map_entry_t_##N, &h->_data, 0);                               \
}                                                                              \
                                                                               \
static const map_entry_t_##N *map_begin_##N(MAP_T(N) *h) {                     \
    return ARRAY_BEGIN(&h->_data);                                             \
}                                                                              \
                                                                               \
static const map_entry_t_##N *map_end_##N(MAP_T(N) *h) {                       \
    return ARRAY_END(&h->_data);                                               \
}                                                                              \
                                                                               \
static const map_entry_t_##N *multimap_find_first_##N(MAP_T(N) *h, uint64_t key) {\
    const uint32_t i = _map_find_or_fail_##N(h, key);                          \
    return i == _MAP_END_OF_LIST ? 0 : &ARRAY_AT(&h->_data, i);                \
}                                                                              \
                                                                               \
static const map_entry_t_##N *multimap_find_next_##N(                          \
    MAP_T(N) *h,                                                               \
    const map_entry_t_##N *e                                                   \
) {                                                                            \
    uint32_t i = e->next;                                                      \
    while (i != _MAP_END_OF_LIST) {                                            \
        if (ARRAY_AT(&h->_data, i).key == e->key)                              \
            return &ARRAY_AT(&h->_data, i);                                    \
                                                                               \
        i = ARRAY_AT(&h->_data, i).next;                                       \
    }                                                                          \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static uint32_t multimap_count_##N(MAP_T(N) *h, uint64_t key) {                \
    uint32_t i = 0;                                                            \
    const map_entry_t_##N *e = multimap_find_first_##N(h, key);                \
    while (e) {                                                                \
        ++i;                                                                   \
        e = multimap_find_next_##N(h, e);                                      \
    }                                                                          \
    return i;                                                                  \
}                                                                              \
                                                                               \
static void multimap_get_##N(MAP_T(N) *h, uint64_t key, ARRAY_T(N) *items) {   \
    const map_entry_t_##N *e = multimap_find_first_##N(h, key);                \
    while (e) {                                                                \
        ARRAY_PUSH_BACK(N, items, e->value);                                   \
        e = multimap_find_next_##N(h, e);                                      \
    }                                                                          \
}                                                                              \
                                                                               \
static void multimap_insert_##N(MAP_T(N) *h, uint64_t key, T value) {          \
    if (ARRAY_SIZE(&h->_hash) == 0)                                            \
        _map_grow_##N(h);                                                      \
                                                                               \
    const uint32_t i = _map_make_##N(h, key);                                  \
    ARRAY_AT(&h->_data, i).value = value;                                      \
                                                                               \
    if (_map_full_##N(h))                                                      \
        _map_grow_##N(h);                                                      \
}                                                                              \
                                                                               \
static void multimap_remove_##N(MAP_T(N) *h, const map_entry_t_##N *e) {       \
    const struct map_find_result fr = _map_find_entry_##N(h, e);               \
    if (fr.data_i != _MAP_END_OF_LIST)                                         \
        _map_erase_##N(h, &fr);                                                \
}                                                                              \
                                                                               \
static void multimap_remove_all_##N(MAP_T(N) *h, uint64_t key) {               \
    while (map_has_##N(h, key))                                                \
        map_remove_##N(h, key);                                                \
}                                                                              \

MAP_PROTOTYPE_N(char*, pchar)

MAP_PROTOTYPE_N(void*, void)

MAP_PROTOTYPE(char)

MAP_PROTOTYPE(int)

MAP_PROTOTYPE(u8)

MAP_PROTOTYPE(u16)

MAP_PROTOTYPE(u32)

MAP_PROTOTYPE(u64)

MAP_PROTOTYPE(i8)

MAP_PROTOTYPE(i16)

MAP_PROTOTYPE(i32)

MAP_PROTOTYPE(i64)

#endif //CETECH_CONTAINERS_MAP_H
