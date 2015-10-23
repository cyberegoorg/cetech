.build/projects/gmake:
	$(PREMAKE5) --file=premake5.lua --cc=clang --simd=$(LINUX_SIMD) --runtime=$(LINUX_RUNTIME) gmake

.PHONY: linux-all
linux-all: linux-debug32 linux-release32 linux-debug64 linux-release64

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

.PHONY: test-linux-debug64
test-linux-debug64: linux-debug64
	.build/linux64/bin/cetech1_test_debug -b -r compact

.PHONY: test-linux-release64
test-linux-release: linux-release64
	.build/linux64/bin/cetech1_test -b -r compact

.PHONY: test-linux-debug64-travis
test-linux-debug-travis: linux-debug
	.build/linux64/bin/cetech1_test_debug -d yes

.PHONY: test-linux-release64-travis
test-linux-release-travis: linux-release
	.build/linux64/bin/cetech1_test -d yes
