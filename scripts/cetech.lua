project "cetech_develop"
	kind "ConsoleApp"

	use_corelib()

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
		"bgfxRelease",
		"bimgRelease",
		"bxRelease",
		"sqlite3",
		"IrrXML",
		"minizip",
		"assimp",
	}

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
			"X11"
		}


	configuration {}
