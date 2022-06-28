project "pan-and-zoom"

    kind "ConsoleApp"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")


    files
    {
        "src/**.cpp",
        "src/**.h",
    }

    defines
    {
    }

    includedirs
    {
        -- static library inclusions
        "%{IncludeDirs.GLFW}",
        "%{IncludeDirs.glad}",
        "%{IncludeDirs.assimp}",
        "%{IncludeDirs.freetype2}",
        "%{IncludeDirs.muparser}",

        -- header-only inclusions
        "%{IncludeDirs.headers}",

        -- synapse inclusion
        "%{IncludeDirs.synapse}",

        -- assets
        "%{wks.location}",

        -- embedded python
        "/usr/include/python3.8/",
        "/usr/include/python3.8/cpython/",
        "/home/iomanip/.local/lib/python3.8/site-packages/numpy/core/include/",
    }

    libdirs
    {
        "%{LibDirs.GLFW}",
        "%{LibDirs.glad}",
        "%{LibDirs.assimp}",
        "%{LibDirs.freetype2}",
        "%{LibDirs.libpng}",
        "%{LibDirs.muparser}",

        "/usr/lib/python3.8/config-3.8-x86_64-linux-gnu/"
    }

    links
    {
        "synapse-core",
        "glfw3",
        "glad",
        "assimp",
        "freetype",
        "muparser",
        "bz2",          -- dep of freetype2
        "png",          -- dep of freetype2
        "z",            -- zlib, dep of assmip
        "pthread",      -- for lots of stuff
        "dl",           -- dep of glfw
        "X11",          -- dep of glfw (Linux only)
        "python3.8",    -- dep if using embedded Python
    }
    
    filter "configurations.Debug"
        libdirs "%{wks.location}/bin/Debug-linux-x86_64/synapse-core"
        runtime "Debug"

    filter "configurations.Release"
        libdirs "%{wks.location}/bin/Release-linux-x86_64/synapse-core"
        runtime "Release"
    
