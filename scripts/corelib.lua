function use_corelib()
	links {
		"corelib",
		"iconv",
		"z",

		path.join(EXTERNALS_DIR, "lib", "libyaml_static.a"),
		path.join(EXTERNALS_DIR, "lib", "libSDL2.a"),
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

end

project "corelib"
	kind "StaticLib"

	files {
		path.join(BX_DIR, "src/corelib/**.h"),
		path.join(BX_DIR, "src/corelib/**.inl"),
		path.join(BX_DIR, "src/corelib/**.c"),
		path.join(BX_DIR, "src/corelib/**.cpp"),
	}

	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}

	configuration {}
