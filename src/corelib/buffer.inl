#ifndef CETECH_BUFFER_H
#define CETECH_BUFFER_H

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "array.inl"

#define ct_buffer_clear(b) ct_array_clean(b)
#define ct_buffer_size(b) ct_array_size(b)
#define ct_buffer_push_n(a, items, n, alloc) ct_array_push_n(a, items, n, alloc)
#define ct_buffer_push_ch(a, ch, alloc) ct_array_push(a, ch, alloc)
#define ct_buffer_free(a, alloc) ct_array_free(a, alloc)

static inline int ct_buffer_printf(char **b,
                                    struct ct_alloc *alloc,
                                    const char *format,
                                    ...) CTECH_ATTR_FORMAT(3, 4);

static inline int ct_buffer_printf(char **b,
                                    struct ct_alloc *alloc,
                                    const char *format,
                                    ...){
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
