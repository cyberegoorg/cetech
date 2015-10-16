# .PHONY: update-3rdparty
# update-3rdparty:
# 	git subtree pull --prefix 3rdparty/premake https://github.com/premake/premake-core.git  master --squash
# 	git subtree pull --prefix 3rdparty/luajit http://luajit.org/git/luajit-2.0.git master --squash
# 	git subtree pull --prefix 3rdparty/Catch https://github.com/philsquared/Catch.git master --squash
# 	git subtree pull --prefix 3rdparty/SDL2 https://github.com/spurious/SDL-mirror.git  master --squash


#########################
# ARCH and OS detection #
#########################
UNAME := $(shell uname)
ARCH := $(shell getconf LONG_BIT)

ifeq ($(UNAME),$(filter $(UNAME),Linux Darwin))
	ifeq ($(UNAME),$(filter $(UNAME),Darwin))
		OS=darwin
	else
		OS=linux
	endif
else
	OS=windows
endif

ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
PREMAKE5 := $(ROOT_DIR)/../3rdparty/.build/bin/$(OS)$(ARCH)/premake5
#########

#########
# Utils #
#########
.PHONY: analyze
analyze:
	@echo Analyze...
	cppcheck --template=gcc --enable=all --inconclusive --std=posix ./src/

.PHONY: scan-build
scan-build: clean
	scan-build -analyze-headers make -C ${ROOT_DIR} -j 4 test-linux-debug

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

.build/projects/gmake:
	$(PREMAKE5) --file=premake5.lua --simd=$(LINUX_SIMD) --runtime=$(LINUX_RUNTIME) gmake

.PHONY: linux-all
linux-all: linux-debug32 linux-release32 linux-debug64 linux-release64

.PHONY: linux-debug
linux-debug: linux-debug${ARCH}

.PHONY: linux-develop
linux-develop: linux-develop${ARCH}

.PHONY: linux-release
linux-release: linux-release${ARCH}

.PHONY: linux-debug32
linux-debug32:  .build/projects/gmake
	+${MAKE} -C .build/projects/gmake config=debug_x32

.PHONY: linux-develop32
linux-develop32:  .build/projects/gmake
	+${MAKE} -C .build/projects/gmake config=develop_x32
	
.PHONY: linux-release32
linux-release32: .build/projects/gmake
	+${MAKE} -C .build/projects/gmake config=release_x32

.PHONY: linux-debug64
linux-debug64: .build/projects/gmake
	+${MAKE} -C .build/projects/gmake config=debug_x64

.PHONY: linux-develop64
linux-develop64:  .build/projects/gmake
	+${MAKE} -C .build/projects/gmake config=develop_x64
	
.PHONY: linux-release64
linux-release64: .build/projects/gmake
	+${MAKE} -C .build/projects/gmake config=release_x64

.PHONY: test-linux-debug
test-linux-debug: linux-debug
	.build/linux${ARCH}/bin/cetech1_test_debug -b -r compact

.PHONY: test-linux-release
test-linux-release: linux-release
	.build/linux${ARCH}/bin/cetech1_test -b -r compact

.PHONY: test-linux-debug-travis
test-linux-debug-travis: linux-debug
	.build/linux${ARCH}/bin/cetech1_test_debug -d yes

.PHONY: test-linux-release-travis
test-linux-release-travis: linux-release
	.build/linux${ARCH}/bin/cetech1_test -d yes
#########

#################
# Visual studio #
#################
WINDOWS_SIMD = fpu
WINDOWS_RUNTIME = sdl2

.build/projects/vs2013:
	$(PREMAKE5) --file=premake5.lua --simd=$(WINDOWS_SIMD) --runtime=$(WINDOWS_PLATFORM) vs2013

.build/projects/vs2015:
	$(PREMAKE5) --file=premake5.lua --simd=$(WINDOWS_SIMD) --runtime=$(WINDOWS_PLATFORM) vs2015

vs2013: .build/projects/vs2013
vs2015: .build/projects/vs2015
#################
