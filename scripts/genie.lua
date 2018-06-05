
solution "cetech"
	configurations {
		"Debug",
		"Release",
	}

	if _ACTION == "xcode4" then
		platforms {
			"Universal",
		}
	else
		platforms {
			"x32",
			"x64",
			"Native",
		}
	end

	language "C++"

BGFX_DIR   = path.getabsolute("..")

local BGFX_BUILD_DIR = path.join(BGFX_DIR, ".build")
local EXTERNALS_DIR = path.join(BGFX_DIR, "externals", "build", "darwin64", "release")

dofile ("toolchain.lua")
if not toolchain(BGFX_BUILD_DIR, EXTERNALS_DIR) then
	return
end

function engine_project_default()
	includedirs {
		path.join(EXTERNALS_DIR),
		path.join(EXTERNALS_DIR, "include"),
		path.join(BGFX_DIR, "src"),
	}

	libdirs {
		path.join(EXTERNALS_DIR, "lib"),
	}

	flags {
		"FatalWarnings",
	}

	configuration { "vs*", "x32 or x64" }
		linkoptions {
			"/ignore:4199", -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
		}
		links { -- this is needed only for testing with GLES2/3 on Windows with VS2008
			"DelayImp",
		}

	configuration { "vs201*", "x32 or x64" }
		linkoptions { -- this is needed only for testing with GLES2/3 on Windows with VS201x
			"/DELAYLOAD:\"libEGL.dll\"",
			"/DELAYLOAD:\"libGLESv2.dll\"",
		}

	configuration { "mingw*" }
		targetextension ".exe"
		links {
			"gdi32",
			"psapi",
		}

	configuration { "vs20*", "x32 or x64" }
		links {
			"gdi32",
			"psapi",
		}

	configuration { "durango" }
		links {
			"d3d11_x",
			"d3d12_x",
			"combase",
			"kernelx",
		}

	configuration { "winphone8* or winstore8*" }
		removelinks {
			"DelayImp",
			"gdi32",
			"psapi"
		}
		links {
			"d3d11",
			"dxgi"
		}
		linkoptions {
			"/ignore:4264" -- LNK4264: archiving object file compiled with /ZW into a static library; note that when authoring Windows Runtime types it is not recommended to link with a static library that contains Windows Runtime metadata
		}

	-- WinRT targets need their own output directories or build files stomp over each other
	configuration { "x32", "winphone8* or winstore8*" }
		targetdir (path.join(BGFX_BUILD_DIR, "win32_" .. _ACTION, "bin", _name))
		objdir (path.join(BGFX_BUILD_DIR, "win32_" .. _ACTION, "obj", _name))

	configuration { "x64", "winphone8* or winstore8*" }
		targetdir (path.join(BGFX_BUILD_DIR, "win64_" .. _ACTION, "bin", _name))
		objdir (path.join(BGFX_BUILD_DIR, "win64_" .. _ACTION, "obj", _name))

	configuration { "ARM", "winphone8* or winstore8*" }
		targetdir (path.join(BGFX_BUILD_DIR, "arm_" .. _ACTION, "bin", _name))
		objdir (path.join(BGFX_BUILD_DIR, "arm_" .. _ACTION, "obj", _name))

	configuration { "mingw-clang" }
		kind "ConsoleApp"

	configuration { "android*" }
		kind "ConsoleApp"
		targetextension ".so"
		linkoptions {
			"-shared",
		}
		links {
			"EGL",
			"GLESv2",
		}

	configuration { "nacl*" }
		kind "ConsoleApp"
		targetextension ".nexe"
		links {
			"ppapi",
			"ppapi_gles2",
			"pthread",
		}

	configuration { "pnacl" }
		kind "ConsoleApp"
		targetextension ".pexe"
		links {
			"ppapi",
			"ppapi_gles2",
			"pthread",
		}

	configuration { "asmjs" }
		kind "ConsoleApp"
		targetextension ".bc"

	configuration { "linux-* or freebsd", "not linux-steamlink" }
		links {
			"X11",
			"GL",
			"pthread",
		}

		linkoptions {
			"-rdynamic",
		}


	configuration { "linux-steamlink" }
		links {
			"EGL",
			"GLESv2",
			"SDL2",
			"pthread",
		}

	configuration { "rpi" }
		links {
			"X11",
			"GLESv2",
			"EGL",
			"bcm_host",
			"vcos",
			"vchiq_arm",
			"pthread",
		}

	configuration { "osx" }
		linkoptions {
			"-framework Cocoa",
			"-framework QuartzCore",
			"-framework OpenGL",
			"-weak_framework Metal",
		}

	configuration { "ios* or tvos*" }
		kind "ConsoleApp"
		linkoptions {
			"-framework CoreFoundation",
			"-framework Foundation",
			"-framework OpenGLES",
			"-framework UIKit",
			"-framework QuartzCore",
			"-weak_framework Metal",
		}

	configuration { "xcode4", "ios" }
		kind "WindowedApp"
		files {
			path.join(BGFX_DIR, "examples/runtime/iOS-Info.plist"),
		}

	configuration { "xcode4", "tvos" }
		kind "WindowedApp"
		files {
			path.join(BGFX_DIR, "examples/runtime/tvOS-Info.plist"),
		}


	configuration { "qnx*" }
		targetextension ""
		links {
			"EGL",
			"GLESv2",
		}

	configuration {}

	--strip()
end

project ("cetech_develop")
	uuid (os.uuid("cetech_develop"))
	kind "WindowedApp"

if _ACTION == 'cmake' then
	linkoptions {
		"-L" .. path.join(EXTERNALS_DIR, "lib")
	}

	links {
		"nanomsg.a",
		"bgfxRelease.a",
		"bimgRelease.a",
		"bxRelease.a",
		"luajit.a",
		"sqlite3.a",
		"SDL2.a",
		"assimp.a",
		"IrrXML.a",
		"minizip.a",
		"aes.a",
		"libyaml_static.a",
	}


else
	links {
		path.join(EXTERNALS_DIR, "lib", "libnanomsg.a"),
		path.join(EXTERNALS_DIR, "lib", "libbgfxRelease.a"),
		path.join(EXTERNALS_DIR, "lib", "libbimgRelease.a"),
		path.join(EXTERNALS_DIR, "lib", "libbxRelease.a"),
		path.join(EXTERNALS_DIR, "lib", "libluajit.a"),
		path.join(EXTERNALS_DIR, "lib", "libsqlite3.a"),
		path.join(EXTERNALS_DIR, "lib", "libSDL2.a"),
		path.join(EXTERNALS_DIR, "lib", "libassimp.a"),
		path.join(EXTERNALS_DIR, "lib", "libIrrXML.a"),
		path.join(EXTERNALS_DIR, "lib", "libminizip.a"),
		path.join(EXTERNALS_DIR, "lib", "libaes.a"),
		path.join(EXTERNALS_DIR, "lib", "libyaml_static.a"),
	}
end
	engine_project_default()


	links {
		"anl",
		"z"
	}

	defines {
		"CETECH_DARWIN=1",
		"CETECH_DEBUG=1",
		"CETECH_DEVELOP=1",
		"CETECH_CAN_COMPILE=1",
		"CETECH_COLORED_LOG=1"
	}

	files {
		path.join(EXTERNALS_DIR, "include/mpack/", "**.c"),
		path.join(BGFX_DIR, "src", "**.c"),
		path.join(BGFX_DIR, "src", "**.cpp"),
		path.join(BGFX_DIR, "src", "**.h"),
	}
