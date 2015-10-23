# .PHONY: update-3rdparty
# update-3rdparty:
# 	git subtree pull --prefix 3rdparty/premake https://github.com/premake/premake-core.git  master --squash
# 	git subtree pull --prefix 3rdparty/luajit http://luajit.org/git/luajit-2.0.git master --squash
# 	git subtree pull --prefix 3rdparty/Catch https://github.com/philsquared/Catch.git master --squash
# 	git subtree pull --prefix 3rdparty/SDL2 https://github.com/spurious/SDL-mirror.git  master --squash


#########################
# ARCH and OS detection #
#########################
ARCH := $(shell getconf LONG_BIT)
UNAME := $(shell uname)
ifeq ($(UNAME),$(filter $(UNAME),Linux Darwin FreeBSD GNU/kFreeBSD))
ifeq ($(UNAME),$(filter $(UNAME),Darwin))
OS=darwin
else
ifeq ($(UNAME),$(filter $(UNAME),FreeBSD GNU/kFreeBSD))
OS=bsd
else
OS=linux
endif
endif
else
OS=windows
endif

ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
PREMAKE5 := $(ROOT_DIR)/../3rdparty/.build/bin/$(OS)$(ARCH)/premake5
GENIE=$(ROOT_DIR)/../3rdparty/.build/bin/$(OS)$(ARCH)/genie
#########

all:
	$(GENIE) vs2008
	$(GENIE) vs2010
	$(GENIE) vs2012
	$(GENIE) vs2013
	$(GENIE) vs2015
	$(GENIE) --gcc=mingw-gcc gmake
	$(GENIE) --gcc=linux-gcc gmake
	$(GENIE) --gcc=osx gmake
	$(GENIE) --xcode=osx xcode4
	$(GENIE) --xcode=ios xcode4
	$(GENIE) --gcc=freebsd gmake

	$(GENIE) --gcc=android-arm gmake
	$(GENIE) --gcc=android-mips gmake
	$(GENIE) --gcc=android-x86 gmake
	$(GENIE) --gcc=asmjs gmake
	$(GENIE) --gcc=ios-arm gmake
	$(GENIE) --gcc=ios-simulator gmake
	$(GENIE) --gcc=nacl gmake
	$(GENIE) --gcc=nacl-arm gmake
	$(GENIE) --gcc=pnacl gmake
	$(GENIE) --gcc=rpi gmake

.build/projects/gmake-android-arm:
	$(GENIE) --gcc=android-arm gmake
android-arm-debug: .build/projects/gmake-android-arm
	$(MAKE) -R -C .build/projects/gmake-android-arm config=debug
android-arm-release: .build/projects/gmake-android-arm
	$(MAKE) -R -C .build/projects/gmake-android-arm config=release
android-arm: android-arm-debug android-arm-release

.build/projects/gmake-android-mips:
	$(GENIE) --gcc=android-mips gmake
android-mips-debug: .build/projects/gmake-android-mips
	$(MAKE) -R -C .build/projects/gmake-android-mips config=debug
android-mips-release: .build/projects/gmake-android-mips
	$(MAKE) -R -C .build/projects/gmake-android-mips config=release
android-mips: android-mips-debug android-mips-release

.build/projects/gmake-android-x86:
	$(GENIE) --gcc=android-x86 gmake
android-x86-debug: .build/projects/gmake-android-x86
	$(MAKE) -R -C .build/projects/gmake-android-x86 config=debug
android-x86-release: .build/projects/gmake-android-x86
	$(MAKE) -R -C .build/projects/gmake-android-x86 config=release
android-x86: android-x86-debug android-x86-release

.build/projects/gmake-asmjs:
	$(GENIE) --gcc=asmjs gmake
asmjs-debug: .build/projects/gmake-asmjs
	$(MAKE) -R -C .build/projects/gmake-asmjs config=debug
asmjs-release: .build/projects/gmake-asmjs
	$(MAKE) -R -C .build/projects/gmake-asmjs config=release
asmjs: asmjs-debug asmjs-release

.build/projects/gmake-linux:
	$(GENIE) --gcc=linux-gcc gmake
linux-debug32: .build/projects/gmake-linux
	$(MAKE) -R -C .build/projects/gmake-linux config=debug32
linux-release32: .build/projects/gmake-linux
	$(MAKE) -R -C .build/projects/gmake-linux config=release32
linux-debug64: .build/projects/gmake-linux
	$(MAKE) -R -C .build/projects/gmake-linux config=debug64
linux-release64: .build/projects/gmake-linux
	$(MAKE) -R -C .build/projects/gmake-linux config=release64
linux: linux-debug32 linux-release32 linux-debug64 linux-release64

.build/projects/gmake-linux-clang:
	$(GENIE) --gcc=linux-clang gmake
linux-clang-debug32: .build/projects/gmake-linux-clang
	$(MAKE) -R -C .build/projects/gmake-linux-clang config=debug32
linux-clang-release32: .build/projects/gmake-linux-clang
	$(MAKE) -R -C .build/projects/gmake-linux-clang config=release32
linux-clang-debug64: .build/projects/gmake-linux-clang
	$(MAKE) -R -C .build/projects/gmake-linux-clang config=debug64
linux-clang-release64: .build/projects/gmake-linux-clang
	$(MAKE) -R -C .build/projects/gmake-linux-clang config=release64
linux-clang: linux-clang-debug32 linux-clang-release32 linux-clang-debug64 linux-clang-release64

.build/projects/gmake-freebsd:
	$(GENIE) --gcc=freebsd gmake
freebsd-debug32: .build/projects/gmake-freebsd
	$(MAKE) -R -C .build/projects/gmake-freebsd config=debug32
freebsd-release32: .build/projects/gmake-freebsd
	$(MAKE) -R -C .build/projects/gmake-freebsd config=release32
freebsd-debug64: .build/projects/gmake-freebsd
	$(MAKE) -R -C .build/projects/gmake-freebsd config=debug64
freebsd-release64: .build/projects/gmake-freebsd
	$(MAKE) -R -C .build/projects/gmake-freebsd config=release64
freebsd: freebsd-debug32 freebsd-release32 freebsd-debug64 freebsd-release64

.build/projects/gmake-mingw-gcc:
	$(GENIE) --gcc=mingw-gcc gmake
mingw-gcc-debug32: .build/projects/gmake-mingw-gcc
	$(MAKE) -R -C .build/projects/gmake-mingw-gcc config=debug32
mingw-gcc-release32: .build/projects/gmake-mingw-gcc
	$(MAKE) -R -C .build/projects/gmake-mingw-gcc config=release32
mingw-gcc-debug64: .build/projects/gmake-mingw-gcc
	$(MAKE) -R -C .build/projects/gmake-mingw-gcc config=debug64
mingw-gcc-release64: .build/projects/gmake-mingw-gcc
	$(MAKE) -R -C .build/projects/gmake-mingw-gcc config=release64
mingw-gcc: mingw-gcc-debug32 mingw-gcc-release32 mingw-gcc-debug64 mingw-gcc-release64

.build/projects/gmake-mingw-clang:
	$(GENIE) --gcc=mingw-clang gmake
mingw-clang-debug32: .build/projects/gmake-mingw-clang
	$(MAKE) -R -C .build/projects/gmake-mingw-clang config=debug32
mingw-clang-release32: .build/projects/gmake-mingw-clang
	$(MAKE) -R -C .build/projects/gmake-mingw-clang config=release32
mingw-clang-debug64: .build/projects/gmake-mingw-clang
	$(MAKE) -R -C .build/projects/gmake-mingw-clang config=debug64
mingw-clang-release64: .build/projects/gmake-mingw-clang
	$(MAKE) -R -C .build/projects/gmake-mingw-clang config=release64
mingw-clang: mingw-clang-debug32 mingw-clang-release32 mingw-clang-debug64 mingw-clang-release64

.build/projects/vs2008:
	$(GENIE) vs2008
vs2008-debug32: .build/projects/vs2008
	devenv .build/projects/vs2008/bgfx.sln /Build "Debug|Win32"
vs2008-release32: .build/projects/vs2008
	devenv .build/projects/vs2008/bgfx.sln /Build "Release|Win32"
vs2008-debug64: .build/projects/vs2008
	devenv .build/projects/vs2008/bgfx.sln /Build "Debug|x64"
vs2008-release64: .build/projects/vs2008
	devenv .build/projects/vs2008/bgfx.sln /Build "Release|x64"
vs2008: vs2008-debug32 vs2008-release32 vs2008-debug64 vs2008-release64

.build/projects/vs2010:
	$(GENIE) vs2010

.build/projects/vs2012:
	$(GENIE) vs2012

.build/projects/vs2013:
	$(GENIE) vs2013

.build/projects/vs2015:
	$(GENIE) vs2015

.build/projects/gmake-nacl:
	$(GENIE) --gcc=nacl gmake
nacl-debug32: .build/projects/gmake-nacl
	$(MAKE) -R -C .build/projects/gmake-nacl config=debug32
nacl-release32: .build/projects/gmake-nacl
	$(MAKE) -R -C .build/projects/gmake-nacl config=release32
nacl-debug64: .build/projects/gmake-nacl
	$(MAKE) -R -C .build/projects/gmake-nacl config=debug64
nacl-release64: .build/projects/gmake-nacl
	$(MAKE) -R -C .build/projects/gmake-nacl config=release64
nacl: nacl-debug32 nacl-release32 nacl-debug64 nacl-release64

.build/projects/gmake-nacl-arm:
	$(GENIE) --gcc=nacl-arm gmake
nacl-arm-debug: .build/projects/gmake-nacl-arm
	$(MAKE) -R -C .build/projects/gmake-nacl-arm config=debug
nacl-arm-release: .build/projects/gmake-nacl-arm
	$(MAKE) -R -C .build/projects/gmake-nacl-arm config=release
nacl-arm: nacl-arm-debug32 nacl-arm-release32

.build/projects/gmake-pnacl:
	$(GENIE) --gcc=pnacl gmake
pnacl-debug: .build/projects/gmake-pnacl
	$(MAKE) -R -C .build/projects/gmake-pnacl config=debug
pnacl-release: .build/projects/gmake-pnacl
	$(MAKE) -R -C .build/projects/gmake-pnacl config=release
pnacl: pnacl-debug pnacl-release

.build/projects/gmake-osx:
	$(GENIE) --gcc=osx gmake
osx-debug32: .build/projects/gmake-osx
	$(MAKE) -C .build/projects/gmake-osx config=debug32
osx-release32: .build/projects/gmake-osx
	$(MAKE) -C .build/projects/gmake-osx config=release32
osx-debug64: .build/projects/gmake-osx
	$(MAKE) -C .build/projects/gmake-osx config=debug64
osx-release64: .build/projects/gmake-osx
	$(MAKE) -C .build/projects/gmake-osx config=release64
osx: osx-debug32 osx-release32 osx-debug64 osx-release64

.build/projects/gmake-ios-arm:
	$(GENIE) --gcc=ios-arm gmake
ios-arm-debug: .build/projects/gmake-ios-arm
	$(MAKE) -R -C .build/projects/gmake-ios-arm config=debug
ios-arm-release: .build/projects/gmake-ios-arm
	$(MAKE) -R -C .build/projects/gmake-ios-arm config=release
ios-arm: ios-arm-debug ios-arm-release

.build/projects/gmake-ios-simulator:
	$(GENIE) --gcc=ios-simulator gmake
ios-simulator-debug: .build/projects/gmake-ios-simulator
	$(MAKE) -R -C .build/projects/gmake-ios-simulator config=debug
ios-simulator-release: .build/projects/gmake-ios-simulator
	$(MAKE) -R -C .build/projects/gmake-ios-simulator config=release
ios-simulator: ios-simulator-debug ios-simulator-release

.build/projects/gmake-rpi:
	$(GENIE) --gcc=rpi gmake
rpi-debug: .build/projects/gmake-rpi
	$(MAKE) -R -C .build/projects/gmake-rpi config=debug
rpi-release: .build/projects/gmake-rpi
	$(MAKE) -R -C .build/projects/gmake-rpi config=release
rpi: rpi-debug rpi-release

#########
# Utils #
#########
.PHONY: analyze
analyze:
	@echo Analyze...
	cppcheck --template=gcc --enable=all --inconclusive --std=posix ./src/

.PHONY: scan-build32
scan-build32:
	scan-build -analyze-headers make -C ${ROOT_DIR} -j 4 test-linux-debug64
	
.PHONY: scan-build64
scan-build64:
	scan-build -analyze-headers make -C ${ROOT_DIR} -j 4 test-linux-debug64

.PHONY: uncrustify
uncrustify:
	@echo Uncrustify...
	-@find ./src/ ./tests/ -regex ".*\.\(c\|cc\|h\)" -print0 | xargs -0 uncrustify -l c -c ./scripts/uncrustify.cfg --no-backup

.PHONY: doc
doc:
	doxygen ./doc/Doxyfile
#########

#########
# Clean #
#########
.PHONY: clean
clean: clean-doc
	@echo Cleaning...
	-@rm -rf .build

.PHONY: clean-tilda
clean-tilda:
	@echo Cleaning tilda...
	-@find ./ -name "*~" -delete
	-@find ./ -name "~swp" -delete

.PHONY: clean-doc
clean-doc:
	@echo Cleaning doc
	-@rm -rf ./doc/doxygen/html
#########

#########
# Linux #
#########
LINUX_SIMD = fpu
LINUX_RUNTIME = sdl2

#include scripts/linux.mk

###########
# Windows #
###########
WINDOWS_SIMD = fpu
WINDOWS_RUNTIME = sdl2

#include scripts/windows.mk
###########
