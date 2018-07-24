function use_corelib()
	links {
		"corelib",
		--"iconv",
		"pthread",
		"z",

		path.join(EXTERNALS_DIR, "lib", "libyaml_static.a"),
		path.join(EXTERNALS_DIR, "lib", "libSDL2.a"),
	}

    configuration { "osx" }
        links {
            "iconv",
            "objc",
        }

        linkoptions {
            "-framework Cocoa",
            "-framework CoreAudio",
            "-framework AudioToolbox",
            "-framework ForceFeedback",
            "-framework CoreVideo",
            "-framework Carbon",
            "-framework IOKit",
            "-framework QuartzCore",

            "-framework OpenGL",
            "-weak_framework Metal",
        }

    configuration {}


end

project "corelib"
	kind "StaticLib"

	files {
		path.join(CETECH_DIR, "src/corelib/**.h"),
		path.join(CETECH_DIR, "src/corelib/**.inl"),
		path.join(CETECH_DIR, "src/corelib/**.c"),
		path.join(CETECH_DIR, "src/corelib/**.cpp"),
	}

	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}

	configuration {}
