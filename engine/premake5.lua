ROOT_DIR = (path.getabsolute(".") .. "/")
BUILD_DIR = ROOT_DIR .. ".build/"

THIRD_PARTY_BUILD = "../3rdparty/.build/"
THIRD_PARTY_LIB = THIRD_PARTY_BUILD .."lib/"
THIRD_PARTY_INCLUDE = THIRD_PARTY_BUILD .. "include/"
THIRD_PARTY_INCLUDE_ARCH_DEP = THIRD_PARTY_INCLUDE

function dispatch_third_party_path()
  local arch = os.is64bit() and '64/' or '32/'
  local os_arch = _OS .. arch

  THIRD_PARTY_LIB = THIRD_PARTY_LIB .. os_arch
  THIRD_PARTY_INCLUDE_ARCH_DEP =  THIRD_PARTY_INCLUDE_ARCH_DEP .. os_arch
end

function toolchain(build_dir)
  dispatch_third_party_path()

    location (build_dir .. "projects/" .. _ACTION)

    floatingpoint "Fast"
    warnings "Extra"

    includedirs {
      THIRD_PARTY_INCLUDE,
      THIRD_PARTY_INCLUDE_ARCH_DEP,
      ROOT_DIR .. "src"
    }

    libdirs {
      THIRD_PARTY_LIB
    }

    filter "Debug"
        defines {"DEBUG", "CETECH1_DEBUG"}

        flags {"Symbols"}
        targetsuffix '_debug'
        optimize "Off"

    filter "Release"
        optimize "Full"
        defines {"NDEBUG"}

    filter "system:linux"
        defines {'CETECH1_LINUX'}

        buildoptions {"-std=c++11", "-fPIC", "-msse2"}

	links {
	    "m",
	}

    filter {"system:linux", "platforms:x32"}
        targetdir (build_dir .. "linux32" .. "/bin")
        objdir (build_dir .. "linux32" .. "/obj")

    filter {"system:linux", "platforms:x64"}
        targetdir (build_dir .. "linux64" .. "/bin")
        objdir (build_dir .. "linux64" .. "/obj")

    filter "system:windows"
        defines {'CETECH1_WINDOWS'}

    filter {}
end
--------------------------------------------------------------------------------
solution "cyberego.org tech1"
    configurations {"Debug", "Release"}
    platforms {"native", "x32", "x64"}

    toolchain (BUILD_DIR)
--------------------------------------------------------------------------------
project "tech1_static"
    kind "StaticLib"
    language "C++"

    files {
        ROOT_DIR .. "src/**.cc",
        ROOT_DIR .. "src/**.h",
    }

    links {
      "SDL2"
    }

    excludes {
      ROOT_DIR .. "src/runtime/main.cc",
    }

    filter {}
--------------------------------------------------------------------------------
project "tech1_test"
    kind "ConsoleApp"
    language "C++"

    links {
      'tech1_static'
    }

    defines {
      'CETECH1_SIMD_FPU'
    }
    
    includedirs {
      ROOT_DIR .. "tests"
    }
    
    files {
        ROOT_DIR .. "tests/**.cc",
        ROOT_DIR .. "tests/**.h",
    }

    filter {}
--------------------------------------------------------------------------------
project "tech1"
    kind "ConsoleApp"
    language "C++"

    defines {
      'CETECH1_SIMD_FPU'
    }
    
    files {
        ROOT_DIR .. "src/runtime/*.cc",
    }

    links {
      'tech1_static'
    }

    filter {}
