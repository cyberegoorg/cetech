ARCH = os.is64bit() and '64' or '32'

OS = _OS
if OS == "macosx" then
	OS = "darwin"
end

OS_ARCH = OS .. ARCH

BX_DIR = path.getabsolute("..")
BX_BUILD_DIR = path.join(BX_DIR, "bin/")
BX_THIRD_PARTY_DIR = path.join(BX_DIR, "3rdparty")

EXTERNALS_DIR = path.join(BX_DIR, "externals", "build", OS_ARCH, "release")

solution "cetech"
	configurations {
		"Debug",
		"Release",
	}

	platforms {
		"x32",
		"x64",
		"Native", -- for targets where bitness is not specified
	}

	language "C++"


dofile "toolchain.lua"
toolchain(BX_BUILD_DIR, BX_THIRD_PARTY_DIR)

function copyLib()
end

configuration { "Debug" }
	defines {
		"CETECH_DEBUG=1",
	}

includedirs {
	path.join(BX_DIR, "src/"),
	path.join(EXTERNALS_DIR),
	path.join(EXTERNALS_DIR, "include"),
}

libdirs {
	path.join(EXTERNALS_DIR, "lib"),
}

print(OS_ARCH)

dofile "corelib.lua"
dofile "tool_hash.lua"

dofile "cetech.lua"
