.build/projects/vs2013:
	$(PREMAKE5) --file=premake5.lua --simd=$(WINDOWS_SIMD) --runtime=$(WINDOWS_PLATFORM) vs2013

.build/projects/vs2015:
	$(PREMAKE5) --file=premake5.lua --simd=$(WINDOWS_SIMD) --runtime=$(WINDOWS_PLATFORM) vs2015

vs2013: .build/projects/vs2013
vs2015: .build/projects/vs2015