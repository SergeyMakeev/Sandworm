-- build script
if not _ACTION then
	print "Sandworm buildscript. You should specify an action."
	os.exit(1)
end

solution "Sandworm"
	language "C++"

	flags {
		"NoManifest",
		"FloatFast",
	}

        buildoptions {
		"/wd4800",   --forcing value to bool 'true' or 'false'
	}


	debugdir( "." )

	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"_HAS_EXCEPTIONS=0",
		"_SCL_SECURE=0",
		"_SECURE_SCL=0",
	}

	location ( "Build/" .. _ACTION )

	local config_list = {
		"Debug",
		"Release",
	}

	local platform_list = {
		"x32",
		"x64",
	}

	configurations(config_list)
	platforms(platform_list)

configuration "Debug"
	defines {
		"_DEBUG",
		"_CRTDBG_MAP_ALLOC"
	}
	flags { 
		"Symbols"
	}

	libdirs {
		"ThirdParty/clang/build/Debug/lib/",
	}


configuration "Release"
	defines	{
		"NDEBUG",
	}
	flags {
		"Symbols",
		"OptimizeSpeed"
	}

	libdirs {
		"ThirdParty/clang/build/Release/lib/",
	}

configuration "x32"
	flags {
		"EnableSSE2",
	}
	defines {
		"WIN32",
	}

configuration "x64"
	defines {
		"WIN32",
	}

--  give each configuration/platform a unique output/target directory
for _, config in ipairs(config_list) do
	for _, plat in ipairs(platform_list) do
		configuration { config, plat }
		objdir( "Build/" .. _ACTION .. "/tmp/"  .. config  .. "-" .. plat )
		targetdir( "Bin/" .. _ACTION .. "/" .. config .. "-" .. plat )
	end
end

project "Sandworm"
	kind "StaticLib"

	flags {
		"NoPCH",
	}

	files {
		"Sandworm/Source/**.*", 
		"Sandworm/Include/**.*",
	}

	includedirs {
		"Sandworm/Include/",
		"ThirdParty/clang/build/include/",
		"ThirdParty/Clang/build/tools/clang/include/",
		"ThirdParty/CLang/llvm-5.0.0/include/",
		"ThirdParty/CLang/llvm-5.0.0/tools/clang/include/",
	}

project "Example"
	kind "ConsoleApp"

	flags {
		"NoPCH",
		"ExtraWarnings",
	}

	files {
		"Example/**.*", 
	}

	includedirs {
		"Sandworm/Include/",
	}

	links {
		"Sandworm",
	}

