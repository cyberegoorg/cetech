project "doc"
	kind "ConsoleApp"

	use_corelib()


	files {
		path.join(BX_DIR, "src/tools/doc/**.c"),
	}

	configuration {}
