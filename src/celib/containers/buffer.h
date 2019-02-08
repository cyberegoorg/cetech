//                             **Char buffer**
// # Description
// Helper function for [array](array.md.html) of type `char`.
//
// !!! ERROR: Buffer set to NULL before first use[.](https://www.monkeyuser.com/2018/debugging/)
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    char* array = NULL;
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// # Example
// ~~~~~~~~~~
// char* buffer = NULL;
// ce_buffer_printf(&buffer, ce_memory_a0–>system, "%d, %f, %s", 1, 1.5f, "foo");
//
// printf("%s", buffer); // 1, 1.5f, foo
//
// ce_buffer_free(buffer, ce_memory_a0–>system)
// ~~~~~~~~~~
//

#ifndef CE_BUFFER_H
#define CE_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"
#include "celib/containers/array.h"


// # Macro

// Buffer size
#define ce_buffer_size(b) \
     ce_array_size(b)

// Push n chars to buffer
#define ce_buffer_push_n(a, items, n, alloc) \
    ce_array_push_n(a, items, n, alloc)

// Push char to buffer
#define ce_buffer_push_ch(a, ch, alloc) \
    ce_array_push(a, ch, alloc)

// Clear buffer
#define ce_buffer_clear(b) \
    ce_array_clean(b)

// Free buffer
#define ce_buffer_free(a, alloc) \
    ce_array_free(a, alloc)

static inline int ce_buffer_printf(char **b,
                                   struct ce_alloc_t0 *alloc,
                                   const char *format,
                                   ...) CTECH_ATTR_FORMAT(3, 4);

// # Function

// printf *format*, *...* msg to buffer
static inline int ce_buffer_printf(char **b,
                                   struct ce_alloc_t0 *alloc,
                                   const char *format,
                                   ...) {
    va_list args;

    va_start(args, format);
    int n = vsnprintf(NULL, 0, format, args);
    va_end(args);

    uint32_t end = ce_array_size(*b);
    ce_array_resize(*b, end + n + 1, alloc);

    va_start(args, format);
    vsnprintf((*b) + end, n + 1, format, args);
    va_end(args);

    ce_array_resize(*b, end + n, alloc);

    return n;
}

#ifdef __cplusplus
};
#endif

#endif //CE_BUFFER_H
