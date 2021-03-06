
dir = {
	base = os.getcwd() .. "/",
}

dir.bin = dir.base .. "bin_" .. (_ACTION or "") .. "/"
bindir = dir.bin

workspace "StarVFS"
	configurations { "Debug", "Release", }
	platforms { "x64", "x32", }
	language "C++"
	
	startproject "svfs"	
	location(dir.bin)
	basedir "."
	debugdir "."
	targetdir(bindir)
  
	links {
	}
	defines {
	}
	includedirs {
		".",
		"..",
		bindir,
	}	
	
	flags { 
		"NoMinimalRebuild", 
		"MultiProcessorCompile", 
	}
	
	filter "system:windows"
		defines{ "WINDOWS", "OS_NAME=\"Windows\"", }
	filter "system:linux"
		defines{ "LINUX", "OS_NAME=\"Linux\"", }

	filter "action:gmake"
		defines { "GCC", }
		buildoptions{ 
			"-std=gnu++14", 
		}
		-- linkoptions{ "-Wl,--no-as-needed", }
		links { 
			"dl",	
			"pthread",
			"boost_system",
			"boost_filesystem",	
		}
		disablewarnings {
			"unused-parameter",
		}
	filter "action:vs*"
		defines { 
			"MSVC", 
		}
		disablewarnings {
			"4100", -- unreferenced formal parameter
			"4201", -- nameless struct/union
			"4127", -- conditional expression is constant		
			"4996",
			"4200",
		}	
	
	filter "platforms:x32" 
		architecture "x32"
		defines { "X32", "PLATFORM_NAME=\"X32\"", }
		targetsuffix "32"
	filter "platforms:x64" 
		architecture "x64"		
		defines { "X64", "PLATFORM_NAME=\"X64\"", }		
		targetsuffix "64"
		
	filter "configurations:Debug"
		flags { "Symbols", }
		optimize "Debug"
		warnings "Extra"
		runtime "Debug"
		defines {
			"ConfigurationName=\"Debug\"",
			"DEBUG",
		}
	filter "configurations:Release"
		flags { }		
		optimize "Speed"
		warnings "Default"	
		runtime "Release"		
		defines {
			"ConfigurationName=\"Release\"",
			"RELEASE",	
		}	
		
	filter { }
		
	local i,v
	for i,v in ipairs(os.matchfiles(os.getcwd() .. "/**/project.lua")) do
		print("Found project: " .. v)
		include(v)
	end
