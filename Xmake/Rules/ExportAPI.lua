rule("ExportAPI")
	on_config(function (target)
		if has_config("BuildShared") then
			target:add("defines", "ELOS_EXPORTS")
			target:set("kind", "shared")
		else
			target:set("kind", "static")
		end
	end)
rule_end()
