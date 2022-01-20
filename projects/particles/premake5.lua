project "particles"

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

        -- header-only inclusions
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.boost}",

        -- synapse inclusion
        "../../synapse-core/",

        -- assets
        "%{wks.location}",
    }

    libdirs
    {
        "%{LibDirs.GLFW}",
        "%{LibDirs.glad}",
        "%{LibDirs.assimp}",
        "%{LibDirs.freetype2}",
        "%{LibDirs.libpng}",
    }

    links
    {
        "synapse-core",
        "glfw3",
        "glad",
        "assimp",
        "freetype",
        "bz2",          -- dep of freetype2
        "png",          -- dep of freetype2
        "z",            -- zlib, dep of assmip
        "pthread",      -- for lots of stuff
        "dl",           -- dep of glfw
        "X11",          -- dep of glfw (Linux only)
    }
    

    filter "configurations.Debug"
        libdirs "../../synapse-core/build/bin/Debug-linux-x86_64/synapse-core"
        runtime "Debug"


    filter "configurations.Release"
        libdirs "../../synapse-core/build/bin/Release-linux-x86_64/synapse-core"
        runtime "Release"
    
