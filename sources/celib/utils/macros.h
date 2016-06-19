#ifndef CETECH_MACROS_H
#define CETECH_MACROS_H

#define CE_ALIGNOF(type) __alignof(type)

#define CE_RESTRICT __restrict

#define CE_FORCE_INLINE __extension__ static __inline __attribute__( (__always_inline__) )
#define CE_FUNCTION __PRETTY_FUNCTION__
#define CE_LIKELY(x)   __builtin_expect(!!(x), 1)
#define CE_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define CE_NO_INLINE __attribute__( (noinline) )
#define CE_NO_RETURN __attribute__( (noreturn) )

#define CE_STR_(x) #x
#define CE_STR(x) CE_STR_(x)

#define CE_CONCATE_(x, y) x ## y
#define CE_CONCATE(x, y) CE_CONCATE_(x, y)

#define CE_CLEANUP_ATTR(fce) __attribute__((cleanup(fce)))

#endif //CETECH_MACROS_H
