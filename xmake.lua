includes("Xmake/**.lua")
includes("**/xmake.lua")

add_rules("mode.debug", "mode.release")

set_allowedmodes("debug", "release")
set_defaultmode("debug")
set_project("Elos")
set_languages("c17", "cxx23")
set_policy("build.warning", true)
set_warnings("all", "extra")
set_policy("run.autobuild", true)
set_allowedarchs("windows|x64")
set_runtimes(is_mode("debug") and "MDd" or "MD")

if is_mode("debug") then
	set_policy("preprocessor.linemarkers", true)  -- Enable preprocessor markers in debug mode
	add_defines("ELOS_BUILD_DEBUG=1")
	set_symbols("debug", "edit")  -- Enable hot reloading
end

if is_mode("release") then
	set_symbols("hidden")
	add_defines("ELOS_BUILD_DEBUG=0")
	set_strip("all")
end

add_defines("UNICODE", "_UNICODE", "NOMINMAX", "NOMCX", "NOSERVICE", "NOHELP", "WIN32_LEAN_AND_MEAN")
add_tests("CompileSuccess", { build_should_pass = true, group = "Compilation" })



target("Elos")
	set_group("Elos")

	add_rules("ExportAPI")
	add_includedirs(".", { public = true })
	add_files("**.cpp")
	add_extrafiles("**.inl")
	add_headerfiles("**.h", { install = true })

	add_links("user32", "gdi32", "dwmapi", "shcore", "Comctl32")
	--add_ldflags("/manifest", "/manifestuac:\"level='asInvoker' uiAccess='false'\"", {force = true})
target_end()
