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
// ct_buffer_printf(&buffer, ct_memory_a0–>system, "%d, %f, %s", 1, 1.5f, "foo");
//
// printf("%s", buffer); // 1, 1.5f, foo
//
// ct_buffer_free(buffer, ct_memory_a0–>system)
// ~~~~~~~~~~
//

#ifndef CETECH_BUFFER_H
#define CETECH_BUFFER_H

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "array.inl"

// # Macro

// Buffer size
#define ct_buffer_size(b) \
     ct_array_size(b)

// Push n chars to buffer
#define ct_buffer_push_n(a, items, n, alloc) \
    ct_array_push_n(a, items, n, alloc)

// Push char to buffer
#define ct_buffer_push_ch(a, ch, alloc) \
    ct_array_push(a, ch, alloc)

// Clear buffer
#define ct_buffer_clear(b) \
    ct_array_clean(b)

// Free buffer
#define ct_buffer_free(a, alloc) \
    ct_array_free(a, alloc)

static inline int ct_buffer_printf(char **b,
                                   struct ct_alloc *alloc,
                                   const char *format,
                                   ...) CTECH_ATTR_FORMAT(3, 4);

// # Function

// printf *format*, *...* msg to buffer
static inline int ct_buffer_printf(char **b,
                                   struct ct_alloc *alloc,
                                   const char *format,
                                   ...) {
    va_list args;

    va_start(args, format);
    int n = vsnprintf(NULL, 0, format, args);
    va_end(args);

    uint32_t end = ct_array_size(*b);
    ct_array_resize(*b, end + n + 1, alloc);

    va_start(args, format);
    vsnprintf((*b) + end, n + 1, format, args);
    va_end(args);

    ct_array_resize(*b, end + n, alloc);

    return n;
}

#endif //CETECH_BUFFER_H
