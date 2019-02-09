//
//                      **Dynamic growing array**
// # Description
//
// ****************************************************
// *
// *  header              pointer  size  capacity
// *    |                   |       |       |
// *    v                   v       v       v
// *    +-----------------+---+---+---+---+---+
// *    | size | capacity | O | 1 | 2 |   |   |
// *    +-----------------+---+---+---+---+---+
// *
// *
// ****************************************************
//
// !!! ERROR: Array must be set to NULL before first use[.](https://www.monkeyuser.com/2018/debugging/)
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    TYPE* array = NULL;
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// # Example
//
// ## Classic int array
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// int* array = NULL;
//
// ce_array_push(array, 1, ce_memory_a0->system);
// ce_array_push(array, 2, ce_memory_a0->system);
// ce_array_push(array, 3, ce_memory_a0->system);
// ce_array_push(array, 4, ce_memory_a0->system);
//
// const uint32_t n = ce_array_size(array);
// for(uint32_t i = i; i < n; ++i) {
//     printf("%d\n", array[i]);
// }
//
// ce_array_free(array, ce_memory_a0->system);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ## Array as char buffer
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// char* buffer = NULL;
//
// ce_array_push(buffer, 'h', ce_memory_a0->system);
// ce_array_push(buffer, 'e', ce_memory_a0->system);
// ce_array_push(buffer, 'l', ce_memory_a0->system);
// ce_array_push(buffer, 'l', ce_memory_a0->system);
// ce_array_push(buffer, 'o', ce_memory_a0->system);
// ce_array_push(buffer, '\0', ce_memory_a0->system);
//
// printf("%s\n", buffer);
//
// ce_array_free(buffer, ce_memory_a0->system);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ## Array as string buffer
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// char* buffer = NULL;
//
// ce_array_push_n(buffer, "Hello", strlen("Hello"), ce_memory_a0->system);
// ce_array_push_n(buffer, ", ", strlen(", "), ce_memory_a0->system);
// ce_array_push_n(buffer, "world.", strlen("world."), ce_memory_a0->system);
//
// printf("%s\n", buffer); // hello, world.
//
// ce_array_free(buffer, ce_memory_a0->system);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// ## Struct memory "pool"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// struct object {
//     uint64 id;
// }
//
// object* pool = NULL;
//
// static uint32_t _new_object(struct object obj) {
//     uint32_t idx = ce_array_size(pool);
//     ce_array_push(pool, obj, ce_memory_a0->system);
//     return idx;
// }
//
// uint32_t obj1 = _new_object((struct object){.id = 1}));
// uint32_t obj2 = _new_object((struct object){.id = 2}));
// uint32_t obj3 = _new_object((struct object){.id = 3}));
//
// printf("%d\n", pool[obj1].id); // 1
// printf("%d\n", pool[obj2].id); // 2
// printf("%d\n", pool[obj3].id); // 3
//
// ce_array_free(pool, ce_memory_a0->system);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#ifndef CE_ARRAY_H
#define CE_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

// # Struct

// Array header (pointer preambule)
typedef struct ce_array_header_t {
    uint32_t size;
    uint32_t capacity;
}ce_array_header_t;

// # Macro

// Free array.
#define ce_array_free(a, alloc) \
    ((a) ?  CE_FREE(alloc, ce_array_header(a)) : 0, a = NULL )

// Get array [header](#ce_array_header_t).
#define ce_array_header(a) \
    ((a) ? (struct ce_array_header_t *)((char *)(a) - sizeof(ce_array_header_t)): NULL)

// Get array size.
#define ce_array_size(a) \
    ((a) ? ce_array_header(a)->size : 0)

// Get array capacity.
#define ce_array_capacity(a) \
     ((a) ? ce_array_header(a)->capacity : 0)

// Clean array (only set size = 0)
#define ce_array_clean(a) \
     ((a) ? ce_array_header(a)->size = 0 : 0)

// Set array capacity
#define ce_array_set_capacity(a, c, alloc) \
     ((a) = (__typeof(a)) ce_array_grow(a, c, sizeof(*(a)), CE_ALIGNOF(*(a)), alloc, __FILE__, __LINE__))

// Resize array
#define ce_array_resize(a, c, alloc) \
     (ce_array_set_capacity(a, c, alloc), ce_array_header(a)->size = (c))

// Is array emty?
#define ce_array_empty(a) \
    (ce_array_size(a) == 0)

// Has array any element?
#define ce_array_any(a) \
    (ce_array_size(a) > 0)

// Is array full?
#define ce_array_full(a) \
    ce_array_full_n(a, 1)

// Is array full for n elemenets?
#define ce_array_full_n(a, n) \
     ((a) ? (ce_array_size(a) + (n)) >= ce_array_capacity(a) : 1)

// Push value to array
#define ce_array_push(a, item, alloc) \
    (ce_array_full(a) ? (a) = (__typeof(a))ce_array_grow(a, ce_array_size(a) + 1, sizeof(*(a)), CE_ALIGNOF(*(a)), alloc, __FILE__, __LINE__) : 0, \
    (a)[ce_array_header(a)->size++] = item)

// Push n values from items to array
#define ce_array_push_n(a, items, n, alloc) \
    (ce_array_full_n(a, n) ? (a) =  (__typeof(a))ce_array_grow(a, ce_array_size(a) + (n), sizeof(*(a)), CE_ALIGNOF(*(a)), alloc, __FILE__, __LINE__) : 0, \
    memcpy((a)+ce_array_header(a)->size, (items), sizeof(*(a)) * (n)), \
    ce_array_header(a)->size += (n))

// Pop element from front
#define ce_array_pop_front(a) \
    (ce_array_any(a) ? memmove(a, ((a)+1), sizeof(*(a)) * (ce_array_header(a)->size--)) : 0)

// Pop element from back
#define ce_array_pop_back(a) \
    (ce_array_any(a) ? ce_array_header(a)->size-- : 0)

// Insert element before element at *idx*
#define ce_array_insert(a, idx, v, alloc) \
    do {\
        ce_array_resize(a, ce_array_size(a) + 1, alloc);\
        memmove(a+idx+1,a+idx, (ce_array_size(a)-idx)*sizeof(*(a)));\
        a[idx] = v;\
    } while(0)

// Get front element
#define ce_array_front(a) \
    a[0]

// Get back element
#define ce_array_back(a) \
    a[ce_array_size(a)-1]

// # Function

// Grow array if need.
static inline void *ce_array_grow(void *array,
                                  uint32_t capacity,
                                  size_t type_size,
                                  size_t type_align,
                                  const ce_alloc_t0 *alloc,
                                  const char *filename,
                                  uint32_t line) {
    if (capacity < ce_array_capacity(array)) {
        return array;
    }

    uint32_t new_capacity = (ce_array_capacity(array) * 2) + 8;
    if (new_capacity < capacity) {
        new_capacity = capacity;
    }

    const uint32_t orig_size = ce_array_size(array);

    const uint32_t size = sizeof(struct ce_array_header_t) + (new_capacity
                                                              * type_size);

    void *new_data = alloc->reallocate(alloc, ce_array_header(array),
                                       size, type_align, filename, line);

    char *new_array = (char *) new_data + sizeof(struct ce_array_header_t);

    *((struct ce_array_header_t *) new_data) = (struct ce_array_header_t) {
            .size = orig_size,
            .capacity = new_capacity
    };

    return new_array;
}

// Clone array.
static inline void *ce_array_clone(void *array,
                                   size_t type_size,
                                   const ce_alloc_t0 *alloc) {
    char *new_array = NULL;
    ce_array_push_n(new_array, array, ce_array_size(array) * type_size, alloc);
    return new_array;
}

#ifdef __cplusplus
};
#endif

#endif //CE_ARRAY_H
