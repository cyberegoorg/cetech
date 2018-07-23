project "cetech_develop"
	kind "ConsoleApp"

	use_corelib()

	defines {
		"CETECH_DEVELOP=1"
	}

	links {
		path.join(EXTERNALS_DIR, "lib", "libbgfxRelease.a"),
		path.join(EXTERNALS_DIR, "lib", "libbimgRelease.a"),
		path.join(EXTERNALS_DIR, "lib", "libbxRelease.a"),
		path.join(EXTERNALS_DIR, "lib", "libsqlite3.a"),
		path.join(EXTERNALS_DIR, "lib", "libIrrXML.a"),
		path.join(EXTERNALS_DIR, "lib", "libminizip.a"),
		path.join(EXTERNALS_DIR, "lib", "libassimp.a"),
	}

	files {
		path.join(BX_DIR, "src/cetech/**.c"),
		path.join(BX_DIR, "src/cetech/**.h"),
		path.join(BX_DIR, "src/cetech/**.inl"),
		path.join(BX_DIR, "src/cetech/**.cpp"),
	}

	configuration {}
