project "hash"
	kind "ConsoleApp"

	use_corelib()

	files {
		path.join(BX_DIR, "src/tools/hash/**.c"),
	}

	configuration {}
