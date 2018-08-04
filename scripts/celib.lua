function use_celib()
	links {
		"celib",
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

project "celib"
	kind "StaticLib"

	files {
		path.join(CETECH_DIR, "src/celib/**.h"),
		path.join(CETECH_DIR, "src/celib/**.inl"),
		path.join(CETECH_DIR, "src/celib/**.c"),
		path.join(CETECH_DIR, "src/celib/**.cpp"),
	}

	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}

	configuration {}
