--------------------------------------------------------------------------------
ROOT_DIR = (path.getabsolute("../") .. "/")
BUILD_DIR = ROOT_DIR .. ".build/"

ARCH = os.is64bit() and '64' or '32'
OS_ARCH = _OS .. ARCH
OS_ARCH_DIR = _OS ..'/' .. ARCH

THIRD_PARTY = ROOT_DIR .. "../3rdparty/"
THIRD_PARTY_BUILD = path.join( ROOT_DIR, THIRD_PARTY, ".build")
THIRD_PARTY_LIB = path.join(THIRD_PARTY_BUILD, "lib", OS_ARCH)
THIRD_PARTY_INCLUDE = path.join( THIRD_PARTY_BUILD, "include")
THIRD_PARTY_INCLUDE_ARCH_DEP = path.join(THIRD_PARTY_INCLUDE, OS_ARCH)
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

    includedirs {
      THIRD_PARTY_INCLUDE,
      THIRD_PARTY_INCLUDE_ARCH_DEP,
      ROOT_DIR .. "src"
    }

    libdirs {
      THIRD_PARTY_LIB
    }

    defines {
        'CETECH_SIMD_FPU',
        'CETECH_SDL2'
    }

    configuration "Debug"
        defines {
          "DEBUG",
          "CETECH_DEBUG",
          "CETECH_DEVELOP",
          "CETECH_CE_INLINE_OFF",
        }

    configuration "Develop"
        targetsuffix '_develop'

        defines {
          "NDEBUG",
          "CETECH_DEVELOP",
        }
        
    configuration "linux*"
        defines {
          'CETECH_PLATFORM=linux',
          'CETECH_LINUX'
        }

        linkoptions {
          "-rdynamic",
        }

        links {
            "m",
        }
    
    dofile(THIRD_PARTY .. "bx/scripts/toolchain.lua")
    if not toolchain(BUILD_DIR, THIRD_PARTY) then
            return -- no action specified
    end

    configuration {}
    
--------------------------------------------------------------------------------
project "celib_static"
    kind "StaticLib"
    language "C++"

    defines {
        'DTHREADSAFE=1'
    }

    files {
        ROOT_DIR .. "src/celib/**.cc",
        ROOT_DIR .. "src/celib/**.h",
    }


    configuration {}
--------------------------------------------------------------------------------
-- project "cetech1_test"
--     kind "ConsoleApp"
--     language "C++"
-- 
--     links {
--       'celib_static'
--     }
-- 
--     includedirs {
--       ROOT_DIR .. "tests"
--     }
-- 
--     files {
--         ROOT_DIR .. "tests/**.cc",
--         ROOT_DIR .. "tests/**.h",
--     }
-- 
--     configuration {}
--------------------------------------------------------------------------------
project "cetech1"
    kind "ConsoleApp"
    language "C++"

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
    
    configuration "Release or Develop"
        links {
            "bgfxRelease",
        }

    configuration "Debug"
        links {
          --"bgfxDebug",
          "bgfxRelease",
        }

    configuration ("linux*")
        links {
            'SDL2',
            'dl',
            'pthread',
            "GL",
            "X11"
        }

    strip()
