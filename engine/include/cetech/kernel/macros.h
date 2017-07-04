#ifndef CETECH_TYPES_H
#define CETECH_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

//==============================================================================
// Macros
//==============================================================================

#define CETECH_ALIGNOF(type) __alignof(type)

#define CETECH_RESTRICT __restrict

#define CETECH_FORCE_INLINE __extension__ static __inline __attribute__( (__always_inline__) )
#define CETECH_PRETTY_FUNCTION __PRETTY_FUNCTION__
#define CETECH_LIKELY(x)   __builtin_expect(!!(x), 1)
#define CETECH_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define CETECH_NO_INLINE __attribute__( (noinline) )
#define CETECH_NO_RETURN __attribute__( (noreturn) )

#define CETECH_STR_(x) #x
#define CETECH_STR(x) CETECH_STR_(x)

#define CETECH_CONCATE_(x, y) x ## y
#define CETECH_CONCATE(x, y) CETECH_CONCATE_(x, y)

#define CETECH_CLEANUP_ATTR(fce) __attribute__((cleanup(fce)))

#define CETECH_ARRAY_LEN(_name) (sizeof(_name) / sizeof(_name[0]))

#define ATTR_FORMAT(fmt, args) __attribute__ ((format(printf, fmt, args)))

#define CETECH_STATIC_ASSERT_MSG(expr, msg) _Static_assert(expr, msg)
#define CETECH_STATIC_ASSERT(expr) CETECH_STATIC_ASSERT_MSG(expr, #expr)


#endif //CETECH_TYPES_H
