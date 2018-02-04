#ifndef CETECH_MACROS_H
#define CETECH_MACROS_H

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

//==============================================================================
// Macros
//==============================================================================

#define CETECH_UNLIKELY(x) __builtin_expect(!!(x), 0)

#define CETECH_ARRAY_LEN(_name) (sizeof(_name) / sizeof(_name[0]))

#define CETECH_ARR_ARG(a) a, CETECH_ARRAY_LEN(a)

#define CTECH_ATTR_FORMAT(fmt, args) __attribute__ ((format(printf, fmt, args)))

#define CETECH_STATIC_ASSERT_MSG(expr, msg) _Static_assert(expr, msg)


#endif //CETECH_MACROS_H
