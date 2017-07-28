/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef CEL_PLATFORM_H
#define CEL_PLATFORM_H

// Architecture
#define CEL_ARCH_32BIT 0
#define CEL_ARCH_64BIT 0

// Compiler
#define CEL_COMPILER_CLANG           0
#define CEL_COMPILER_CLANG_ANALYZER  0
#define CEL_COMPILER_GCC             0
#define CEL_COMPILER_MSVC            0

// Endianess
#define CEL_CPU_ENDIAN_BIG    0
#define CEL_CPU_ENDIAN_LITTLE 0

// CPU
#define CEL_CPU_ARM   0
#define CEL_CPU_JIT   0
#define CEL_CPU_MIPS  0
#define CEL_CPU_PPC   0
#define CEL_CPU_RISCV 0
#define CEL_CPU_X86   0

// C Runtime
#define CEL_CRT_BIONIC 0
#define CEL_CRT_GLIBC  0
#define CEL_CRT_LIBCXX 0
#define CEL_CRT_MINGW  0
#define CEL_CRT_MSVC   0
#define CEL_CRT_NEWLIB 0

#ifndef CEL_CRT_MUSL
#	define CEL_CRT_MUSL 0
#endif // CEL_CRT_MUSL

#ifndef CEL_CRT_NONE
#	define CEL_CRT_NONE 0
#endif // CEL_CRT_NONE

// Platform
#define CEL_PLATFORM_ANDROID    0
#define CEL_PLATFORM_EMSCRIPTEN 0
#define CEL_PLATFORM_BSD        0
#define CEL_PLATFORM_HURD       0
#define CEL_PLATFORM_IOS        0
#define CEL_PLATFORM_LINUX      0
#define CEL_PLATFORM_OSX        0
#define CEL_PLATFORM_PS4        0
#define CEL_PLATFORM_QNX        0
#define CEL_PLATFORM_RPI        0
#define CEL_PLATFORM_STEAMLINK  0
#define CEL_PLATFORM_WINDOWS    0
#define CEL_PLATFORM_WINRT      0
#define CEL_PLATFORM_XBOXONE    0

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Compilers
#if defined(__clang__)
// clang defines __GNUC__ or _MSC_VER
#	undef  CEL_COMPILER_CLANG
#	define CEL_COMPILER_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#	if defined(__clang_analyzer__)
#		undef  CEL_COMPILER_CLANG_ANALYZER
#		define CEL_COMPILER_CLANG_ANALYZER 1
#	endif // defined(__clang_analyzer__)
#elif defined(_MSC_VER)
#	undef  CEL_COMPILER_MSVC
#	define CEL_COMPILER_MSVC _MSC_VER
#elif defined(__GNUC__)
#	undef  CEL_COMPILER_GCC
#	define CEL_COMPILER_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#	error "CEL_COMPILER_* is not defined!"
#endif //

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Architectures
#if defined(__arm__) || \
    defined(__aarch64__) || \
    defined(_M_ARM)
#	undef  CEL_CPU_ARM
#	define CEL_CPU_ARM 1
#	define CEL_CACHE_LINE_SIZE 64
#elif defined(__MIPSEL__) || \
      defined(__mips_isa_rev) || \
      defined(__mips64)
#	undef  CEL_CPU_MIPS
#	define CEL_CPU_MIPS 1
#	define CEL_CACHE_LINE_SIZE 64
#elif defined(_M_PPC) || \
      defined(__powerpc__) || \
      defined(__powerpc64__)
#	undef  CEL_CPU_PPC
#	define CEL_CPU_PPC 1
#	define CEL_CACHE_LINE_SIZE 128
#elif defined(__riscv) || \
      defined(__riscv__) || \
      defined(RISCVEL)
#	undef  CEL_CPU_RISCV
#	define CEL_CPU_RISCV 1
#	define CEL_CACHE_LINE_SIZE 64
#elif defined(_M_IX86) || \
      defined(_M_X64) || \
      defined(__i386__) || \
      defined(__x86_64__)
#	undef  CEL_CPU_X86
#	define CEL_CPU_X86 1
#	define CEL_CACHE_LINE_SIZE 64
#else // PNaCl doesn't have CPU defined.
#	undef  CEL_CPU_JIT
#	define CEL_CPU_JIT 1
#	define CEL_CACHE_LINE_SIZE 64
#endif //

#if defined(__x86_64__) || \
    defined(_M_X64) || \
    defined(__aarch64__) || \
    defined(__64BIT__) || \
    defined(__mips64) || \
    defined(__powerpc64__) || \
    defined(__ppc64__) || \
    defined(__LP64__)
#	undef  CEL_ARCH_64BIT
#	define CEL_ARCH_64BIT 64
#else
#	undef  CEL_ARCH_32BIT
#	define CEL_ARCH_32BIT 32
#endif //

#if CEL_CPU_PPC
#	undef  CEL_CPU_ENDIAN_BIG
#	define CEL_CPU_ENDIAN_BIG 1
#else
#	undef  CEL_CPU_ENDIAN_LITTLE
#	define CEL_CPU_ENDIAN_LITTLE 1
#endif // CEL_PLATFORM_

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Operating_Systems
#if defined(_DURANGO) || defined(_XBOX_ONE)
#	undef  CEL_PLATFORM_XBOXONE
#	define CEL_PLATFORM_XBOXONE 1
#elif defined(_WIN32) || defined(_WIN64)
// http://msdn.microsoft.com/en-us/library/6sehtctf.aspx
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif // NOMINMAX
//  If _USING_V110_SDK71_ is defined it means we are using the v110_xp or v120_xp toolset.
#	if defined(_MSC_VER) && (_MSC_VER >= 1700) && (!_USING_V110_SDK71_)
#		include <winapifamily.h>
#	endif // defined(_MSC_VER) && (_MSC_VER >= 1700) && (!_USING_V110_SDK71_)
#	if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#		undef  CEL_PLATFORM_WINDOWS
#		if !defined(WINVER) && !defined(_WIN32_WINNT)
#			if CEL_ARCH_64BIT
//				When building 64-bit target Win7 and above.
#				define WINVER 0x0601
#				define _WIN32_WINNT 0x0601
#			else
//				Windows Server 2003 with SP1, Windows XP with SP2 and above
#				define WINVER 0x0502
#				define _WIN32_WINNT 0x0502
#			endif // CEL_ARCH_64BIT
#		endif // !defined(WINVER) && !defined(_WIN32_WINNT)
#		define CEL_PLATFORM_WINDOWS _WIN32_WINNT
#	else
#		undef  CEL_PLATFORM_WINRT
#		define CEL_PLATFORM_WINRT 1
#	endif
#elif defined(__ANDROID__)
// Android compiler defines __linux__
#	include <sys/cdefs.h> // Defines __BIONIC__ and includes android/api-level.h
#	undef  CEL_PLATFORM_ANDROID
#	define CEL_PLATFORM_ANDROID __ANDROID_API__
#elif defined(__STEAMLINK__)
// SteamLink compiler defines __linux__
#	undef  CEL_PLATFORM_STEAMLINK
#	define CEL_PLATFORM_STEAMLINK 1
#elif defined(__VCCOREVER__)
// RaspberryPi compiler defines __linux__
#	undef  CEL_PLATFORM_RPI
#	define CEL_PLATFORM_RPI 1
#elif  defined(__linux__)
#	undef  CEL_PLATFORM_LINUX
#	define CEL_PLATFORM_LINUX 1
#elif  defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) \
 || defined(__ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__)
#	undef  CEL_PLATFORM_IOS
#	define CEL_PLATFORM_IOS 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#	undef  CEL_PLATFORM_OSX
#	define CEL_PLATFORM_OSX __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#elif defined(__EMSCRIPTEN__)
#	undef  CEL_PLATFORM_EMSCRIPTEN
#	define CEL_PLATFORM_EMSCRIPTEN 1
#elif defined(__ORBIS__)
#	undef  CEL_PLATFORM_PS4
#	define CEL_PLATFORM_PS4 1
#elif defined(__QNX__)
#	undef  CEL_PLATFORM_QNX
#	define CEL_PLATFORM_QNX 1
#elif  defined(__FreeBSD__)        \
    || defined(__FreeBSD_kernel__) \
    || defined(__NetBSD__)         \
    || defined(__OpenBSD__)        \
    || defined(__DragonFly__)
#	undef  CEL_PLATFORM_BSD
#	define CEL_PLATFORM_BSD 1
#elif defined(__GNU__)
#	undef  CEL_PLATFORM_HURD
#	define CEL_PLATFORM_HURD 1
#endif //

#if !CEL_CRT_NONE
// https://sourceforge.net/p/predef/wiki/Libraries/
#	if defined(__BIONIC__)
#		undef  CEL_CRT_BIONIC
#		define CEL_CRT_BIONIC 1
#	elif defined(_MSC_VER)
#		undef  CEL_CRT_MSVC
#		define CEL_CRT_MSVC 1
#	elif defined(__GLIBC__)
#		undef  CEL_CRT_GLIBC
#		define CEL_CRT_GLIBC (__GLIBC__ * 10000 + __GLIBC_MINOR__ * 100)
#	elif defined(__MINGW32__) || defined(__MINGW64__)
#		undef  CEL_CRT_MINGW
#		define CEL_CRT_MINGW 1
#	elif defined(__apple_build_version__) || defined(__ORBIS__) || defined(__EMSCRIPTEN__) || defined(__llvm__)
#		undef  CEL_CRT_LIBCXX
#		define CEL_CRT_LIBCXX 1
#	endif //

#	if !CEL_CRT_BIONIC \
 && !CEL_CRT_GLIBC  \
 && !CEL_CRT_LIBCXX \
 && !CEL_CRT_MINGW  \
 && !CEL_CRT_MSVC   \
 && !CEL_CRT_MUSL   \
 && !CEL_CRT_NEWLIB
#		undef  CEL_CRT_NONE
#		define CEL_CRT_NONE 1
#	endif // CEL_CRT_*
#endif // !CEL_CRT_NONE

#define CEL_PLATFORM_POSIX (0      \
        || CEL_PLATFORM_ANDROID    \
        || CEL_PLATFORM_EMSCRIPTEN \
        || CEL_PLATFORM_BSD        \
        || CEL_PLATFORM_HURD       \
        || CEL_PLATFORM_IOS        \
        || CEL_PLATFORM_LINUX      \
        || CEL_PLATFORM_OSX        \
        || CEL_PLATFORM_QNX        \
        || CEL_PLATFORM_STEAMLINK  \
        || CEL_PLATFORM_PS4        \
        || CEL_PLATFORM_RPI        \
        )

#define CEL_PLATFORM_NONE !(0      \
        || CEL_PLATFORM_ANDROID    \
        || CEL_PLATFORM_EMSCRIPTEN \
        || CEL_PLATFORM_BSD        \
        || CEL_PLATFORM_HURD       \
        || CEL_PLATFORM_IOS        \
        || CEL_PLATFORM_LINUX      \
        || CEL_PLATFORM_OSX        \
        || CEL_PLATFORM_PS4        \
        || CEL_PLATFORM_QNX        \
        || CEL_PLATFORM_RPI        \
        || CEL_PLATFORM_STEAMLINK  \
        || CEL_PLATFORM_WINDOWS    \
        || CEL_PLATFORM_WINRT      \
        || CEL_PLATFORM_XBOXONE    \
        )

#if CEL_COMPILER_GCC
#	define CEL_COMPILER_NAME "GCC " \
                CEL_STRINGIZE(__GNUC__) "." \
                CEL_STRINGIZE(__GNUC_MINOR__) "." \
                CEL_STRINGIZE(__GNUC_PATCHLEVEL__)
#elif CEL_COMPILER_CLANG
#	define CEL_COMPILER_NAME "Clang " \
                CEL_STRINGIZE(__clang_major__) "." \
                CEL_STRINGIZE(__clang_minor__) "." \
                CEL_STRINGIZE(__clang_patchlevel__)
#elif CEL_COMPILER_MSVC
#	if CEL_COMPILER_MSVC >= 1910 // Visual Studio 2017
#		define CEL_COMPILER_NAME "MSVC 15.0"
#	elif CEL_COMPILER_MSVC >= 1900 // Visual Studio 2015
#		define CEL_COMPILER_NAME "MSVC 14.0"
#	elif CEL_COMPILER_MSVC >= 1800 // Visual Studio 2013
#		define CEL_COMPILER_NAME "MSVC 12.0"
#	elif CEL_COMPILER_MSVC >= 1700 // Visual Studio 2012
#		define CEL_COMPILER_NAME "MSVC 11.0"
#	elif CEL_COMPILER_MSVC >= 1600 // Visual Studio 2010
#		define CEL_COMPILER_NAME "MSVC 10.0"
#	elif CEL_COMPILER_MSVC >= 1500 // Visual Studio 2008
#		define CEL_COMPILER_NAME "MSVC 9.0"
#	else
#		define CEL_COMPILER_NAME "MSVC"
#	endif //
#endif // CEL_COMPILER_

#if CEL_PLATFORM_ANDROID
#	define CEL_PLATFORM_NAME "Android " \
                CEL_STRINGIZE(CEL_PLATFORM_ANDROID)
#elif CEL_PLATFORM_EMSCRIPTEN
#	define CEL_PLATFORM_NAME "asm.js " \
                CEL_STRINGIZE(__EMSCRIPTEN_major__) "." \
                CEL_STRINGIZE(__EMSCRIPTEN_minor__) "." \
                CEL_STRINGIZE(__EMSCRIPTEN_tiny__)
#elif CEL_PLATFORM_BSD
#	define CEL_PLATFORM_NAME "BSD"
#elif CEL_PLATFORM_HURD
#	define CEL_PLATFORM_NAME "Hurd"
#elif CEL_PLATFORM_IOS
#	define CEL_PLATFORM_NAME "iOS"
#elif CEL_PLATFORM_LINUX
#	define CEL_PLATFORM_NAME "Linux"
#elif CEL_PLATFORM_OSX
#	define CEL_PLATFORM_NAME "OSX"
#elif CEL_PLATFORM_PS4
#	define CEL_PLATFORM_NAME "PlayStation 4"
#elif CEL_PLATFORM_QNX
#	define CEL_PLATFORM_NAME "QNX"
#elif CEL_PLATFORM_RPI
#	define CEL_PLATFORM_NAME "RaspberryPi"
#elif CEL_PLATFORM_STEAMLINK
#	define CEL_PLATFORM_NAME "SteamLink"
#elif CEL_PLATFORM_WINDOWS
#	define CEL_PLATFORM_NAME "Windows"
#elif CEL_PLATFORM_WINRT
#	define CEL_PLATFORM_NAME "WinRT"
#elif CEL_PLATFORM_XBOXONE
#	define CEL_PLATFORM_NAME "Xbox One"
#elif CEL_PLATFORM_NONE
#	define CEL_PLATFORM_NAME "None"
#else
#	error "Unknown CEL_PLATFORM!"
#endif // CEL_PLATFORM_

#if CEL_CPU_ARM
#	define CEL_CPU_NAME "ARM"
#elif CEL_CPU_JIT
#	define CEL_CPU_NAME "JIT-VM"
#elif CEL_CPU_MIPS
#	define CEL_CPU_NAME "MIPS"
#elif CEL_CPU_PPC
#	define CEL_CPU_NAME "PowerPC"
#elif CEL_CPU_RISCV
#	define CEL_CPU_NAME "RISC-V"
#elif CEL_CPU_X86
#	define CEL_CPU_NAME "x86"
#endif // CEL_CPU_

#if CEL_CRT_BIONIC
#	define CEL_CRT_NAME "Bionic libc"
#elif CEL_CRT_GLIBC
#	define CEL_CRT_NAME "GNU C Library"
#elif CEL_CRT_MSVC
#	define CEL_CRT_NAME "MSVC C Runtime"
#elif CEL_CRT_MINGW
#	define CEL_CRT_NAME "MinGW C Runtime"
#elif CEL_CRT_LIBCXX
#	define CEL_CRT_NAME "Clang C Library"
#elif CEL_CRT_NEWLIB
#	define CEL_CRT_NAME "Newlib"
#elif CEL_CRT_MUSL
#	define CEL_CRT_NAME "musl libc"
#elif CEL_CRT_NONE
#	define CEL_CRT_NAME "None"
#else
#	error "Unknown CEL_CRT!"
#endif // CEL_CRT_

#if CEL_ARCH_32BIT
#	define CEL_ARCH_NAME "32-bit"
#elif CEL_ARCH_64BIT
#	define CEL_ARCH_NAME "64-bit"
#endif // CEL_ARCH_

#endif // CEL_PLATFORM_H
