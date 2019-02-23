//
//                      **Bitset**
// # Description


#ifndef CE_BITSET_H
#define CE_BITSET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

// # Struct
typedef struct ce_bitset_t0 {
    uint64_t *slots;
    uint64_t n;
} ce_bitset_t0;

// # Function

// Init bitset for max *n* items.
static inline void ce_bitset_init(ce_bitset_t0 *bitset,
                                  uint64_t n,
                                  ce_alloc_t0 *alloc) {
    uint64_t slot_num = n/64;
    ce_array_resize(bitset->slots, slot_num, alloc);
    bitset->n = n;
}

// Free bitset.
static inline void ce_bitset_free(ce_bitset_t0 *bitset,
                                  ce_alloc_t0 *alloc) {
    ce_array_free(bitset->slots, alloc);
    bitset->n = 0;
}

// Set contain key
static inline void ce_bitset_set(ce_bitset_t0 *bitset,
                                 uint64_t key) {
    uint64_t idx = key % bitset->n;

    uint64_t slot_idx = idx / 64;
    uint64_t bit_idx = idx % 64;

    uint64_t bit_mask = 1UL << bit_idx;

    bitset->slots[slot_idx] |= bit_mask;
}

// Uset contain idx
static inline void ce_bitset_unset(ce_bitset_t0 *bitset,
                                   uint64_t key) {
    uint64_t idx = key % bitset->n;

    uint64_t slot_idx = idx / 64;
    uint64_t bit_idx = idx % 64;

    uint64_t bit_mask = ~(1UL << bit_idx);

    bitset->slots[slot_idx] &= bit_mask;
}

// Contain key?
static inline bool ce_bitset_is_set(ce_bitset_t0 *bitset,
                                    uint64_t key) {
    uint64_t idx = key % bitset->n;

    uint64_t slot_idx = idx / 64;
    uint64_t bit_idx = idx % 64;

    uint64_t bit_mask = 1UL << bit_idx;

    return (bitset->slots[slot_idx] & bit_mask) > 0;
}

#ifdef __cplusplus
};
#endif

#endif //CE_BITSET_H
