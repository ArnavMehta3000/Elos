target("ElosTestWindow")
	set_group("Elos/Tests")

	add_includedirs(".", { public = true })
	add_files("Main.cpp")
	add_extrafiles("SimpleMath.inl")

	add_deps("Elos")
	add_links("d2d1")
target_end()