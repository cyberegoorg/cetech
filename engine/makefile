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
PREMAKE5=./3rdparty/build/bin/$(OS)$(ARCH)/premake5

#########
# Linux #
#########
.build/projects/gmake:
	$(PREMAKE5) --file=premake5.lua gmake

linux-all: linux-debug32 linux-release32 linux-debug64 linux-release64
linux-debug: linux-debug${ARCH}
linux-release: linux-release${ARCH}

linux-debug32:  .build/projects/gmake
	+${MAKE} -C .build/projects/gmake config=debug_x32
linux-release32: .build/projects/gmake
	+${MAKE} -C .build/projects/gmake config=release_x32
linux-debug64: .build/projects/gmake
	+${MAKE} -C .build/projects/gmake config=debug_x64
linux-release64: .build/projects/gmake
	+${MAKE} -C .build/projects/gmake config=release_x64
	
test_linux_debug: linux-debug
	.build/linux${ARCH}/bin/tech1_test_debug -b -r compact 
#########

analyze:
	@echo Analyze...
	cppcheck --template=gcc -I ./3rdparty/include/ --enable=all --inconclusive --std=posix ./src/

uncrustify:
	@echo Uncrustify...
	-@find ./src/ ./tests/ -regex ".*\.\(c\|cc\|h\)" -print0 | xargs -0 uncrustify -l c -c ./scripts/uncrustify.cfg --no-backup

.PHONY: doc
doc:
	doxygen ./doc/Doxyfile

clean: clean_doc
	@echo Cleaning...
	-@rm -rf .build

clean_tilda:
	@echo Cleaning tilda...
	-@find ./ -name "*~" -delete
	-@find ./ -name "~swp" -delete

clean_doc:
	@echo Cleaning doc
	-@rm -rf ./docs/html

install-git-hooks:
	cp ./scripts/hooks/* ./.git/hooks/

update-3rdparty:
	git subtree pull --prefix 3rdparty/premake https://github.com/premake/premake-core.git  master --squash
	git subtree pull --prefix 3rdparty/luajit http://luajit.org/git/luajit-2.0.git master --squash
	git subtree pull --prefix 3rdparty/Catch https://github.com/philsquared/Catch.git master --squash
	git subtree pull --prefix 3rdparty/SDL2 https://github.com/spurious/SDL-mirror.git  master --squash
