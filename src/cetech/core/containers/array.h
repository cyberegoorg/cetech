#ifndef CETECH_ARRAY_H
#define CETECH_ARRAY_H

#include <stdint.h>
#include <string.h>
#include <memory.h>

#include "cetech/core/memory/allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ct_array_free(a, alloc) \
    ((a) ?  CT_FREE(alloc, ct_array_header(a)) : 0, a = NULL )

#define ct_array_header(a) \
    ((struct ct_array_header_t *)((char *)(a) - sizeof(struct ct_array_header_t)))

#define ct_array_size(a) \
    ((a) ? ct_array_header(a)->size : 0)

#define ct_array_capacity(a) \
     ((a) ? ct_array_header(a)->capacity : 0)

#define ct_array_clean(a) \
     ((a) ? ct_array_header(a)->size = 0 : 0)

#define ct_array_set_capacity(a, c, alloc) \
     ((a) = (__typeof(a)) ct_array_grow(a, c, sizeof(*(a)), alloc))

#define ct_array_resize(a, c, alloc) \
     (ct_array_set_capacity(a, c, alloc), ct_array_header(a)->size = (c))

#define ct_array_empty(a) (ct_array_size(a) == 0)
#define ct_array_any(a) (ct_array_size(a) > 0)

#define ct_array_full(a) \
    ct_array_full_n(a, 1)

#define ct_array_full_n(a, n) \
     ((a) ? (ct_array_size(a) + (n)) >= ct_array_capacity(a) : 1)


#define ct_array_push(a, item, alloc) \
    ct_array_full(a) ? (a) = (__typeof(a))ct_array_grow(a, ct_array_size(a) + 1, sizeof(*(a)), alloc) : 0, \
    (a)[ct_array_header(a)->size++] = item

#define ct_array_push_n(a, items, n, alloc) \
    ct_array_full_n(a, n) ? (a) =  (__typeof(a))ct_array_grow(a, ct_array_size(a) + n, sizeof(*(a)), alloc) : 0, \
    memcpy((a)+ct_array_header(a)->size, (items), sizeof(*(a)) * (n)), \
    ct_array_header(a)->size += (n)

#define ct_array_pop_front(a) \
    (ct_array_any(a) ? memmove(a, ((a)+1), sizeof(*(a)) * (ct_array_header(a)->size--)) : 0)

#define ct_array_pop_back(a) \
    (ct_array_any(a) ? ct_array_header(a)->size-- : 0)


#define ct_array_front(a) a[0]
#define ct_array_back(a) a[ct_array_size(a)-1]

struct ct_array_header_t {
    uint32_t size;
    uint32_t capacity;
};

static void *ct_array_grow(void *a,
                           uint32_t capacity,
                           size_t type_size,
                           const struct ct_alloc *alloc) {
    if (capacity <= ct_array_capacity(a)) {
        return a;
    }

    uint32_t new_capacity = ct_array_capacity(a) * 2 + 8;
    if (new_capacity < capacity) {
        new_capacity = capacity;
    }

    const uint32_t size = sizeof(struct ct_array_header_t) +
                          (new_capacity * type_size);

    void *new_data = CT_ALLOC(alloc, void*, size);
    char *new_array = (char *) new_data + sizeof(struct ct_array_header_t);

    memset(new_data, 0, size);

    *((struct ct_array_header_t *) new_data) = (struct ct_array_header_t) {
            .size = ct_array_size(a),
            .capacity = new_capacity
    };

    memcpy(new_array, a, type_size * ct_array_size(a));

    return new_array;
}

static void *_ = (void *) &ct_array_grow; // UNUSED

#ifdef __cplusplus
}
#endif

#endif //CETECH_ARRAY_H
