includes("Xmake/**.lua")

add_rules("mode.debug", "mode.release")

set_allowedmodes("debug", "release")
set_defaultmode("debug")
set_project("Elos")
set_version("1.0.0")
set_languages("c17", "cxx23")
set_policy("build.warning", true)
set_warnings("all", "extra")
set_policy("run.autobuild", true)
set_allowedarchs("windows|x64")
--set_runtimes(is_mode("debug") and "MDd" or "MD")

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

add_requires("directxtk")
add_packages("directxtk")

target("Elos")
	set_group("Elos")

	add_rules("ExportAPI")
	add_includedirs(".", { public = true })
	add_files("**.cpp")

	add_headerfiles("(Elos/**.h)", { install = true })

	add_links("user32", "gdi32", "dwmapi", "shcore", "Comctl32")

	add_tests("CompileSuccess", { build_should_pass = true, group = "Compilation" })
target_end()


local ignore_tests =
{
	-- Test name = Reason
    ["TestWindowNestedD2DInterop"] = "GUI application with nested windows & D2D interop",
    ["TestAppBase"] = "GUI application base",
}

local test_path = path.join(os.projectdir(), "Test")
for _, test_dir in ipairs(os.dirs(path.join(test_path, "*"))) do
	local main_file = path.join(test_dir, "Main.cpp")
	local test_name = path.basename(test_dir)

	target(test_name)
		set_group("Elos/Tests")
		add_files(test_dir .. "**.cpp")
		add_deps("Elos")
		add_defines("ELOS_TESTING")
		add_tests("CompileSuccess", { build_should_pass = true, group = "Compilation" })

		local skip_message = ignore_tests[test_name]
        if skip_message then
            on_test(function(target, opt)
                print(format("Test %s (%s)", test_name, skip_message))
                return true
            end)
        else
            add_tests("Run" .. test_name, { run_timeout = 2000 })
        end
	target_end()
end