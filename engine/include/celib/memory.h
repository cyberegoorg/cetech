/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef CEL_MEMORY_H
#define CEL_MEMORY_H

#include <alloca.h> // alloca
#include <stdarg.h> // va_list
#include <stdint.h> // uint32_t
#include <stdlib.h> // size_t
#include <stddef.h> // ptrdiff_t

#if !CEL_CRT_NONE

#	include <string.h> // memcpy, memmove, memset

#endif // !CEL_CRT_NONE


#include "macros.h"

///
#define CEL_COUNTOF(_x) sizeof(celib::COUNTOF_REQUIRES_ARRAY_ARGUMENT(_x) )

///
#define CEL_IGNORE_C4127(_x) celib::ignoreC4127(!!(_x) )

///
#define CEL_ENABLED(_x) celib::isEnabled<!!(_x)>()

namespace celib {
    /// Template for avoiding MSVC: C4127: conditional expression is constant
    template<bool>
    inline bool isEnabled();

    inline bool ignoreC4127(bool _x);

    template<typename Ty>
    inline void xchg(Ty &_a,
                     Ty &_b);

    inline void xchg(void *_a,
                     void *_b,
                     size_t _numBytes);

    // http://cnicholson.net/2011/01/stupid-c-tricks-a-better-sizeof_array/
    template<typename T, size_t N>
    inline char (&COUNTOF_REQUIRES_ARRAY_ARGUMENT(const T(&)[N]))[N];

    inline void mem_copy(void *_dst,
                         const void *_src,
                         size_t _numBytes);

    inline void mem_copy(void *_dst,
                         const void *_src,
                         uint32_t _size,
                         uint32_t _num,
                         uint32_t _srcPitch,
                         uint32_t _dstPitch);

    inline void gather(void *_dst,
                       const void *_src,
                       uint32_t _size,
                       uint32_t _num,
                       uint32_t _srcPitch);

    inline void scatter(void *_dst,
                        const void *_src,
                        uint32_t _size,
                        uint32_t _num,
                        uint32_t _dstPitch);

    inline void mem_move(void *_dst,
                         const void *_src,
                         size_t _numBytes);

    inline void mem_set(void *_dst,
                        uint8_t _ch,
                        size_t _numBytes);

    inline int32_t mem_cmp(const void *_lhs,
                           const void *_rhs,
                           size_t _numBytes);
}

namespace celib {
    template<bool>
    inline bool isEnabled()
    {
        return true;
    }

    template<>
    inline bool isEnabled<false>()
    {
        return false;
    }

    inline bool ignoreC4127(bool _x)
    {
        return _x;
    }

    template<typename Ty>
    inline void xchg(Ty& _a, Ty& _b)
    {
        Ty tmp = _a; _a = _b; _b = tmp;
    }

    inline void xchg(void *_a,
                     void *_b,
                     size_t _numBytes) {
        uint8_t *lhs = (uint8_t *) _a;
        uint8_t *rhs = (uint8_t *) _b;
        const uint8_t *end = rhs + _numBytes;
        while (rhs != end) {
            xchg(*lhs++, *rhs++);
        }
    }

    inline void memCopyRef(void *_dst,
                           const void *_src,
                           size_t _numBytes) {
        uint8_t *dst = (uint8_t *) _dst;
        const uint8_t *end = dst + _numBytes;
        const uint8_t *src = (const uint8_t *) _src;
        while (dst != end) {
            *dst++ = *src++;
        }
    }

    inline void mem_copy(void *_dst,
                         const void *_src,
                         size_t _numBytes) {
#if CEL_CRT_NONE
        memCopyRef(_dst, _src, _numBytes);
#else
        ::memcpy(_dst, _src, _numBytes);
#endif // CEL_CRT_NONE
    }

    inline void mem_copy(void *_dst,
                         const void *_src,
                         uint32_t _size,
                         uint32_t _num,
                         uint32_t _srcPitch,
                         uint32_t _dstPitch) {
        const uint8_t *src = (const uint8_t *) _src;
        uint8_t *dst = (uint8_t *) _dst;

        for (uint32_t ii = 0; ii < _num; ++ii) {
            mem_copy(dst, src, _size);
            src += _srcPitch;
            dst += _dstPitch;
        }
    }

    ///
    inline void gather(void *_dst,
                       const void *_src,
                       uint32_t _size,
                       uint32_t _num,
                       uint32_t _srcPitch) {
        mem_copy(_dst, _src, _size, _num, _srcPitch, _size);
    }

    ///
    inline void scatter(void *_dst,
                        const void *_src,
                        uint32_t _size,
                        uint32_t _num,
                        uint32_t _dstPitch) {
        mem_copy(_dst, _src, _size, _num, _size, _dstPitch);
    }

    inline void mem_move_ref(void *_dst,
                             const void *_src,
                             size_t _numBytes) {
        uint8_t *dst = (uint8_t *) _dst;
        const uint8_t *src = (const uint8_t *) _src;

        if (_numBytes == 0
            || dst == src) {
            return;
        }

        //	if (src+_numBytes <= dst || end <= src)
        if (dst < src) {
            mem_copy(_dst, _src, _numBytes);
            return;
        }

        for (intptr_t ii = _numBytes - 1; ii >= 0; --ii) {
            dst[ii] = src[ii];
        }
    }

    inline void mem_move(void *_dst,
                         const void *_src,
                         size_t _numBytes) {
#if CEL_CRT_NONE
        memMoveRef(_dst, _src, _numBytes);
#else
        ::memmove(_dst, _src, _numBytes);
#endif // CEL_CRT_NONE
    }

    inline void mem_set_ref(void *_dst,
                            uint8_t _ch,
                            size_t _numBytes) {
        uint8_t *dst = (uint8_t *) _dst;
        const uint8_t *end = dst + _numBytes;
        while (dst != end) {
            *dst++ = char(_ch);
        }
    }

    inline void mem_set(void *_dst,
                        uint8_t _ch,
                        size_t _numBytes) {
#if CEL_CRT_NONE
        memSetRef(_dst, _ch, _numBytes);
#else
        ::memset(_dst, _ch, _numBytes);
#endif // CEL_CRT_NONE
    }

    inline int32_t mem_cmp_ref(const void *_lhs,
                               const void *_rhs,
                               size_t _numBytes) {
        const char *lhs = (const char *) _lhs;
        const char *rhs = (const char *) _rhs;
        for (; 0 < _numBytes && *lhs == *rhs; ++lhs, ++rhs, --_numBytes
                ) {
        }

        return 0 == _numBytes ? 0 : *lhs - *rhs;
    }

    int32_t mem_cmp(const void *_lhs,
                    const void *_rhs,
                    size_t _numBytes) {
#if CEL_CRT_NONE
        return memCmpRef(_lhs, _rhs, _numBytes);
#else
        return ::memcmp(_lhs, _rhs, _numBytes);
#endif // CEL_CRT_NONE
    }
}

#endif // CEL_MEMORY_H
