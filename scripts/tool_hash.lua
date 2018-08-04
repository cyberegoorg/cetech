project "hash"
	kind "ConsoleApp"

	use_celib()

	files {
		path.join(CETECH_DIR, "src/tools/hash/**.c"),
	}

	copy_to_bin()

	configuration {}
