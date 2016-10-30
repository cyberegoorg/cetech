#ifndef CELIB_TYPES_H
#define CELIB_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

//==============================================================================
// Macros
//==============================================================================

#define CE_ALIGNOF(type) __alignof(type)

#define CE_RESTRICT __restrict

#define CE_FORCE_INLINE __extension__ static __inline __attribute__( (__always_inline__) )
#define CE_PRETTY_FUNCTION __PRETTY_FUNCTION__
#define CE_LIKELY(x)   __builtin_expect(!!(x), 1)
#define CE_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define CE_NO_INLINE __attribute__( (noinline) )
#define CE_NO_RETURN __attribute__( (noreturn) )

#define CE_STR_(x) #x
#define CE_STR(x) CE_STR_(x)

#define CE_CONCATE_(x, y) x ## y
#define CE_CONCATE(x, y) CE_CONCATE_(x, y)

#define CE_CLEANUP_ATTR(fce) __attribute__((cleanup(fce)))

#define CE_ARRAY_LEN(_name) (sizeof(_name) / sizeof(_name[0]))

#define ATTR_FORMAT(fmt, args) __attribute__ ((format(printf, fmt, args)))

#define CE_STATIC_ASSERT_MSG(expr, msg) _Static_assert(expr, msg)
#define CE_STATIC_ASSERT(expr) CE_STATIC_ASSERT_MSG(expr, #expr)


//==============================================================================
// Base types
//==============================================================================

typedef char *string_t;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#endif //CELIB_TYPES_H
