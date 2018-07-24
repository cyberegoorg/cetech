project "doc"
	kind "ConsoleApp"

	use_corelib()


	files {
		path.join(CETECH_DIR, "src/tools/doc/**.c"),
	}

	configuration {}
