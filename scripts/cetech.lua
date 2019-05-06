project "cetech_develop"
	kind "ConsoleApp"

	use_celib()

	defines {
		"CETECH_DEVELOP=1"
	}

	files {
		path.join(CETECH_DIR, "src/cetech/**.c"),
		path.join(CETECH_DIR, "src/cetech/**.h"),
		path.join(CETECH_DIR, "src/cetech/**.inl"),
		path.join(CETECH_DIR, "src/cetech/**.cpp"),
	}

	links {
		path.join(EXTERNALS_DIR, "lib", "libbgfxRelease.a"),
		path.join(EXTERNALS_DIR, "lib", "libbimgRelease.a"),
		path.join(EXTERNALS_DIR, "lib", "libbxRelease.a"),
		path.join(EXTERNALS_DIR, "lib", "libsqlite3.a"),
	}

	copy_to_bin()

	configuration { "osx" }
		links {
			"iconv",
            "objc",
		}

		linkoptions {
		}

	configuration { "linux" }
		links {
			"GL",
			"X11",
            "z"
		}


	configuration {}
