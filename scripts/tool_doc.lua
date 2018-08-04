project "doc"
	kind "ConsoleApp"

	use_celib()


	files {
		path.join(CETECH_DIR, "src/tools/doc/**.c"),
	}

	copy_to_bin()

	configuration {}
