--------------------------------------------------------------------------------
ROOT_DIR = path.getabsolute("..")
BUILD_DIR = path.join( ROOT_DIR, ".build")

ARCH = os.is64bit() and '64' or '32'

if _OS == 'macosx' then
    _OS = 'darwin'
end

OS_ARCH = _OS .. ARCH
OS_ARCH_DIR = _OS ..'/' .. ARCH

ENGINE_SRC_DIR = path.join(ROOT_DIR, 'engine', 'src')

THIRD_PARTY = path.join(ROOT_DIR, "external")
THIRD_PARTY_BUILD = path.join( ROOT_DIR, THIRD_PARTY, ".build")
THIRD_PARTY_LIB = path.join(THIRD_PARTY_BUILD, OS_ARCH, "lib")
THIRD_PARTY_INCLUDE = path.join( THIRD_PARTY_BUILD, OS_ARCH, "include")

print(OS_ARCH)

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
      ENGINE_SRC_DIR,
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
        targetsuffix 'Develop'

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

    configuration "osx*"
        defines {
          'CETECH_PLATFORM=linux',
          'CETECH_LINUX',
          'CETECH_DARWIN'
        }

        linkoptions {
          "-rdynamic",
        }

        links {
            "m",
        }

    dofile(path.join(ROOT_DIR, "scripts", "toolchain.lua"))
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
        path.join(ENGINE_SRC_DIR, "celib", "**.cc"),
        path.join(ENGINE_SRC_DIR, "celib", "**.h"),
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
        path.join(ENGINE_SRC_DIR, "cetech", "**.cc"),
        path.join(ENGINE_SRC_DIR, "cetech", "**.h"),
    }

    links {
      'celib_static',

      "luajit",
      "enet",
      "SOIL",
      "sqlite3",
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
            "X11",
            "rt"
        }

    configuration ("osx*")
        linkoptions {
            "-framework Cocoa",
            "-framework Metal",
            "-framework QuartzCore",
            "-framework OpenGL",
            "-framework ForceFeedback",
            "-framework Carbon",
            "-framework IOKit",
            "-framework CoreAudio",
            "-framework AudioToolbox",
            "-framework AudioUnit",
        }

        links {
            'SDL2',
            'dl',
            'pthread',
            'liconv',
        }

    strip()
