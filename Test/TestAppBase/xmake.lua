target("ElosTestAppBase")
	set_group("Elos/Tests")

	add_includedirs(".", { public = true })
	add_files("Main.cpp")

	add_deps("Elos")
target_end()