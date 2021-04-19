workspace "libproc-test"

    location "build"
    
    -- used for storing compiler / linker settings togehter
    configurations { "Debug", "Release" }

    -- enable symbols for Debug builds
    filter "configurations:Debug"
        defines "DEBUG"
        symbols "On"
        optimize "Debug"

    -- enable optimization for Release builds
    filter "configurations:Release"
        defines "NDEBUG"
        symbols "Off"
        optimize "Speed"

    -- reset filter
    filter { }


    project "libproc-test"

        -- architecture: 'x86' or 'x86_64'
        architecture "x86_64"

        staticruntime "on"
    
        -- language to be compiled and c++ flavor
        language "C++"
        cppdialect "C++17"

        -- set flags for the compiler
        flags { "MultiProcessorCompile" }

        -- used for storing compiler / linker settings togehter
        configurations { "Debug", "Release" }

        -- enable symbols for Debug builds
        filter "configurations:Debug"
            defines "DEBUG"

        -- enable optimization for Release builds
        filter "configurations:Release"
            defines "NDEBUG"

        -- reset filter
        filter { }

        kind "ConsoleApp"

        outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
        
        targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
        objdir ("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")

        files
        {
            "src/**.c",
            "src/**.cpp",
            "stc/**.h",
            "src/**.hpp",
        }

        defines
        {
        }

        includedirs
        {
        }

        libdirs
        {
        }

        links
        {
            --"bz2",          -- dep of freetype2
            --"png",          -- dep of freetype2
            --"z",            -- zlib, dep of assmip
            --"pthread",      -- for lots of stuff
            --"dl",           -- dep of glfw
            --"X11",          -- dep of glfw (Linux only)
        }
        

        filter "configurations.Debug"
            runtime "Debug"

        filter "configurations.Release"
            runtime "Release"
        
