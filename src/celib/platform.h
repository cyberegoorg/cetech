/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef CE_PLATFORM_H
#define CE_PLATFORM_H

// Architecture
#define CE_ARCH_32BIT 0
#define CE_ARCH_64BIT 0

// Compiler
#define CE_COMPILER_CLANG           0
#define CE_COMPILER_CLANG_ANALYZER  0
#define CE_COMPILER_GCC             0
#define CE_COMPILER_MSVC            0

// Endianess
#define CE_CPU_ENDIAN_BIG    0
#define CE_CPU_ENDIAN_LITTLE 0

// CPU
#define CE_CPU_ARM   0
#define CE_CPU_JIT   0
#define CE_CPU_MIPS  0
#define CE_CPU_PPC   0
#define CE_CPU_RISCV 0
#define CE_CPU_X86   0

// C Runtime
#define CE_CRT_BIONIC 0
#define CE_CRT_GLIBC  0
#define CE_CRT_LIBCXX 0
#define CE_CRT_MINGW  0
#define CE_CRT_MSVC   0
#define CE_CRT_NEWLIB 0

#ifndef CE_CRT_MUSL
#	define CE_CRT_MUSL 0
#endif // CE_CRT_MUSL

#ifndef CE_CRT_NONE
#	define CE_CRT_NONE 0
#endif // CE_CRT_NONE

// Platform
#define CE_PLATFORM_ANDROID    0
#define CE_PLATFORM_EMSCRIPTEN 0
#define CE_PLATFORM_BSD        0
#define CE_PLATFORM_HURD       0
#define CE_PLATFORM_IOS        0
#define CE_PLATFORM_LINUX      0
#define CE_PLATFORM_OSX        0
#define CE_PLATFORM_PS4        0
#define CE_PLATFORM_QNX        0
#define CE_PLATFORM_RPI        0
#define CE_PLATFORM_STEAMLINK  0
#define CE_PLATFORM_WINDOWS    0
#define CE_PLATFORM_WINRT      0
#define CE_PLATFORM_XBOXONE    0

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Compilers
#if defined(__clang__)
// clang defines __GNUC__ or _MSC_VER
#	undef  CE_COMPILER_CLANG
#	define CE_COMPILER_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#	if defined(__clang_analyzer__)
#		undef  CE_COMPILER_CLANG_ANALYZER
#		define CE_COMPILER_CLANG_ANALYZER 1
#	endif // defined(__clang_analyzer__)
#elif defined(_MSC_VER)
#	undef  CE_COMPILER_MSVC
#	define CE_COMPILER_MSVC _MSC_VER
#elif defined(__GNUC__)
#	undef  CE_COMPILER_GCC
#	define CE_COMPILER_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#	error "CE_COMPILER_* is not defined!"
#endif //

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Architectures
#if defined(__arm__) || \
    defined(__aarch64__) || \
    defined(_M_ARM)
#	undef  CE_CPU_ARM
#	define CE_CPU_ARM 1
#	define CE_CACHE_LINE_SIZE 64
#elif defined(__MIPSEL__) || \
      defined(__mips_isa_rev) || \
      defined(__mips64)
#	undef  CE_CPU_MIPS
#	define CE_CPU_MIPS 1
#	define CE_CACHE_LINE_SIZE 64
#elif defined(_M_PPC) || \
      defined(__powerpc__) || \
      defined(__powerpc64__)
#	undef  CE_CPU_PPC
#	define CE_CPU_PPC 1
#	define CE_CACHE_LINE_SIZE 128
#elif defined(__riscv) || \
      defined(__riscv__) || \
      defined(RISCVEL)
#	undef  CE_CPU_RISCV
#	define CE_CPU_RISCV 1
#	define CE_CACHE_LINE_SIZE 64
#elif defined(_M_IX86) || \
      defined(_M_X64) || \
      defined(__i386__) || \
      defined(__x86_64__)
#	undef  CE_CPU_X86
#	define CE_CPU_X86 1
#	define CE_CACHE_LINE_SIZE 64
#else // PNaCl doesn't have CPU defined.
#	undef  CE_CPU_JIT
#	define CE_CPU_JIT 1
#	define CE_CACHE_LINE_SIZE 64
#endif //

#if defined(__x86_64__) || \
    defined(_M_X64) || \
    defined(__aarch64__) || \
    defined(__64BIT__) || \
    defined(__mips64) || \
    defined(__powerpc64__) || \
    defined(__ppc64__) || \
    defined(__LP64__)
#	undef  CE_ARCH_64BIT
#	define CE_ARCH_64BIT 64
#else
#	undef  CE_ARCH_32BIT
#	define CE_ARCH_32BIT 32
#endif //

#if CE_CPU_PPC
#	undef  CE_CPU_ENDIAN_BIG
#	define CE_CPU_ENDIAN_BIG 1
#else
#	undef  CE_CPU_ENDIAN_LITTLE
#	define CE_CPU_ENDIAN_LITTLE 1
#endif // CE_PLATFORM_

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Operating_Systems
#if defined(_DURANGO) || defined(_XBOX_ONE)
#	undef  CE_PLATFORM_XBOXONE
#	define CE_PLATFORM_XBOXONE 1
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
#		undef  CE_PLATFORM_WINDOWS
#		if !defined(WINVER) && !defined(_WIN32_WINNT)
#			if CE_ARCH_64BIT
//				When building 64-bit target Win7 and above.
#				define WINVER 0x0601
#				define _WIN32_WINNT 0x0601
#			else
//				Windows Server 2003 with SP1, Windows XP with SP2 and above
#				define WINVER 0x0502
#				define _WIN32_WINNT 0x0502
#			endif // CE_ARCH_64BIT
#		endif // !defined(WINVER) && !defined(_WIN32_WINNT)
#		define CE_PLATFORM_WINDOWS _WIN32_WINNT
#	else
#		undef  CE_PLATFORM_WINRT
#		define CE_PLATFORM_WINRT 1
#	endif
#elif defined(__ANDROID__)
// Android compiler defines __linux__
#	include <sys/cdefs.h> // Defines __BIONIC__ and includes android/api-level.h
#	undef  CE_PLATFORM_ANDROID
#	define CE_PLATFORM_ANDROID __ANDROID_API__
#elif defined(__STEAMLINK__)
// SteamLink compiler defines __linux__
#	undef  CE_PLATFORM_STEAMLINK
#	define CE_PLATFORM_STEAMLINK 1
#elif defined(__VCCOREVER__)
// RaspberryPi compiler defines __linux__
#	undef  CE_PLATFORM_RPI
#	define CE_PLATFORM_RPI 1
#elif  defined(__linux__)
#	undef  CE_PLATFORM_LINUX
#	define CE_PLATFORM_LINUX 1
#elif  defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) \
 || defined(__ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__)
#	undef  CE_PLATFORM_IOS
#	define CE_PLATFORM_IOS 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#	undef  CE_PLATFORM_OSX
#	define CE_PLATFORM_OSX __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#elif defined(__EMSCRIPTEN__)
#	undef  CE_PLATFORM_EMSCRIPTEN
#	define CE_PLATFORM_EMSCRIPTEN 1
#elif defined(__ORBIS__)
#	undef  CE_PLATFORM_PS4
#	define CE_PLATFORM_PS4 1
#elif defined(__QNX__)
#	undef  CE_PLATFORM_QNX
#	define CE_PLATFORM_QNX 1
#elif  defined(__FreeBSD__)        \
    || defined(__FreeBSD_kernel__) \
    || defined(__NetBSD__)         \
    || defined(__OpenBSD__)        \
    || defined(__DragonFly__)
#	undef  CE_PLATFORM_BSD
#	define CE_PLATFORM_BSD 1
#elif defined(__GNU__)
#	undef  CE_PLATFORM_HURD
#	define CE_PLATFORM_HURD 1
#endif //

#if !CE_CRT_NONE
// https://sourceforge.net/p/predef/wiki/Libraries/
#	if defined(__BIONIC__)
#		undef  CE_CRT_BIONIC
#		define CE_CRT_BIONIC 1
#	elif defined(_MSC_VER)
#		undef  CE_CRT_MSVC
#		define CE_CRT_MSVC 1
#	elif defined(__GLIBC__)
#		undef  CE_CRT_GLIBC
#		define CE_CRT_GLIBC (__GLIBC__ * 10000 + __GLIBC_MINOR__ * 100)
#	elif defined(__MINGW32__) || defined(__MINGW64__)
#		undef  CE_CRT_MINGW
#		define CE_CRT_MINGW 1
#	elif defined(__apple_build_version__) || defined(__ORBIS__) || defined(__EMSCRIPTEN__) || defined(__llvm__)
#		undef  CE_CRT_LIBCXX
#		define CE_CRT_LIBCXX 1
#	endif //

#	if !CE_CRT_BIONIC \
 && !CE_CRT_GLIBC  \
 && !CE_CRT_LIBCXX \
 && !CE_CRT_MINGW  \
 && !CE_CRT_MSVC   \
 && !CE_CRT_MUSL   \
 && !CE_CRT_NEWLIB
#		undef  CE_CRT_NONE
#		define CE_CRT_NONE 1
#	endif // CE_CRT_*
#endif // !CE_CRT_NONE

#define CE_PLATFORM_POSIX (0      \
        || CE_PLATFORM_ANDROID    \
        || CE_PLATFORM_EMSCRIPTEN \
        || CE_PLATFORM_BSD        \
        || CE_PLATFORM_HURD       \
        || CE_PLATFORM_IOS        \
        || CE_PLATFORM_LINUX      \
        || CE_PLATFORM_OSX        \
        || CE_PLATFORM_QNX        \
        || CE_PLATFORM_STEAMLINK  \
        || CE_PLATFORM_PS4        \
        || CE_PLATFORM_RPI        \
        )

#define CE_PLATFORM_NONE !(0      \
        || CE_PLATFORM_ANDROID    \
        || CE_PLATFORM_EMSCRIPTEN \
        || CE_PLATFORM_BSD        \
        || CE_PLATFORM_HURD       \
        || CE_PLATFORM_IOS        \
        || CE_PLATFORM_LINUX      \
        || CE_PLATFORM_OSX        \
        || CE_PLATFORM_PS4        \
        || CE_PLATFORM_QNX        \
        || CE_PLATFORM_RPI        \
        || CE_PLATFORM_STEAMLINK  \
        || CE_PLATFORM_WINDOWS    \
        || CE_PLATFORM_WINRT      \
        || CE_PLATFORM_XBOXONE    \
        )

#if CE_COMPILER_GCC
#	define CE_COMPILER_NAME "GCC " \
                CE_STRINGIZE(__GNUC__) "." \
                CE_STRINGIZE(__GNUC_MINOR__) "." \
                CE_STRINGIZE(__GNUC_PATCHLEVEL__)
#elif CE_COMPILER_CLANG
#	define CE_COMPILER_NAME "Clang " \
                CE_STRINGIZE(__clang_major__) "." \
                CE_STRINGIZE(__clang_minor__) "." \
                CE_STRINGIZE(__clang_patchlevel__)
#elif CE_COMPILER_MSVC
#	if CE_COMPILER_MSVC >= 1910 // Visual Studio 2017
#		define CE_COMPILER_NAME "MSVC 15.0"
#	elif CE_COMPILER_MSVC >= 1900 // Visual Studio 2015
#		define CE_COMPILER_NAME "MSVC 14.0"
#	elif CE_COMPILER_MSVC >= 1800 // Visual Studio 2013
#		define CE_COMPILER_NAME "MSVC 12.0"
#	elif CE_COMPILER_MSVC >= 1700 // Visual Studio 2012
#		define CE_COMPILER_NAME "MSVC 11.0"
#	elif CE_COMPILER_MSVC >= 1600 // Visual Studio 2010
#		define CE_COMPILER_NAME "MSVC 10.0"
#	elif CE_COMPILER_MSVC >= 1500 // Visual Studio 2008
#		define CE_COMPILER_NAME "MSVC 9.0"
#	else
#		define CE_COMPILER_NAME "MSVC"
#	endif //
#endif // CE_COMPILER_

#if CE_PLATFORM_ANDROID
#	define CE_PLATFORM_NAME "Android " \
                CE_STRINGIZE(CE_PLATFORM_ANDROID)
#elif CE_PLATFORM_EMSCRIPTEN
#	define CE_PLATFORM_NAME "asm.js " \
                CE_STRINGIZE(__EMSCRIPTEN_major__) "." \
                CE_STRINGIZE(__EMSCRIPTEN_minor__) "." \
                CE_STRINGIZE(__EMSCRIPTEN_tiny__)
#elif CE_PLATFORM_BSD
#	define CE_PLATFORM_NAME "BSD"
#elif CE_PLATFORM_HURD
#	define CE_PLATFORM_NAME "Hurd"
#elif CE_PLATFORM_IOS
#	define CE_PLATFORM_NAME "iOS"
#elif CE_PLATFORM_LINUX
#	define CE_PLATFORM_NAME "Linux"
#elif CE_PLATFORM_OSX
#	define CE_PLATFORM_NAME "OSX"
#elif CE_PLATFORM_PS4
#	define CE_PLATFORM_NAME "PlayStation 4"
#elif CE_PLATFORM_QNX
#	define CE_PLATFORM_NAME "QNX"
#elif CE_PLATFORM_RPI
#	define CE_PLATFORM_NAME "RaspberryPi"
#elif CE_PLATFORM_STEAMLINK
#	define CE_PLATFORM_NAME "SteamLink"
#elif CE_PLATFORM_WINDOWS
#	define CE_PLATFORM_NAME "Windows"
#elif CE_PLATFORM_WINRT
#	define CE_PLATFORM_NAME "WinRT"
#elif CE_PLATFORM_XBOXONE
#	define CE_PLATFORM_NAME "Xbox One"
#elif CE_PLATFORM_NONE
#	define CE_PLATFORM_NAME "None"
#else
#	error "Unknown CE_PLATFORM!"
#endif // CE_PLATFORM_

#if CE_CPU_ARM
#	define CE_CPU_NAME "ARM"
#elif CE_CPU_JIT
#	define CE_CPU_NAME "JIT-VM"
#elif CE_CPU_MIPS
#	define CE_CPU_NAME "MIPS"
#elif CE_CPU_PPC
#	define CE_CPU_NAME "PowerPC"
#elif CE_CPU_RISCV
#	define CE_CPU_NAME "RISC-V"
#elif CE_CPU_X86
#	define CE_CPU_NAME "x86"
#endif // CE_CPU_

#if CE_CRT_BIONIC
#	define CE_CRT_NAME "Bionic libc"
#elif CE_CRT_GLIBC
#	define CE_CRT_NAME "GNU C Library"
#elif CE_CRT_MSVC
#	define CE_CRT_NAME "MSVC C Runtime"
#elif CE_CRT_MINGW
#	define CE_CRT_NAME "MinGW C Runtime"
#elif CE_CRT_LIBCXX
#	define CE_CRT_NAME "Clang C Library"
#elif CE_CRT_NEWLIB
#	define CE_CRT_NAME "Newlib"
#elif CE_CRT_MUSL
#	define CE_CRT_NAME "musl libc"
#elif CE_CRT_NONE
#	define CE_CRT_NAME "None"
#else
#	error "Unknown CE_CRT!"
#endif // CE_CRT_

#if CE_ARCH_32BIT
#	define CE_ARCH_NAME "32-bit"
#elif CE_ARCH_64BIT
#	define CE_ARCH_NAME "64-bit"
#endif // CE_ARCH_

#endif // CE_PLATFORM_H
