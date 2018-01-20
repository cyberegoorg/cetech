/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef CT_PLATFORM_H
#define CT_PLATFORM_H

// Architecture
#define CT_ARCH_32BIT 0
#define CT_ARCH_64BIT 0

// Compiler
#define CT_COMPILER_CLANG           0
#define CT_COMPILER_CLANG_ANALYZER  0
#define CT_COMPILER_GCC             0
#define CT_COMPILER_MSVC            0

// Endianess
#define CT_CPU_ENDIAN_BIG    0
#define CT_CPU_ENDIAN_LITTLE 0

// CPU
#define CT_CPU_ARM   0
#define CT_CPU_JIT   0
#define CT_CPU_MIPS  0
#define CT_CPU_PPC   0
#define CT_CPU_RISCV 0
#define CT_CPU_X86   0

// C Runtime
#define CT_CRT_BIONIC 0
#define CT_CRT_GLIBC  0
#define CT_CRT_LIBCXX 0
#define CT_CRT_MINGW  0
#define CT_CRT_MSVC   0
#define CT_CRT_NEWLIB 0

#ifndef CT_CRT_MUSL
#	define CT_CRT_MUSL 0
#endif // CT_CRT_MUSL

#ifndef CT_CRT_NONE
#	define CT_CRT_NONE 0
#endif // CT_CRT_NONE

// Platform
#define CT_PLATFORM_ANDROID    0
#define CT_PLATFORM_EMSCRIPTEN 0
#define CT_PLATFORM_BSD        0
#define CT_PLATFORM_HURD       0
#define CT_PLATFORM_IOS        0
#define CT_PLATFORM_LINUX      0
#define CT_PLATFORM_OSX        0
#define CT_PLATFORM_PS4        0
#define CT_PLATFORM_QNX        0
#define CT_PLATFORM_RPI        0
#define CT_PLATFORM_STEAMLINK  0
#define CT_PLATFORM_WINDOWS    0
#define CT_PLATFORM_WINRT      0
#define CT_PLATFORM_XBOXONE    0

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Compilers
#if defined(__clang__)
// clang defines __GNUC__ or _MSC_VER
#	undef  CT_COMPILER_CLANG
#	define CT_COMPILER_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#	if defined(__clang_analyzer__)
#		undef  CT_COMPILER_CLANG_ANALYZER
#		define CT_COMPILER_CLANG_ANALYZER 1
#	endif // defined(__clang_analyzer__)
#elif defined(_MSC_VER)
#	undef  CT_COMPILER_MSVC
#	define CT_COMPILER_MSVC _MSC_VER
#elif defined(__GNUC__)
#	undef  CT_COMPILER_GCC
#	define CT_COMPILER_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#	error "CT_COMPILER_* is not defined!"
#endif //

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Architectures
#if defined(__arm__) || \
    defined(__aarch64__) || \
    defined(_M_ARM)
#	undef  CT_CPU_ARM
#	define CT_CPU_ARM 1
#	define CT_CACHE_LINE_SIZE 64
#elif defined(__MIPSEL__) || \
      defined(__mips_isa_rev) || \
      defined(__mips64)
#	undef  CT_CPU_MIPS
#	define CT_CPU_MIPS 1
#	define CT_CACHE_LINE_SIZE 64
#elif defined(_M_PPC) || \
      defined(__powerpc__) || \
      defined(__powerpc64__)
#	undef  CT_CPU_PPC
#	define CT_CPU_PPC 1
#	define CT_CACHE_LINE_SIZE 128
#elif defined(__riscv) || \
      defined(__riscv__) || \
      defined(RISCVEL)
#	undef  CT_CPU_RISCV
#	define CT_CPU_RISCV 1
#	define CT_CACHE_LINE_SIZE 64
#elif defined(_M_IX86) || \
      defined(_M_X64) || \
      defined(__i386__) || \
      defined(__x86_64__)
#	undef  CT_CPU_X86
#	define CT_CPU_X86 1
#	define CT_CACHE_LINE_SIZE 64
#else // PNaCl doesn't have CPU defined.
#	undef  CT_CPU_JIT
#	define CT_CPU_JIT 1
#	define CT_CACHE_LINE_SIZE 64
#endif //

#if defined(__x86_64__) || \
    defined(_M_X64) || \
    defined(__aarch64__) || \
    defined(__64BIT__) || \
    defined(__mips64) || \
    defined(__powerpc64__) || \
    defined(__ppc64__) || \
    defined(__LP64__)
#	undef  CT_ARCH_64BIT
#	define CT_ARCH_64BIT 64
#else
#	undef  CT_ARCH_32BIT
#	define CT_ARCH_32BIT 32
#endif //

#if CT_CPU_PPC
#	undef  CT_CPU_ENDIAN_BIG
#	define CT_CPU_ENDIAN_BIG 1
#else
#	undef  CT_CPU_ENDIAN_LITTLE
#	define CT_CPU_ENDIAN_LITTLE 1
#endif // CT_PLATFORM_

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Operating_Systems
#if defined(_DURANGO) || defined(_XBOX_ONE)
#	undef  CT_PLATFORM_XBOXONE
#	define CT_PLATFORM_XBOXONE 1
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
#		undef  CT_PLATFORM_WINDOWS
#		if !defined(WINVER) && !defined(_WIN32_WINNT)
#			if CT_ARCH_64BIT
//				When building 64-bit target Win7 and above.
#				define WINVER 0x0601
#				define _WIN32_WINNT 0x0601
#			else
//				Windows Server 2003 with SP1, Windows XP with SP2 and above
#				define WINVER 0x0502
#				define _WIN32_WINNT 0x0502
#			endif // CT_ARCH_64BIT
#		endif // !defined(WINVER) && !defined(_WIN32_WINNT)
#		define CT_PLATFORM_WINDOWS _WIN32_WINNT
#	else
#		undef  CT_PLATFORM_WINRT
#		define CT_PLATFORM_WINRT 1
#	endif
#elif defined(__ANDROID__)
// Android compiler defines __linux__
#	include <sys/cdefs.h> // Defines __BIONIC__ and includes android/api-level.h
#	undef  CT_PLATFORM_ANDROID
#	define CT_PLATFORM_ANDROID __ANDROID_API__
#elif defined(__STEAMLINK__)
// SteamLink compiler defines __linux__
#	undef  CT_PLATFORM_STEAMLINK
#	define CT_PLATFORM_STEAMLINK 1
#elif defined(__VCCOREVER__)
// RaspberryPi compiler defines __linux__
#	undef  CT_PLATFORM_RPI
#	define CT_PLATFORM_RPI 1
#elif  defined(__linux__)
#	undef  CT_PLATFORM_LINUX
#	define CT_PLATFORM_LINUX 1
#elif  defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) \
 || defined(__ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__)
#	undef  CT_PLATFORM_IOS
#	define CT_PLATFORM_IOS 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#	undef  CT_PLATFORM_OSX
#	define CT_PLATFORM_OSX __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#elif defined(__EMSCRIPTEN__)
#	undef  CT_PLATFORM_EMSCRIPTEN
#	define CT_PLATFORM_EMSCRIPTEN 1
#elif defined(__ORBIS__)
#	undef  CT_PLATFORM_PS4
#	define CT_PLATFORM_PS4 1
#elif defined(__QNX__)
#	undef  CT_PLATFORM_QNX
#	define CT_PLATFORM_QNX 1
#elif  defined(__FreeBSD__)        \
    || defined(__FreeBSD_kernel__) \
    || defined(__NetBSD__)         \
    || defined(__OpenBSD__)        \
    || defined(__DragonFly__)
#	undef  CT_PLATFORM_BSD
#	define CT_PLATFORM_BSD 1
#elif defined(__GNU__)
#	undef  CT_PLATFORM_HURD
#	define CT_PLATFORM_HURD 1
#endif //

#if !CT_CRT_NONE
// https://sourceforge.net/p/predef/wiki/Libraries/
#	if defined(__BIONIC__)
#		undef  CT_CRT_BIONIC
#		define CT_CRT_BIONIC 1
#	elif defined(_MSC_VER)
#		undef  CT_CRT_MSVC
#		define CT_CRT_MSVC 1
#	elif defined(__GLIBC__)
#		undef  CT_CRT_GLIBC
#		define CT_CRT_GLIBC (__GLIBC__ * 10000 + __GLIBC_MINOR__ * 100)
#	elif defined(__MINGW32__) || defined(__MINGW64__)
#		undef  CT_CRT_MINGW
#		define CT_CRT_MINGW 1
#	elif defined(__apple_build_version__) || defined(__ORBIS__) || defined(__EMSCRIPTEN__) || defined(__llvm__)
#		undef  CT_CRT_LIBCXX
#		define CT_CRT_LIBCXX 1
#	endif //

#	if !CT_CRT_BIONIC \
 && !CT_CRT_GLIBC  \
 && !CT_CRT_LIBCXX \
 && !CT_CRT_MINGW  \
 && !CT_CRT_MSVC   \
 && !CT_CRT_MUSL   \
 && !CT_CRT_NEWLIB
#		undef  CT_CRT_NONE
#		define CT_CRT_NONE 1
#	endif // CT_CRT_*
#endif // !CT_CRT_NONE

#define CT_PLATFORM_POSIX (0      \
        || CT_PLATFORM_ANDROID    \
        || CT_PLATFORM_EMSCRIPTEN \
        || CT_PLATFORM_BSD        \
        || CT_PLATFORM_HURD       \
        || CT_PLATFORM_IOS        \
        || CT_PLATFORM_LINUX      \
        || CT_PLATFORM_OSX        \
        || CT_PLATFORM_QNX        \
        || CT_PLATFORM_STEAMLINK  \
        || CT_PLATFORM_PS4        \
        || CT_PLATFORM_RPI        \
        )

#define CT_PLATFORM_NONE !(0      \
        || CT_PLATFORM_ANDROID    \
        || CT_PLATFORM_EMSCRIPTEN \
        || CT_PLATFORM_BSD        \
        || CT_PLATFORM_HURD       \
        || CT_PLATFORM_IOS        \
        || CT_PLATFORM_LINUX      \
        || CT_PLATFORM_OSX        \
        || CT_PLATFORM_PS4        \
        || CT_PLATFORM_QNX        \
        || CT_PLATFORM_RPI        \
        || CT_PLATFORM_STEAMLINK  \
        || CT_PLATFORM_WINDOWS    \
        || CT_PLATFORM_WINRT      \
        || CT_PLATFORM_XBOXONE    \
        )

#if CT_COMPILER_GCC
#	define CT_COMPILER_NAME "GCC " \
                CT_STRINGIZE(__GNUC__) "." \
                CT_STRINGIZE(__GNUC_MINOR__) "." \
                CT_STRINGIZE(__GNUC_PATCHLEVEL__)
#elif CT_COMPILER_CLANG
#	define CT_COMPILER_NAME "Clang " \
                CT_STRINGIZE(__clang_major__) "." \
                CT_STRINGIZE(__clang_minor__) "." \
                CT_STRINGIZE(__clang_patchlevel__)
#elif CT_COMPILER_MSVC
#	if CT_COMPILER_MSVC >= 1910 // Visual Studio 2017
#		define CT_COMPILER_NAME "MSVC 15.0"
#	elif CT_COMPILER_MSVC >= 1900 // Visual Studio 2015
#		define CT_COMPILER_NAME "MSVC 14.0"
#	elif CT_COMPILER_MSVC >= 1800 // Visual Studio 2013
#		define CT_COMPILER_NAME "MSVC 12.0"
#	elif CT_COMPILER_MSVC >= 1700 // Visual Studio 2012
#		define CT_COMPILER_NAME "MSVC 11.0"
#	elif CT_COMPILER_MSVC >= 1600 // Visual Studio 2010
#		define CT_COMPILER_NAME "MSVC 10.0"
#	elif CT_COMPILER_MSVC >= 1500 // Visual Studio 2008
#		define CT_COMPILER_NAME "MSVC 9.0"
#	else
#		define CT_COMPILER_NAME "MSVC"
#	endif //
#endif // CT_COMPILER_

#if CT_PLATFORM_ANDROID
#	define CT_PLATFORM_NAME "Android " \
                CT_STRINGIZE(CT_PLATFORM_ANDROID)
#elif CT_PLATFORM_EMSCRIPTEN
#	define CT_PLATFORM_NAME "asm.js " \
                CT_STRINGIZE(__EMSCRIPTEN_major__) "." \
                CT_STRINGIZE(__EMSCRIPTEN_minor__) "." \
                CT_STRINGIZE(__EMSCRIPTEN_tiny__)
#elif CT_PLATFORM_BSD
#	define CT_PLATFORM_NAME "BSD"
#elif CT_PLATFORM_HURD
#	define CT_PLATFORM_NAME "Hurd"
#elif CT_PLATFORM_IOS
#	define CT_PLATFORM_NAME "iOS"
#elif CT_PLATFORM_LINUX
#	define CT_PLATFORM_NAME "Linux"
#elif CT_PLATFORM_OSX
#	define CT_PLATFORM_NAME "OSX"
#elif CT_PLATFORM_PS4
#	define CT_PLATFORM_NAME "PlayStation 4"
#elif CT_PLATFORM_QNX
#	define CT_PLATFORM_NAME "QNX"
#elif CT_PLATFORM_RPI
#	define CT_PLATFORM_NAME "RaspberryPi"
#elif CT_PLATFORM_STEAMLINK
#	define CT_PLATFORM_NAME "SteamLink"
#elif CT_PLATFORM_WINDOWS
#	define CT_PLATFORM_NAME "Windows"
#elif CT_PLATFORM_WINRT
#	define CT_PLATFORM_NAME "WinRT"
#elif CT_PLATFORM_XBOXONE
#	define CT_PLATFORM_NAME "Xbox One"
#elif CT_PLATFORM_NONE
#	define CT_PLATFORM_NAME "None"
#else
#	error "Unknown CT_PLATFORM!"
#endif // CT_PLATFORM_

#if CT_CPU_ARM
#	define CT_CPU_NAME "ARM"
#elif CT_CPU_JIT
#	define CT_CPU_NAME "JIT-VM"
#elif CT_CPU_MIPS
#	define CT_CPU_NAME "MIPS"
#elif CT_CPU_PPC
#	define CT_CPU_NAME "PowerPC"
#elif CT_CPU_RISCV
#	define CT_CPU_NAME "RISC-V"
#elif CT_CPU_X86
#	define CT_CPU_NAME "x86"
#endif // CT_CPU_

#if CT_CRT_BIONIC
#	define CT_CRT_NAME "Bionic libc"
#elif CT_CRT_GLIBC
#	define CT_CRT_NAME "GNU C Library"
#elif CT_CRT_MSVC
#	define CT_CRT_NAME "MSVC C Runtime"
#elif CT_CRT_MINGW
#	define CT_CRT_NAME "MinGW C Runtime"
#elif CT_CRT_LIBCXX
#	define CT_CRT_NAME "Clang C Library"
#elif CT_CRT_NEWLIB
#	define CT_CRT_NAME "Newlib"
#elif CT_CRT_MUSL
#	define CT_CRT_NAME "musl libc"
#elif CT_CRT_NONE
#	define CT_CRT_NAME "None"
#else
#	error "Unknown CT_CRT!"
#endif // CT_CRT_

#if CT_ARCH_32BIT
#	define CT_ARCH_NAME "32-bit"
#elif CT_ARCH_64BIT
#	define CT_ARCH_NAME "64-bit"
#endif // CT_ARCH_

#endif // CT_PLATFORM_H
