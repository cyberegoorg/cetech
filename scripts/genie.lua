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
solution "CETech"
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

function cetech_project(name, test)
    project(name)
        kind "ConsoleApp"
        language "C++"

        files {
            path.join(ENGINE_SRC_DIR, "cetech", "**.cc"),
            path.join(ENGINE_SRC_DIR, "cetech", "**.h"),
            path.join(ENGINE_SRC_DIR, "celib", "**.cc"),
            path.join(ENGINE_SRC_DIR, "celib", "**.h"),
        }

        if test then
            files {
                path.join(ENGINE_SRC_DIR, "tests_main.cc"),
            }

            excludes {
                path.join(ENGINE_SRC_DIR, "cetech", "main.cc"),
            }

            defines {
                'CETECH_TEST'
            }
        end

        links {
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
                '-pagezero_size 10000',
                '-image_base 100000000',
            }

            links {
                'SDL2',
                'dl',
                'pthread',
                'iconv',
            }

        strip()
end

cetech_project('cetech', false)
cetech_project('cetech_test', true)