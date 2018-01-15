#ifndef CETECH_ARRAY_H
#define CETECH_ARRAY_H

#include <stdint.h>
#include <string.h>
#include <memory.h>

#include "allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

#define cel_array_free(a, alloc) \
    ((a) ?  CEL_FREE(alloc, cel_array_header(a)) : 0, a = NULL )

#define cel_array_header(a) \
    ((struct cel_array_header_t *)((char *)(a) - sizeof(struct cel_array_header_t)))

#define cel_array_size(a) \
    ((a) ? cel_array_header(a)->size : 0)

#define cel_array_capacity(a) \
     ((a) ? cel_array_header(a)->capacity : 0)

#define cel_array_clean(a) \
     ((a) ? cel_array_header(a)->size = 0 : 0)

#define cel_array_empty(a) (cel_array_size(a) == 0)
#define cel_array_any(a) (cel_array_size(a) > 0)

#define cel_array_full(a) \
    cel_array_full_n(a, 1)

#define cel_array_full_n(a, n) \
     ((a) ? (cel_array_size(a) + (n)) >= cel_array_capacity(a) : 1)


#define cel_array_push(a, item, alloc) \
    cel_array_full(a) ? (a) = (__typeof(a))cel_array_grow(a, 1, sizeof(*a), alloc) : 0, \
    (a)[cel_array_header(a)->size++] = item

#define cel_array_push_n(a, items, n, alloc) \
    cel_array_full_n(a, n) ? (a) =  (__typeof(a))cel_array_grow(a, n, sizeof(*(a)), alloc) : 0, \
    memcpy((a)+cel_array_header(a)->size, (items), sizeof(*(a)) * (n)), \
    cel_array_header(a)->size += n

#define cel_array_pop_front(a) \
    cel_array_any(a) ? memmove(a, ((a)+1), sizeof(*(a)) * (cel_array_header(a)->size--)) : 0

#define cel_array_pop_back(a) \
    cel_array_any(a) ? cel_array_header(a)->size-- : 0


#define cel_array_front(a) a[0]
#define cel_array_back(a) a[cel_array_size(a)-1]

struct cel_array_header_t {
    uint32_t size;
    uint32_t capacity;
};

static void *cel_array_grow(void *a,
                            uint32_t size,
                            size_t type_size,
                            struct cel_alloc *alloc) {
    uint32_t new_capacity = cel_array_capacity(a) * 2 + size;
    if (new_capacity < size) {
        new_capacity = size;
    }

    void *new_data = CEL_ALLOCATE(alloc, void*,
                                  sizeof(struct cel_array_header_t) +
                                  (new_capacity * type_size));

    char *new_array = (char *) new_data + sizeof(struct cel_array_header_t);

    *((struct cel_array_header_t *) new_data) = (struct cel_array_header_t) {
            .size = cel_array_size(a),
            .capacity = new_capacity
    };

    memcpy(new_array, a, type_size * cel_array_size(a));

    return new_array;
}

//static void* _ = (void*)&cel_array_grow; // UNUSED

#ifdef __cplusplus
}
#endif

#endif //CETECH_ARRAY_H
