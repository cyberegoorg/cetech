project "hash"
	kind "ConsoleApp"

	use_corelib()

	files {
		path.join(CETECH_DIR, "src/tools/hash/**.c"),
	}

	configuration {}
