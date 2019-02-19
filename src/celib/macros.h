/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef CECORE_MACROS_H
#define CECORE_MACROS_H

#include "celib_types.h"
#include "celib/platform.h"

#define CE_ARRAY_LEN(_name) (sizeof(_name) / sizeof(_name[0]))

#define CE_ARR_ARG(a) a, CE_ARRAY_LEN(a)
#define CE_ATTR_FORMAT(fmt, args) __attribute__ ((format(printf, fmt, args)))
#define CE_STATIC_ASSERT(_condition, ...) _Static_assert(_condition, "" __VA_ARGS__)
#define CE_ALIGNOF(_type) __alignof(_type)

///
#if CE_COMPILER_MSVC
// Workaround MSVS bug...
#	define CE_VA_ARGS_PASS(...) CE_VA_ARGS_PASS_1_ __VA_ARGS__ CE_VA_ARGS_PASS_2_
#	define CE_VA_ARGS_PASS_1_ (
#	define CE_VA_ARGS_PASS_2_ )
#else
#	define CE_VA_ARGS_PASS(...) (__VA_ARGS__)
#endif // CE_COMPILER_MSVC

#define CE_VA_ARGS_COUNT(...) CE_VA_ARGS_COUNT_ CE_VA_ARGS_PASS(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define CE_VA_ARGS_COUNT_(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12, _a13, _a14, _a15, _a16, _last, ...) _last

///
#define CE_MACRO_DISPATCHER(_func, ...) CE_MACRO_DISPATCHER_1_(_func, CE_VA_ARGS_COUNT(__VA_ARGS__) )
#define CE_MACRO_DISPATCHER_1_(_func, _argCount) CE_MACRO_DISPATCHER_2_(_func, _argCount)
#define CE_MACRO_DISPATCHER_2_(_func, _argCount) CE_CONCATENATE(_func, _argCount)

///
#define CE_MAKEFOURCC(_a, _b, _c, _d) ( ( (uint32_t)(_a) | ( (uint32_t)(_b) << 8) | ( (uint32_t)(_c) << 16) | ( (uint32_t)(_d) << 24) ) )

///
#define CE_STRINGIZE(_x) CE_STRINGIZE_(_x)
#define CE_STRINGIZE_(_x) #_x

///
#define CE_CONCATENATE(_x, _y) CE_CONCATENATE_(_x, _y)
#define CE_CONCATENATE_(_x, _y) _x ## _y

///
#define CE_FILE_LINE_LITERAL "" __FILE__ "(" CE_STRINGIZE(__LINE__) "): "

///
#define CE_ALIGN_MASK(_value, _mask) ( ( (_value)+(_mask) ) & ( (~0)&(~(_mask) ) ) )
#define CE_ALIGN_16(_value) CE_ALIGN_MASK(_value, 0xf)
#define CE_ALIGN_256(_value) CE_ALIGN_MASK(_value, 0xff)
#define CE_ALIGN_4096(_value) CE_ALIGN_MASK(_value, 0xfff)



#if defined(__has_feature)
#	define CE_CLANG_HAS_FEATURE(_x) __has_feature(_x)
#else
#	define CE_CLANG_HAS_FEATURE(_x) 0
#endif // defined(__has_feature)

#if defined(__has_extension)
#	define CE_CLANG_HAS_EXTENSION(_x) __has_extension(_x)
#else
#	define CE_CLANG_HAS_EXTENSION(_x) 0
#endif // defined(__has_extension)

#if CE_COMPILER_GCC || CE_COMPILER_CLANG
#	define CE_ALIGN_DECL(_align, _decl) _decl __attribute__( (aligned(_align) ) )
#	define CE_ALLOW_UNUSED __attribute__( (unused) )
#	define CE_FORCE_INLINE inline __attribute__( (__always_inline__) )
#	define CE_FUNCTION __PRETTY_FUNCTION__
#	define CE_LIKELY(_x)   __builtin_expect(!!(_x), 1)
#	define CE_UNLIKELY(_x) __builtin_expect(!!(_x), 0)
#	define CE_NO_INLINE   __attribute__( (noinline) )
#	define CE_NO_RETURN   __attribute__( (noreturn) )
#	if CE_COMPILER_GCC >= 70000
#		define CE_FALLTHROUGH __attribute__( (fallthrough) )
#	else
#		define CE_FALLTHROUGH CE_NOOP()
#	endif // CE_COMPILER_GCC >= 70000
#	define CE_NO_VTABLE
#	define CE_OVERRIDE
#	define CE_PRINTF_ARGS(_format, _args) __attribute__( (format(__printf__, _format, _args) ) )
#	if CE_CLANG_HAS_FEATURE(cxx_thread_local)
#		define CE_THREAD_LOCAL __thread
#	endif // CE_COMPILER_CLANG
#	if (!CE_PLATFORM_OSX && (CE_COMPILER_GCC >= 40200)) || (CE_COMPILER_GCC >= 40500)
#		define CE_THREAD_LOCAL __thread
#	endif // CE_COMPILER_GCC
#	define CE_ATTRIBUTE(_x) __attribute__( (_x) )
#	if CE_CRT_MSVC
#		define __stdcall
#	endif // CE_CRT_MSVC
#elif CE_COMPILER_MSVC
#	define CE_ALIGN_DECL(_align, _decl) __declspec(align(_align) ) _decl
#	define CE_ALLOW_UNUSED
#	define CE_FORCE_INLINE __forceinline
#	define CE_FUNCTION __FUNCTION__
#	define CE_LIKELY(_x)   (_x)
#	define CE_UNLIKELY(_x) (_x)
#	define CE_NO_INLINE __declspec(noinline)
#	define CE_NO_RETURN
#	define CE_FALLTHROUGH CE_NOOP()
#	define CE_NO_VTABLE __declspec(novtable)
#	define CE_OVERRIDE override
#	define CE_PRINTF_ARGS(_format, _args)
#	define CE_THREAD_LOCAL __declspec(thread)
#	define CE_ATTRIBUTE(_x)
#else
#	error "Unknown CE_COMPILER_?"
#endif


///
#define CE_ALIGN_DECL_16(_decl) CE_ALIGN_DECL(16, _decl)
#define CE_ALIGN_DECL_256(_decl) CE_ALIGN_DECL(256, _decl)
#define CE_ALIGN_DECL_CACHE_LINE(_decl) CE_ALIGN_DECL(CE_CACHE_LINE_SIZE, _decl)

///
#define CE_MACRO_BLOCK_BEGIN for(;;) {
#define CE_MACRO_BLOCK_END break; }
#define CE_NOOP(...) CE_MACRO_BLOCK_BEGIN CE_MACRO_BLOCK_END

///
#define CE_UNUSED_1(_a1) CE_MACRO_BLOCK_BEGIN (void)(true ? (void)0 : ( (void)(_a1) ) ); CE_MACRO_BLOCK_END
#define CE_UNUSED_2(_a1, _a2) CE_UNUSED_1(_a1); CE_UNUSED_1(_a2)
#define CE_UNUSED_3(_a1, _a2, _a3) CE_UNUSED_2(_a1, _a2); CE_UNUSED_1(_a3)
#define CE_UNUSED_4(_a1, _a2, _a3, _a4) CE_UNUSED_3(_a1, _a2, _a3); CE_UNUSED_1(_a4)
#define CE_UNUSED_5(_a1, _a2, _a3, _a4, _a5) CE_UNUSED_4(_a1, _a2, _a3, _a4); CE_UNUSED_1(_a5)
#define CE_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6) CE_UNUSED_5(_a1, _a2, _a3, _a4, _a5); CE_UNUSED_1(_a6)
#define CE_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7) CE_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6); CE_UNUSED_1(_a7)
#define CE_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8) CE_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7); CE_UNUSED_1(_a8)
#define CE_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9) CE_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8); CE_UNUSED_1(_a9)
#define CE_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10) CE_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9); CE_UNUSED_1(_a10)
#define CE_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11) CE_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10); CE_UNUSED_1(_a11)
#define CE_UNUSED_12(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12) CE_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11); CE_UNUSED_1(_a12)

#if CE_COMPILER_MSVC
// Workaround MSVS bug...
#	define CE_UNUSED(...) CE_MACRO_DISPATCHER(CE_UNUSED_, __VA_ARGS__) CE_VA_ARGS_PASS(__VA_ARGS__)
#else
#	define CE_UNUSED(...) CE_MACRO_DISPATCHER(CE_UNUSED_, __VA_ARGS__)(__VA_ARGS__)
#endif // CE_COMPILER_MSVC

///
#if CE_COMPILER_CLANG
#	define CE_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()     _Pragma("clang diagnostic push")
#	define CE_PRAGMA_DIAGNOSTIC_POP_CLANG_()      _Pragma("clang diagnostic pop")
#	define CE_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x) _Pragma(CE_STRINGIZE(clang diagnostic ignored _x) )
#else
#	define CE_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()
#	define CE_PRAGMA_DIAGNOSTIC_POP_CLANG_()
#	define CE_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x)
#endif // CE_COMPILER_CLANG

#if CE_COMPILER_GCC && CE_COMPILER_GCC >= 40600
#	define CE_PRAGMA_DIAGNOSTIC_PUSH_GCC_()       _Pragma("GCC diagnostic push")
#	define CE_PRAGMA_DIAGNOSTIC_POP_GCC_()        _Pragma("GCC diagnostic pop")
#	define CE_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)   _Pragma(CE_STRINGIZE(GCC diagnostic ignored _x) )
#else
#	define CE_PRAGMA_DIAGNOSTIC_PUSH_GCC_()
#	define CE_PRAGMA_DIAGNOSTIC_POP_GCC_()
#	define CE_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)
#endif // CE_COMPILER_GCC

#if CE_COMPILER_MSVC
#	define CE_PRAGMA_DIAGNOSTIC_PUSH_MSVC_()     __pragma(warning(push) )
#	define CE_PRAGMA_DIAGNOSTIC_POP_MSVC_()      __pragma(warning(pop) )
#	define CE_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x) __pragma(warning(disable:_x) )
#else
#	define CE_PRAGMA_DIAGNOSTIC_PUSH_MSVC_()
#	define CE_PRAGMA_DIAGNOSTIC_POP_MSVC_()
#	define CE_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x)
#endif // CE_COMPILER_CLANG

#if CE_COMPILER_CLANG
#	define CE_PRAGMA_DIAGNOSTIC_PUSH              CE_PRAGMA_DIAGNOSTIC_PUSH_CLANG_
#	define CE_PRAGMA_DIAGNOSTIC_POP               CE_PRAGMA_DIAGNOSTIC_POP_CLANG_
#	define CE_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC CE_PRAGMA_DIAGNOSTIC_IGNORED_CLANG
#elif CE_COMPILER_GCC
#	define CE_PRAGMA_DIAGNOSTIC_PUSH              CE_PRAGMA_DIAGNOSTIC_PUSH_GCC_
#	define CE_PRAGMA_DIAGNOSTIC_POP               CE_PRAGMA_DIAGNOSTIC_POP_GCC_
#	define CE_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC CE_PRAGMA_DIAGNOSTIC_IGNORED_GCC
#elif CE_COMPILER_MSVC
#	define CE_PRAGMA_DIAGNOSTIC_PUSH              CE_PRAGMA_DIAGNOSTIC_PUSH_MSVC_
#	define CE_PRAGMA_DIAGNOSTIC_POP               CE_PRAGMA_DIAGNOSTIC_POP_MSVC_
#	define CE_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC(_x)
#endif // CE_COMPILER_

// static_assert sometimes causes unused-local-typedef...
CE_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunused-local-typedef")

#endif // CECORE_MACROS_H
