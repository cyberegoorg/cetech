ARCH = os.is64bit() and '64' or '32'

OS = _OS
if OS == "macosx" then
    OS = "darwin"
end

OS_ARCH = OS .. ARCH

CETECH_DIR = path.getabsolute("..")
BUILD_DIR = path.join(CETECH_DIR, ".build")
SOURCE_DIR = path.join(CETECH_DIR, "src")
EXAMPLES_DIR = path.join(CETECH_DIR, "examples")

TOOLS_DIR = path.join(CETECH_DIR, "tools", OS_ARCH)
EXTERNALS_DIR = path.join(CETECH_DIR, "externals", "build", OS_ARCH, "release")
BIN_DIR = path.join(CETECH_DIR, "bin", OS_ARCH)

newaction {
    trigger = "doc",
    description = "Generate documentation",
    execute = function()
        local args = "" ..
                " --source " .. SOURCE_DIR ..
                " --build " .. path.join(CETECH_DIR, "docs", "gen")

        return os.execute(path.join(BIN_DIR, "doc") .. args)
    end
}

newaction {
    trigger = "hash",
    description = "Generate static hash",
    execute = function()
        local args = "" ..
                " --source " .. SOURCE_DIR

        os.execute(path.join(BIN_DIR, "hash") .. args)

        args = "" ..
                " --source " .. EXAMPLES_DIR
        os.execute(path.join(BIN_DIR, "hash") .. args)
    end
}


function copy_to_bin()
    postbuildcommands {
        "mkdir -p " .. BIN_DIR,
        "cp -f $(TARGET) " .. BIN_DIR,
    }
end

solution "cetech"
    configurations {
        "Debug",
        "Release",
    }

    platforms {
--        "x32",
        "x64",
        "Native", -- for targets where bitness is not specified
    }

    language "C++"

dofile "toolchain.lua"
toolchain(BUILD_DIR, "")

function copyLib()
end

    includedirs {
        path.join(CETECH_DIR, "src"),
        path.join(EXTERNALS_DIR),
        path.join(EXTERNALS_DIR, "include"),
    }

    libdirs {
        path.join(EXTERNALS_DIR, "lib"),
    }

	configuration { "Debug" }
		targetsuffix "_debug"

	configuration { "Release" }
		targetsuffix ""

    configuration { "Debug" }
        defines {
            "CETECH_DEBUG=1",
        }

    configuration { }



    dofile "celib.lua"

    dofile "tool_hash.lua"
    dofile "tool_doc.lua"

    dofile "cetech.lua"
