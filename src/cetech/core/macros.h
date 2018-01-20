/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef CECORE_MACROS_H
#define CECORE_MACROS_H

#include <stdbool.h>

#include "platform.h"

///
#if CT_COMPILER_MSVC
// Workaround MSVS bug...
#	define CT_VA_ARGS_PASS(...) CT_VA_ARGS_PASS_1_ __VA_ARGS__ CT_VA_ARGS_PASS_2_
#	define CT_VA_ARGS_PASS_1_ (
#	define CT_VA_ARGS_PASS_2_ )
#else
#	define CT_VA_ARGS_PASS(...) (__VA_ARGS__)
#endif // CT_COMPILER_MSVC

#define CT_VA_ARGS_COUNT(...) CT_VA_ARGS_COUNT_ CT_VA_ARGS_PASS(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define CT_VA_ARGS_COUNT_(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12, _a13, _a14, _a15, _a16, _last, ...) _last

///
#define CT_MACRO_DISPATCHER(_func, ...) CT_MACRO_DISPATCHER_1_(_func, CT_VA_ARGS_COUNT(__VA_ARGS__) )
#define CT_MACRO_DISPATCHER_1_(_func, _argCount) CT_MACRO_DISPATCHER_2_(_func, _argCount)
#define CT_MACRO_DISPATCHER_2_(_func, _argCount) CT_CONCATENATE(_func, _argCount)

///
#define CT_MAKEFOURCC(_a, _b, _c, _d) ( ( (uint32_t)(_a) | ( (uint32_t)(_b) << 8) | ( (uint32_t)(_c) << 16) | ( (uint32_t)(_d) << 24) ) )

///
#define CT_STRINGIZE(_x) CT_STRINGIZE_(_x)
#define CT_STRINGIZE_(_x) #_x

///
#define CT_CONCATENATE(_x, _y) CT_CONCATENATE_(_x, _y)
#define CT_CONCATENATE_(_x, _y) _x ## _y

///
#define CT_FILE_LINE_LITERAL "" __FILE__ "(" CT_STRINGIZE(__LINE__) "): "

///
#define CT_ALIGN_MASK(_value, _mask) ( ( (_value)+(_mask) ) & ( (~0)&(~(_mask) ) ) )
#define CT_ALIGN_16(_value) CT_ALIGN_MASK(_value, 0xf)
#define CT_ALIGN_256(_value) CT_ALIGN_MASK(_value, 0xff)
#define CT_ALIGN_4096(_value) CT_ALIGN_MASK(_value, 0xfff)

#define CT_ALIGNOF(_type) __alignof(_type)

#if defined(__has_feature)
#	define CT_CLANG_HAS_FEATURE(_x) __has_feature(_x)
#else
#	define CT_CLANG_HAS_FEATURE(_x) 0
#endif // defined(__has_feature)

#if defined(__has_extension)
#	define CT_CLANG_HAS_EXTENSION(_x) __has_extension(_x)
#else
#	define CT_CLANG_HAS_EXTENSION(_x) 0
#endif // defined(__has_extension)

#if CT_COMPILER_GCC || CT_COMPILER_CLANG
#	define CT_ALIGN_DECL(_align, _decl) _decl __attribute__( (aligned(_align) ) )
#	define CT_ALLOW_UNUSED __attribute__( (unused) )
#	define CT_FORCE_INLINE inline __attribute__( (__always_inline__) )
#	define CT_FUNCTION __PRETTY_FUNCTION__
#	define CT_LIKELY(_x)   __builtin_expect(!!(_x), 1)
#	define CT_UNLIKELY(_x) __builtin_expect(!!(_x), 0)
#	define CT_NO_INLINE   __attribute__( (noinline) )
#	define CT_NO_RETURN   __attribute__( (noreturn) )
#	if CT_COMPILER_GCC >= 70000
#		define CT_FALLTHROUGH __attribute__( (fallthrough) )
#	else
#		define CT_FALLTHROUGH CT_NOOP()
#	endif // CT_COMPILER_GCC >= 70000
#	define CT_NO_VTABLE
#	define CT_OVERRIDE
#	define CT_PRINTF_ARGS(_format, _args) __attribute__( (format(__printf__, _format, _args) ) )
#	if CT_CLANG_HAS_FEATURE(cxx_thread_local)
#		define CT_THREAD_LOCAL __thread
#	endif // CT_COMPILER_CLANG
#	if (!CT_PLATFORM_OSX && (CT_COMPILER_GCC >= 40200)) || (CT_COMPILER_GCC >= 40500)
#		define CT_THREAD_LOCAL __thread
#	endif // CT_COMPILER_GCC
#	define CT_ATTRIBUTE(_x) __attribute__( (_x) )
#	if CT_CRT_MSVC
#		define __stdcall
#	endif // CT_CRT_MSVC
#elif CT_COMPILER_MSVC
#	define CT_ALIGN_DECL(_align, _decl) __declspec(align(_align) ) _decl
#	define CT_ALLOW_UNUSED
#	define CT_FORCE_INLINE __forceinline
#	define CT_FUNCTION __FUNCTION__
#	define CT_LIKELY(_x)   (_x)
#	define CT_UNLIKELY(_x) (_x)
#	define CT_NO_INLINE __declspec(noinline)
#	define CT_NO_RETURN
#	define CT_FALLTHROUGH CT_NOOP()
#	define CT_NO_VTABLE __declspec(novtable)
#	define CT_OVERRIDE override
#	define CT_PRINTF_ARGS(_format, _args)
#	define CT_THREAD_LOCAL __declspec(thread)
#	define CT_ATTRIBUTE(_x)
#else
#	error "Unknown CT_COMPILER_?"
#endif

#define CT_STATIC_ASSERT(_condition, ...) static_assert(_condition, "" __VA_ARGS__)

///
#define CT_ALIGN_DECL_16(_decl) CT_ALIGN_DECL(16, _decl)
#define CT_ALIGN_DECL_256(_decl) CT_ALIGN_DECL(256, _decl)
#define CT_ALIGN_DECL_CACHE_LINE(_decl) CT_ALIGN_DECL(CT_CACHE_LINE_SIZE, _decl)

///
#define CT_MACRO_BLOCK_BEGIN for(;;) {
#define CT_MACRO_BLOCK_END break; }
#define CT_NOOP(...) CT_MACRO_BLOCK_BEGIN CT_MACRO_BLOCK_END

///
#define CT_UNUSED_1(_a1) CT_MACRO_BLOCK_BEGIN (void)(true ? (void)0 : ( (void)(_a1) ) ); CT_MACRO_BLOCK_END
#define CT_UNUSED_2(_a1, _a2) CT_UNUSED_1(_a1); CT_UNUSED_1(_a2)
#define CT_UNUSED_3(_a1, _a2, _a3) CT_UNUSED_2(_a1, _a2); CT_UNUSED_1(_a3)
#define CT_UNUSED_4(_a1, _a2, _a3, _a4) CT_UNUSED_3(_a1, _a2, _a3); CT_UNUSED_1(_a4)
#define CT_UNUSED_5(_a1, _a2, _a3, _a4, _a5) CT_UNUSED_4(_a1, _a2, _a3, _a4); CT_UNUSED_1(_a5)
#define CT_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6) CT_UNUSED_5(_a1, _a2, _a3, _a4, _a5); CT_UNUSED_1(_a6)
#define CT_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7) CT_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6); CT_UNUSED_1(_a7)
#define CT_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8) CT_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7); CT_UNUSED_1(_a8)
#define CT_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9) CT_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8); CT_UNUSED_1(_a9)
#define CT_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10) CT_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9); CT_UNUSED_1(_a10)
#define CT_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11) CT_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10); CT_UNUSED_1(_a11)
#define CT_UNUSED_12(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12) CT_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11); CT_UNUSED_1(_a12)

#if CT_COMPILER_MSVC
// Workaround MSVS bug...
#	define CT_UNUSED(...) CT_MACRO_DISPATCHER(CT_UNUSED_, __VA_ARGS__) CT_VA_ARGS_PASS(__VA_ARGS__)
#else
#	define CT_UNUSED(...) CT_MACRO_DISPATCHER(CT_UNUSED_, __VA_ARGS__)(__VA_ARGS__)
#endif // CT_COMPILER_MSVC

///
#if CT_COMPILER_CLANG
#	define CT_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()     _Pragma("clang diagnostic push")
#	define CT_PRAGMA_DIAGNOSTIC_POP_CLANG_()      _Pragma("clang diagnostic pop")
#	define CT_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x) _Pragma(CT_STRINGIZE(clang diagnostic ignored _x) )
#else
#	define CT_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()
#	define CT_PRAGMA_DIAGNOSTIC_POP_CLANG_()
#	define CT_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x)
#endif // CT_COMPILER_CLANG

#if CT_COMPILER_GCC && CT_COMPILER_GCC >= 40600
#	define CT_PRAGMA_DIAGNOSTIC_PUSH_GCC_()       _Pragma("GCC diagnostic push")
#	define CT_PRAGMA_DIAGNOSTIC_POP_GCC_()        _Pragma("GCC diagnostic pop")
#	define CT_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)   _Pragma(CT_STRINGIZE(GCC diagnostic ignored _x) )
#else
#	define CT_PRAGMA_DIAGNOSTIC_PUSH_GCC_()
#	define CT_PRAGMA_DIAGNOSTIC_POP_GCC_()
#	define CT_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)
#endif // CT_COMPILER_GCC

#if CT_COMPILER_MSVC
#	define CT_PRAGMA_DIAGNOSTIC_PUSH_MSVC_()     __pragma(warning(push) )
#	define CT_PRAGMA_DIAGNOSTIC_POP_MSVC_()      __pragma(warning(pop) )
#	define CT_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x) __pragma(warning(disable:_x) )
#else
#	define CT_PRAGMA_DIAGNOSTIC_PUSH_MSVC_()
#	define CT_PRAGMA_DIAGNOSTIC_POP_MSVC_()
#	define CT_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x)
#endif // CT_COMPILER_CLANG

#if CT_COMPILER_CLANG
#	define CT_PRAGMA_DIAGNOSTIC_PUSH              CT_PRAGMA_DIAGNOSTIC_PUSH_CLANG_
#	define CT_PRAGMA_DIAGNOSTIC_POP               CT_PRAGMA_DIAGNOSTIC_POP_CLANG_
#	define CT_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC CT_PRAGMA_DIAGNOSTIC_IGNORED_CLANG
#elif CT_COMPILER_GCC
#	define CT_PRAGMA_DIAGNOSTIC_PUSH              CT_PRAGMA_DIAGNOSTIC_PUSH_GCC_
#	define CT_PRAGMA_DIAGNOSTIC_POP               CT_PRAGMA_DIAGNOSTIC_POP_GCC_
#	define CT_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC CT_PRAGMA_DIAGNOSTIC_IGNORED_GCC
#elif CT_COMPILER_MSVC
#	define CT_PRAGMA_DIAGNOSTIC_PUSH              CT_PRAGMA_DIAGNOSTIC_PUSH_MSVC_
#	define CT_PRAGMA_DIAGNOSTIC_POP               CT_PRAGMA_DIAGNOSTIC_POP_MSVC_
#	define CT_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC(_x)
#endif // CT_COMPILER_

// static_assert sometimes causes unused-local-typedef...
CT_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunused-local-typedef")

#endif // CECORE_MACROS_H
