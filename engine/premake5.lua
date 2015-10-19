--------------------------------------------------------------------------------
ROOT_DIR = (path.getabsolute(".") .. "/")
BUILD_DIR = ROOT_DIR .. ".build/"

ARCH = os.is64bit() and '64' or '32'
OS_ARCH = _OS .. ARCH
OS_ARCH_DIR = _OS ..'/' .. ARCH

THIRD_PARTY_BUILD = "../3rdparty/.build/"
THIRD_PARTY_LIB = THIRD_PARTY_BUILD .."lib/" .. OS_ARCH
THIRD_PARTY_INCLUDE = THIRD_PARTY_BUILD .. "include/"
THIRD_PARTY_INCLUDE_ARCH_DEP = THIRD_PARTY_INCLUDE .. OS_ARCH

print(THIRD_PARTY_INCLUDE_ARCH_DEP)
--------------------------------------------------------------------------------
newoption {
  trigger = 'simd',
  value = "Implementation",
  description = "Chose a simd implementation",
  allowed = {
    {'fpu', "Generic implementation using FPU."}
  }
}

newoption {
  trigger = 'runtime',
  value = "Runtime",
  description = "Runtime",
  allowed = {
    {'sdl2', "SDL2"}
  }
}
--------------------------------------------------------------------------------
solution "cyberego.org tech1"
    configurations {"Debug", "Develop", "Release"}
    platforms {"native", "x32", "x64"}

    location (BUILD_DIR .. "projects/" .. _ACTION)

    targetdir (BUILD_DIR .. OS_ARCH .. "/bin")
    objdir (BUILD_DIR .. OS_ARCH .. "/obj")

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

    -- SIMD options
    filter "options:simd=fpu"
      defines {
	'CETECH_SIMD_FPU'
      }

    -- Runtime options
    filter "options:runtime=sdl2"
      defines {
        'CETECH_RUNTIME_SDL2'
      }

    filter "Debug"
        flags {"Symbols"}
        targetsuffix '_debug'
        optimize "Off"
    
        defines {
	  "DEBUG",
	  "CETECH_DEBUG",
	  "CETECH_DEVELOP",
	  "CETECH_CE_INLINE_OFF",
	}

    filter "Develop"
        targetsuffix '_develop'
        optimize "Full"
    
        defines {
	  "NDEBUG",
	  "CETECH_DEVELOP",
	}
	
    filter "Release"
	targetsuffix '_release'
        optimize "Full"

        defines {
	  "NDEBUG",
	}

    filter "system:linux"
        defines {
	  'CETECH_PLATFORM=linux',
	  'CETECH_LINUX'
	}

        linkoptions {
          "-rdynamic",
        }

        buildoptions {"-std=c++11", "-fPIC", "-msse2"}

	links {
	    "m",
	}

    filter "system:windows"
        defines {'CETECH_WINDOWS'}

    filter {}
--------------------------------------------------------------------------------
project "celib_static"
    kind "StaticLib"
    language "C++"

    files {
        ROOT_DIR .. "src/celib/**.cc",
        ROOT_DIR .. "src/celib/**.h",
    }

    removefiles {
      ROOT_DIR .. "src/celib/platform_*/**",
    }

    filter "options:runtime=sdl2"
      files {
	  ROOT_DIR .. "src/celib/platform_sdl/*.cc",
	  ROOT_DIR .. "src/celib/platform_sdl/*.h",
      }

    filter {}
--------------------------------------------------------------------------------
project "cetech1_test"
    kind "ConsoleApp"
    language "C++"

    links {
      'celib_static'
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
project "cetech1"
    kind "ConsoleApp"
    language "C++"

    excludes {
      ROOT_DIR .. "src/celib/**",
    }

    files {
	    ROOT_DIR .. "src/cetech/**.cc",
    }

    links {
      'celib_static',

      "luajit",
      "enet",
      "SOIL",
      "sqlite3"
    }
    
    filter "Release or Develop"
        links {
            "bgfxRelease",
        }

    filter "Debug"
        links {
          --"bgfxDebug",
            "bgfxRelease",
        }

   filter ("system:linux","options:runtime=sdl2")
    links {
	'SDL2',
	'dl',
	'pthread',
	"GL",
	"X11"
    }

    filter {}
