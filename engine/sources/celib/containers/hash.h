#ifndef CETECH_CONTAINERS_HASH_H
#define CETECH_CONTAINERS_HASH_H

//==============================================================================
// Includes
//==============================================================================

#include "array.h"


//==============================================================================
// defines
//==============================================================================

const uint32_t _HASH_END_OF_LIST = 0xffffffffu;


//==============================================================================
// Structs
//==============================================================================

struct hash_find_result {
    uint32_t hash_i;
    uint32_t data_prev;
    uint32_t data_i;
};


//==============================================================================
// Interface macros
//==============================================================================

#define HASH_T(T) struct Hash_##T
#define HASH_ENTRY_T(T) struct hash_entry_##T

#define HASH_INIT(T, h, a) hash_init_##T(h, a)
#define HASH_DESTROY(T, h) hash_destroy_##T(h)

#define HASH_HAS(T, h, key)           hash_has_##T(h, key)
#define HASH_GET(T, h, key, deffault) hash_get_##T(h, key, deffault)
#define HASH_SET(T, h, key, value)    hash_set_##T(h, key, value)
#define HASH_REMOVE(T, h, key)        hash_remove_##T(h, key)
#define HASH_RESERVE(T, h, size)      hash_reserve_##T(h, size)
#define HASH_CLEAR(T, h)              hash_clear_##T(h)
#define HASH_BEGIN(T, h)              hash_begin_##T(h)
#define HASH_END(T, h)                hash_end_##T(h)

#define MULTIHASH_FIND_FIRST(T, h, key)    multihash_find_first_##T(h, key)
#define MULTIHASH_FIND_NEXT(T, h, e)       multihash_find_next_##T(h, e)
#define MULTIHASH_COUNT(T, h, key)         multihash_count_##T(h, key)
#define MULTIHASH_GET(T, h, key, items)    multihash_get_##T(h, key, items)
#define MULTIHASH_INSERT(T, h, key, value) multihash_insert_##T(h, key, value)
#define MULTIHASH_REMOVE(T, h, key, e)     multihash_remove_##T(h, e)
#define MULTIHASH_REMOVE_ALLHAS(T, h, key) multihash_remove_all_##T(h, key)


//==============================================================================
// Prototypes macro
//==============================================================================

#define HASH_PROTOTYPE(T)                                                      \
                                                                               \
struct hash_entry_##T {                                                        \
    u64 key;                                                                   \
    u32 next;                                                                  \
    T value;                                                                   \
};                                                                             \
                                                                               \
typedef struct hash_entry_##T hash_entry_t_##T;                                \
ARRAY_PROTOTYPE(hash_entry_t_##T);                                             \
struct Hash_##T {                                                              \
    ARRAY_T(u32) _hash;                                                        \
    ARRAY_T(hash_entry_t_##T) _data;                                           \
};                                                                             \
                                                                               \
static void hash_init_##T(HASH_T(T) *h, struct allocator *allocator) {         \
    ARRAY_INIT(u32, &h->_hash, allocator);                                     \
    ARRAY_INIT(hash_entry_t_##T, &h->_data, allocator);                        \
}                                                                              \
                                                                               \
static void hash_destroy_##T(HASH_T(T) *h) {                                   \
    ARRAY_DESTROY(u32, &h->_hash);                                             \
    ARRAY_DESTROY(hash_entry_t_##T, &h->_data);                                \
}                                                                              \
                                                                               \
                                                                               \
static uint32_t _hash_add_entry_##T(HASH_T(T) *h, uint64_t key) {              \
    hash_entry_t_##T e;                                                        \
    e.key = key;                                                               \
    e.next = _HASH_END_OF_LIST;                                                \
                                                                               \
    uint32_t ei = ARRAY_SIZE(&h->_data);                                       \
    ARRAY_PUSH_BACK(hash_entry_t_##T, &h->_data, e);                           \
                                                                               \
    return ei;                                                                 \
}                                                                              \
                                                                               \
static struct hash_find_result _hash_find_key_##T(HASH_T(T) *h, uint64_t key) {\
    struct hash_find_result fr;                                                \
    fr.hash_i = _HASH_END_OF_LIST;                                             \
    fr.data_prev = _HASH_END_OF_LIST;                                          \
    fr.data_i = _HASH_END_OF_LIST;                                             \
                                                                               \
    if (ARRAY_SIZE(&h->_hash) == 0)                                            \
        return fr;                                                             \
                                                                               \
    fr.hash_i = key % ARRAY_SIZE(&h->_hash);                                   \
    fr.data_i = ARRAY_AT(&h->_hash, fr.hash_i);                                \
    while (fr.data_i != _HASH_END_OF_LIST) {                                   \
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
static void _hash_erase_##T(HASH_T(T) *h, const struct hash_find_result *fr) { \
    if (fr->data_prev == _HASH_END_OF_LIST)                                    \
        ARRAY_AT(&h->_hash, fr->hash_i) = ARRAY_AT(&h->_data, fr->data_i).next;\
    else                                                                       \
        ARRAY_AT(&h->_data, fr->data_prev).next = ARRAY_AT(&h->_data,          \
                                                           fr->data_i).next;   \
                                                                               \
    if (fr->data_i == ARRAY_SIZE(&h->_data) - 1) {                             \
        ARRAY_POP_BACK(hash_entry_t_##T, &h->_data);                           \
        return;                                                                \
    }                                                                          \
                                                                               \
    ARRAY_AT(&h->_data, fr->data_i) = ARRAY_AT(&h->_data,                      \
                                               ARRAY_SIZE(&h->_data) - 1);     \
                                                                               \
    struct hash_find_result last = _hash_find_key_##T(h, ARRAY_AT(&h->_data, fr->data_i).key);\
                                                                               \
    if (last.data_prev != _HASH_END_OF_LIST)                                   \
        ARRAY_AT(&h->_data, last.data_prev).next = fr->data_i;                 \
    else                                                                       \
        ARRAY_AT(&h->_hash, last.hash_i) = fr->data_i;                         \
}                                                                              \
                                                                               \
static struct hash_find_result _hash_find_entry_##T(HASH_T(T) *h, const hash_entry_t_##T *e) {\
    struct hash_find_result fr;                                                \
    fr.hash_i = _HASH_END_OF_LIST;                                             \
    fr.data_prev = _HASH_END_OF_LIST;                                          \
    fr.data_i = _HASH_END_OF_LIST;                                             \
                                                                               \
    if (ARRAY_SIZE(&h->_hash) == 0)                                            \
        return fr;                                                             \
                                                                               \
    fr.hash_i = e->key % ARRAY_SIZE(&h->_hash);                                \
    fr.data_i = ARRAY_AT(&h->_hash, fr.hash_i);                                \
    while (fr.data_i != _HASH_END_OF_LIST) {                                   \
        if (&ARRAY_AT(&h->_data, fr.data_i) == e)                              \
            return fr;                                                         \
                                                                               \
        fr.data_prev = fr.data_i;                                              \
        fr.data_i = ARRAY_AT(&h->_data, fr.data_i).next;                       \
    }                                                                          \
    return fr;                                                                 \
}                                                                              \
                                                                               \
static uint32_t _hash_find_or_fail_##T(HASH_T(T) *h, uint64_t key) {           \
    return _hash_find_key_##T(h, key).data_i;                                  \
}                                                                              \
                                                                               \
static uint32_t _hash_find_or_make_##T(HASH_T(T) *h, uint64_t key) {           \
    const struct hash_find_result fr = _hash_find_key_##T(h, key);             \
    if (fr.data_i != _HASH_END_OF_LIST)                                        \
        return fr.data_i;                                                      \
                                                                               \
    uint32_t i = _hash_add_entry_##T(h, key);                                  \
    if (fr.data_prev == _HASH_END_OF_LIST)                                     \
        ARRAY_AT(&h->_hash, fr.hash_i) = i;                                    \
    else                                                                       \
        ARRAY_AT(&h->_data, fr.data_prev).next = i;                            \
                                                                               \
    return i;                                                                  \
}                                                                              \
                                                                               \
static uint32_t _hash_make_##T(HASH_T(T) *h, uint64_t key) {                   \
    const struct hash_find_result fr = _hash_find_key_##T(h, key);             \
    const uint32_t i = _hash_add_entry_##T(h, key);                            \
                                                                               \
    if (fr.data_prev == _HASH_END_OF_LIST)                                     \
        ARRAY_AT(&h->_hash, fr.hash_i) = i;                                    \
                                                                               \
    else                                                                       \
        ARRAY_AT(&h->_data, fr.data_prev).next = i;                            \
                                                                               \
    ARRAY_AT(&h->_data, i).next = fr.data_i;                                   \
                                                                               \
    return i;                                                                  \
}                                                                              \
                                                                               \
static void _hash_find_and_erase_##T(HASH_T(T) *h, uint64_t key) {             \
    const struct hash_find_result fr = _hash_find_key_##T(h, key);             \
    if (fr.data_i != _HASH_END_OF_LIST)                                        \
        _hash_erase_##T(h, &fr);                                               \
}                                                                              \
                                                                               \
static void multihash_insert_##T(HASH_T(T) *h, uint64_t key, const T value);   \
                                                                               \
static void _hash_rehash_##T(HASH_T(T) *h, uint32_t new_size) {                \
    HASH_T(T) nh;                                                              \
    hash_init_##T(&nh, h->_hash.allocator);                                    \
                                                                               \
    ARRAY_RESIZE(u32, &nh._hash, new_size);                                    \
    ARRAY_RESERVE(hash_entry_t_##T, &nh._data, ARRAY_SIZE(&h->_data));         \
                                                                               \
    for (uint32_t i = 0; i < new_size; ++i)                                    \
        ARRAY_AT(&nh._hash, i) = _HASH_END_OF_LIST;                            \
                                                                               \
    for (uint32_t i = 0; i < ARRAY_SIZE(&h->_data); ++i) {                     \
        const hash_entry_t_##T e = ARRAY_AT(&h->_data, i);                     \
                                                                               \
        multihash_insert_##T(&nh, e.key, e.value);                             \
    }                                                                          \
                                                                               \
    HASH_T(T) empty;                                                           \
    hash_init_##T(&empty, h->_hash.allocator);                                 \
                                                                               \
    hash_destroy_##T(h);                                                       \
    memory_copy(h, &nh, sizeof(HASH_T(T)));                                    \
    memory_copy(&nh, &empty, sizeof(HASH_T(T)));                               \
}                                                                              \
                                                                               \
static int _hash_full_##T(HASH_T(T) *h) {                                      \
    const float max_load_factor = 0.7f;                                        \
    return ARRAY_SIZE(&h->_data) >= ARRAY_SIZE(&h->_hash) * max_load_factor;   \
}                                                                              \
                                                                               \
static void _hash_grow_##T(HASH_T(T) *h) {                                     \
    const uint32_t new_size = ARRAY_SIZE(&h->_data) * 2 + 10;                  \
    _hash_rehash_##T(h, new_size);                                             \
}                                                                              \
                                                                               \
static int hash_has_##T(HASH_T(T) *h, uint64_t key) {                          \
    return _hash_find_or_fail_##T(h, key) != _HASH_END_OF_LIST;                \
}                                                                              \
                                                                               \
static const T hash_get_##T(HASH_T(T) *h, uint64_t key, const T deffault) {    \
    const uint32_t i = _hash_find_or_fail_##T(h, key);                         \
    return i == _HASH_END_OF_LIST ? deffault : ARRAY_AT(&h->_data, i).value;   \
}                                                                              \
                                                                               \
static void hash_set_##T(HASH_T(T) *h, uint64_t key, const T value) {          \
    if (ARRAY_SIZE(&h->_hash) == 0)                                            \
        _hash_grow_##T(h);                                                     \
                                                                               \
    const uint32_t i = _hash_find_or_make_##T(h, key);                         \
    ARRAY_AT(&h->_data, i).value = value;                                      \
                                                                               \
    if (_hash_full_##T(h))                                                     \
        _hash_grow_##T(h);                                                     \
}                                                                              \
                                                                               \
static void hash_remove_##T(HASH_T(T) *h, uint64_t key) {                      \
    _hash_find_and_erase_##T(h, key);                                          \
}                                                                              \
                                                                               \
static void hash_reserve_##T(HASH_T(T) *h, uint32_t size) {                    \
    _hash_rehash_##T(h, size);                                                 \
}                                                                              \
                                                                               \
static void hash_clear_##T(HASH_T(T) *h) {                                     \
    ARRAY_RESIZE(u32, &h->_hash, 0);                                           \
    ARRAY_RESIZE(hash_entry_t_##T, &h->_data, 0);                              \
}                                                                              \
                                                                               \
static const hash_entry_t_##T *hash_begin_##T(HASH_T(T) *h) {                  \
    return ARRAY_BEGIN(&h->_data);                                             \
}                                                                              \
                                                                               \
static const hash_entry_t_##T *hash_end_##T(HASH_T(T) *h) {                    \
    return ARRAY_END(&h->_data);                                               \
}                                                                              \
                                                                               \
static const hash_entry_t_##T *multihash_find_first_##T(HASH_T(T) *h, uint64_t key) {                                                                                                                       \
    const uint32_t i = _hash_find_or_fail_##T(h, key);                         \
    return i == _HASH_END_OF_LIST ? 0 : &ARRAY_AT(&h->_data, i);               \
}                                                                              \
                                                                               \
static const hash_entry_t_##T *multihash_find_next_##T(HASH_T(T) *h, const hash_entry_t_##T *e) {\
    uint32_t i = e->next;                                                      \
    while (i != _HASH_END_OF_LIST) {                                           \
        if (ARRAY_AT(&h->_data, i).key == e->key)                              \
            return &ARRAY_AT(&h->_data, i);                                    \
                                                                               \
        i = ARRAY_AT(&h->_data, i).next;                                       \
    }                                                                          \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static uint32_t multihash_count_##T(HASH_T(T) *h, uint64_t key) {              \
    uint32_t i = 0;                                                            \
    const hash_entry_t_##T *e = multihash_find_first_##T(h, key);              \
    while (e) {                                                                \
        ++i;                                                                   \
        e = multihash_find_next_##T(h, e);                                     \
    }                                                                          \
    return i;                                                                  \
}                                                                              \
                                                                               \
static void multihash_get_##T(HASH_T(T) *h, uint64_t key, ARRAY_T(int) *items) {\
    const hash_entry_t_##T *e = multihash_find_first_##T(h, key);              \
    while (e) {                                                                \
        ARRAY_PUSH_BACK(int, items, e->value);                                 \
        e = multihash_find_next_##T(h, e);                                     \
    }                                                                          \
}                                                                              \
                                                                               \
static void multihash_insert_##T(HASH_T(T) *h, uint64_t key, const T value) {  \
    if (ARRAY_SIZE(&h->_hash) == 0)                                            \
        _hash_grow_##T(h);                                                     \
                                                                               \
    const uint32_t i = _hash_make_##T(h, key);                                 \
    ARRAY_AT(&h->_data, i).value = value;                                      \
                                                                               \
    if (_hash_full_##T(h))                                                     \
        _hash_grow_##T(h);                                                     \
}                                                                              \
                                                                               \
static void multihash_remove_##T(HASH_T(T) *h, const hash_entry_t_##T *e) {    \
    const struct hash_find_result fr = _hash_find_entry_##T(h, e);             \
    if (fr.data_i != _HASH_END_OF_LIST)                                        \
        _hash_erase_##T(h, &fr);                                               \
}                                                                              \
                                                                               \
static void multihash_remove_all_##T(HASH_T(T) *h, uint64_t key) {             \
    while (hash_has_##T(h, key))                                               \
        hash_remove_##T(h, key);                                               \
}                                                                              \

HASH_PROTOTYPE(int);

#endif //CETECH_CONTAINERS_HASH_H
