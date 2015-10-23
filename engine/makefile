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

include scripts/linux.mk

###########
# Windows #
###########
WINDOWS_SIMD = fpu
WINDOWS_RUNTIME = sdl2

include scripts/windows.mk
###########
