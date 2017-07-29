/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef CELIB_MACROS_H
#define CELIB_MACROS_H

#include <type_traits>

#include "platform.h"

///
#if CEL_COMPILER_MSVC
// Workaround MSVS bug...
#	define CEL_VA_ARGS_PASS(...) CEL_VA_ARGS_PASS_1_ __VA_ARGS__ CEL_VA_ARGS_PASS_2_
#	define CEL_VA_ARGS_PASS_1_ (
#	define CEL_VA_ARGS_PASS_2_ )
#else
#	define CEL_VA_ARGS_PASS(...) (__VA_ARGS__)
#endif // CEL_COMPILER_MSVC

#define CEL_VA_ARGS_COUNT(...) CEL_VA_ARGS_COUNT_ CEL_VA_ARGS_PASS(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define CEL_VA_ARGS_COUNT_(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12, _a13, _a14, _a15, _a16, _last, ...) _last

///
#define CEL_MACRO_DISPATCHER(_func, ...) CEL_MACRO_DISPATCHER_1_(_func, CEL_VA_ARGS_COUNT(__VA_ARGS__) )
#define CEL_MACRO_DISPATCHER_1_(_func, _argCount) CEL_MACRO_DISPATCHER_2_(_func, _argCount)
#define CEL_MACRO_DISPATCHER_2_(_func, _argCount) CEL_CONCATENATE(_func, _argCount)

///
#define CEL_MAKEFOURCC(_a, _b, _c, _d) ( ( (uint32_t)(_a) | ( (uint32_t)(_b) << 8) | ( (uint32_t)(_c) << 16) | ( (uint32_t)(_d) << 24) ) )

///
#define CEL_STRINGIZE(_x) CEL_STRINGIZE_(_x)
#define CEL_STRINGIZE_(_x) #_x

///
#define CEL_CONCATENATE(_x, _y) CEL_CONCATENATE_(_x, _y)
#define CEL_CONCATENATE_(_x, _y) _x ## _y

///
#define CEL_FILE_LINE_LITERAL "" __FILE__ "(" CEL_STRINGIZE(__LINE__) "): "

///
#define CEL_ALIGN_MASK(_value, _mask) ( ( (_value)+(_mask) ) & ( (~0)&(~(_mask) ) ) )
#define CEL_ALIGN_16(_value) CEL_ALIGN_MASK(_value, 0xf)
#define CEL_ALIGN_256(_value) CEL_ALIGN_MASK(_value, 0xff)
#define CEL_ALIGN_4096(_value) CEL_ALIGN_MASK(_value, 0xfff)

#define CEL_ALIGNOF(_type) __alignof(_type)

#if defined(__has_feature)
#	define CEL_CLANG_HAS_FEATURE(_x) __has_feature(_x)
#else
#	define CEL_CLANG_HAS_FEATURE(_x) 0
#endif // defined(__has_feature)

#if defined(__has_extension)
#	define CEL_CLANG_HAS_EXTENSION(_x) __has_extension(_x)
#else
#	define CEL_CLANG_HAS_EXTENSION(_x) 0
#endif // defined(__has_extension)

#if CEL_COMPILER_GCC || CEL_COMPILER_CLANG
#	define CEL_ALIGN_DECL(_align, _decl) _decl __attribute__( (aligned(_align) ) )
#	define CEL_ALLOW_UNUSED __attribute__( (unused) )
#	define CEL_FORCE_INLINE inline __attribute__( (__always_inline__) )
#	define CEL_FUNCTION __PRETTY_FUNCTION__
#	define CEL_LIKELY(_x)   __builtin_expect(!!(_x), 1)
#	define CEL_UNLIKELY(_x) __builtin_expect(!!(_x), 0)
#	define CEL_NO_INLINE   __attribute__( (noinline) )
#	define CEL_NO_RETURN   __attribute__( (noreturn) )
#	if CEL_COMPILER_GCC >= 70000
#		define CEL_FALLTHROUGH __attribute__( (fallthrough) )
#	else
#		define CEL_FALLTHROUGH CEL_NOOP()
#	endif // CEL_COMPILER_GCC >= 70000
#	define CEL_NO_VTABLE
#	define CEL_OVERRIDE
#	define CEL_PRINTF_ARGS(_format, _args) __attribute__( (format(__printf__, _format, _args) ) )
#	if CEL_CLANG_HAS_FEATURE(cxx_thread_local)
#		define CEL_THREAD_LOCAL __thread
#	endif // CEL_COMPILER_CLANG
#	if (!CEL_PLATFORM_OSX && (CEL_COMPILER_GCC >= 40200)) || (CEL_COMPILER_GCC >= 40500)
#		define CEL_THREAD_LOCAL __thread
#	endif // CEL_COMPILER_GCC
#	define CEL_ATTRIBUTE(_x) __attribute__( (_x) )
#	if CEL_CRT_MSVC
#		define __stdcall
#	endif // CEL_CRT_MSVC
#elif CEL_COMPILER_MSVC
#	define CEL_ALIGN_DECL(_align, _decl) __declspec(align(_align) ) _decl
#	define CEL_ALLOW_UNUSED
#	define CEL_FORCE_INLINE __forceinline
#	define CEL_FUNCTION __FUNCTION__
#	define CEL_LIKELY(_x)   (_x)
#	define CEL_UNLIKELY(_x) (_x)
#	define CEL_NO_INLINE __declspec(noinline)
#	define CEL_NO_RETURN
#	define CEL_FALLTHROUGH CEL_NOOP()
#	define CEL_NO_VTABLE __declspec(novtable)
#	define CEL_OVERRIDE override
#	define CEL_PRINTF_ARGS(_format, _args)
#	define CEL_THREAD_LOCAL __declspec(thread)
#	define CEL_ATTRIBUTE(_x)
#else
#	error "Unknown CEL_COMPILER_?"
#endif

#define CEL_STATIC_ASSERT(_condition, ...) static_assert(_condition, "" __VA_ARGS__)

///
#define CEL_ALIGN_DECL_16(_decl) CEL_ALIGN_DECL(16, _decl)
#define CEL_ALIGN_DECL_256(_decl) CEL_ALIGN_DECL(256, _decl)
#define CEL_ALIGN_DECL_CACHE_LINE(_decl) CEL_ALIGN_DECL(CEL_CACHE_LINE_SIZE, _decl)

///
#define CEL_MACRO_BLOCK_BEGIN for(;;) {
#define CEL_MACRO_BLOCK_END break; }
#define CEL_NOOP(...) CEL_MACRO_BLOCK_BEGIN CEL_MACRO_BLOCK_END

///
#define CEL_UNUSED_1(_a1) CEL_MACRO_BLOCK_BEGIN (void)(true ? (void)0 : ( (void)(_a1) ) ); CEL_MACRO_BLOCK_END
#define CEL_UNUSED_2(_a1, _a2) CEL_UNUSED_1(_a1); CEL_UNUSED_1(_a2)
#define CEL_UNUSED_3(_a1, _a2, _a3) CEL_UNUSED_2(_a1, _a2); CEL_UNUSED_1(_a3)
#define CEL_UNUSED_4(_a1, _a2, _a3, _a4) CEL_UNUSED_3(_a1, _a2, _a3); CEL_UNUSED_1(_a4)
#define CEL_UNUSED_5(_a1, _a2, _a3, _a4, _a5) CEL_UNUSED_4(_a1, _a2, _a3, _a4); CEL_UNUSED_1(_a5)
#define CEL_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6) CEL_UNUSED_5(_a1, _a2, _a3, _a4, _a5); CEL_UNUSED_1(_a6)
#define CEL_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7) CEL_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6); CEL_UNUSED_1(_a7)
#define CEL_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8) CEL_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7); CEL_UNUSED_1(_a8)
#define CEL_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9) CEL_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8); CEL_UNUSED_1(_a9)
#define CEL_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10) CEL_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9); CEL_UNUSED_1(_a10)
#define CEL_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11) CEL_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10); CEL_UNUSED_1(_a11)
#define CEL_UNUSED_12(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12) CEL_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11); CEL_UNUSED_1(_a12)

#if CEL_COMPILER_MSVC
// Workaround MSVS bug...
#	define CEL_UNUSED(...) CEL_MACRO_DISPATCHER(CEL_UNUSED_, __VA_ARGS__) CEL_VA_ARGS_PASS(__VA_ARGS__)
#else
#	define CEL_UNUSED(...) CEL_MACRO_DISPATCHER(CEL_UNUSED_, __VA_ARGS__)(__VA_ARGS__)
#endif // CEL_COMPILER_MSVC

///
#if CEL_COMPILER_CLANG
#	define CEL_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()     _Pragma("clang diagnostic push")
#	define CEL_PRAGMA_DIAGNOSTIC_POP_CLANG_()      _Pragma("clang diagnostic pop")
#	define CEL_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x) _Pragma(CEL_STRINGIZE(clang diagnostic ignored _x) )
#else
#	define CEL_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()
#	define CEL_PRAGMA_DIAGNOSTIC_POP_CLANG_()
#	define CEL_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x)
#endif // CEL_COMPILER_CLANG

#if CEL_COMPILER_GCC && CEL_COMPILER_GCC >= 40600
#	define CEL_PRAGMA_DIAGNOSTIC_PUSH_GCC_()       _Pragma("GCC diagnostic push")
#	define CEL_PRAGMA_DIAGNOSTIC_POP_GCC_()        _Pragma("GCC diagnostic pop")
#	define CEL_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)   _Pragma(CEL_STRINGIZE(GCC diagnostic ignored _x) )
#else
#	define CEL_PRAGMA_DIAGNOSTIC_PUSH_GCC_()
#	define CEL_PRAGMA_DIAGNOSTIC_POP_GCC_()
#	define CEL_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)
#endif // CEL_COMPILER_GCC

#if CEL_COMPILER_MSVC
#	define CEL_PRAGMA_DIAGNOSTIC_PUSH_MSVC_()     __pragma(warning(push) )
#	define CEL_PRAGMA_DIAGNOSTIC_POP_MSVC_()      __pragma(warning(pop) )
#	define CEL_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x) __pragma(warning(disable:_x) )
#else
#	define CEL_PRAGMA_DIAGNOSTIC_PUSH_MSVC_()
#	define CEL_PRAGMA_DIAGNOSTIC_POP_MSVC_()
#	define CEL_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x)
#endif // CEL_COMPILER_CLANG

#if CEL_COMPILER_CLANG
#	define CEL_PRAGMA_DIAGNOSTIC_PUSH              CEL_PRAGMA_DIAGNOSTIC_PUSH_CLANG_
#	define CEL_PRAGMA_DIAGNOSTIC_POP               CEL_PRAGMA_DIAGNOSTIC_POP_CLANG_
#	define CEL_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC CEL_PRAGMA_DIAGNOSTIC_IGNORED_CLANG
#elif CEL_COMPILER_GCC
#	define CEL_PRAGMA_DIAGNOSTIC_PUSH              CEL_PRAGMA_DIAGNOSTIC_PUSH_GCC_
#	define CEL_PRAGMA_DIAGNOSTIC_POP               CEL_PRAGMA_DIAGNOSTIC_POP_GCC_
#	define CEL_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC CEL_PRAGMA_DIAGNOSTIC_IGNORED_GCC
#elif CEL_COMPILER_MSVC
#	define CEL_PRAGMA_DIAGNOSTIC_PUSH              CEL_PRAGMA_DIAGNOSTIC_PUSH_MSVC_
#	define CEL_PRAGMA_DIAGNOSTIC_POP               CEL_PRAGMA_DIAGNOSTIC_POP_MSVC_
#	define CEL_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC(_x)
#endif // CEL_COMPILER_

///
#if CEL_COMPILER_MSVC
#	define CEL_TYPE_IS_POD(t) (!__is_class(t) || __is_pod(t))
#else
#	define CEL_TYPE_IS_POD(t) std::is_pod<t>::value
#endif
///
#define CEL_CLASS_NO_DEFAULT_CTOR(_class) \
            private: _class()

#define CEL_CLASS_NO_COPY(_class) \
            private: _class(const _class& _rhs)

#define CEL_CLASS_NO_ASSIGNMENT(_class) \
            private: _class& operator=(const _class& _rhs)

#define CEL_CLASS_ALLOCATOR(_class) \
            public: void* operator new(size_t _size); \
            public: void  operator delete(void* _ptr); \
            public: void* operator new[](size_t _size); \
            public: void  operator delete[](void* _ptr)

#define CEL_CLASS_1(_class, _a1) CEL_CONCATENATE(CEL_CLASS_, _a1)(_class)
#define CEL_CLASS_2(_class, _a1, _a2) CEL_CLASS_1(_class, _a1); CEL_CLASS_1(_class, _a2)
#define CEL_CLASS_3(_class, _a1, _a2, _a3) CEL_CLASS_2(_class, _a1, _a2); CEL_CLASS_1(_class, _a3)
#define CEL_CLASS_4(_class, _a1, _a2, _a3, _a4) CEL_CLASS_3(_class, _a1, _a2, _a3); CEL_CLASS_1(_class, _a4)

#if CEL_COMPILER_MSVC
#	define CEL_CLASS(_class, ...) CEL_MACRO_DISPATCHER(CEL_CLASS_, __VA_ARGS__) CEL_VA_ARGS_PASS(_class, __VA_ARGS__)
#else
#	define CEL_CLASS(_class, ...) CEL_MACRO_DISPATCHER(CEL_CLASS_, __VA_ARGS__)(_class, __VA_ARGS__)
#endif // CEL_COMPILER_MSVC

#ifndef CEL_CHECK
#	define CEL_CHECK(_condition, ...) CEL_NOOP()
#endif // CEL_CHECK

#ifndef CEL_TRACE
#	define CEL_TRACE(...) CEL_NOOP()
#endif // CEL_TRACE

#ifndef CEL_WARN
#	define CEL_WARN(_condition, ...) CEL_NOOP()
#endif // CEL_CHECK

// static_assert sometimes causes unused-local-typedef...
CEL_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunused-local-typedef")

#endif // CELIB_MACROS_H
