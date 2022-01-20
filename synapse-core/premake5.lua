project "synapse-core"

    kind "StaticLib"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")

    -- show preprocessor output -- didn't work
    --buildoptions { "-E" }

    --[[
    filter "action:vs"
        pchheader "pch.hpp"
        pchsource "pch.cpp"
    ]]--
    filter "action:not vs"
        --pchheader "pch.h"
        pchheader "pch.hpp" -- clang-complient
        pchsource "pch.cpp"

    files
    {
        "Synapse.h",

        "Synapse/**.c",
        "Synapse/**.cpp",
        "Synapse/**.h",
        "Synapse/**.hpp",

        "SynapseAddons/**.c",
        "SynapseAddons/**.cpp",
        "SynapseAddons/**.h",
        "SynapseAddons/**.hpp",

        "external/FastNoise/**.cpp",
        "external/FastNoise/**.h",
        "external/imgui/**.cpp",
        "external/imgui/**.h",
        "external/jc_voronoi/**.cpp",
        "external/jc_voronoi/**.h",
        "external/muparser/**.cpp",
        "external/muparser/**.h",
        "external/OBJ_loader/**.cpp",
        "external/stb_image/**.cpp",
        "external/stb_image/**.h",
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLFW_INCLUDE_NONE",
        "IMGUI_IMPL_OPENGL_LOADER_GLAD",
    }

    includedirs
    {
        ".",    -- to allow orderly structured includes in files
                -- in the style #include "src/.../file.h" instead
                -- of #include "./file.h" or #include "../../file.h".
        
        -- static library inclusions
        "%{IncludeDirs.GLFW}",
        "%{IncludeDirs.glad}",
        "%{IncludeDirs.assimp}",
        "%{IncludeDirs.freetype2}",
        "%{IncludeDirs.exprtk}",
        "%{IncludeDirs.muparser}",

        -- header-only inclusions
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.boost}",

        -- embedding python (in SynapseAddons)
        "%{IncludeDirs.python}",
        "%{IncludeDirs.cpython}",
        "%{IncludeDirs.numpy}",
    }

    libdirs
    {
        "%{LibDirs.GLFW}",
        "%{LibDirs.glad}",
        "%{LibDirs.assimp}",
        "%{LibDirs.freetype2}",
        "%{LibDirs.libpng}",
        "%{LibDirs.exprtk}",
        "%{LibDirs.muparser}",
        
        -- embedding python (in SynapseAddons)
        "%{LibDirs.python}",
        
    }

    links
    {
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
        "exprtk",
        "omp",          -- OpenMP
        "python3.8",    -- embedding python (in SynapseAddons)
    }
    

    filter "configurations.Debug"
        runtime "Debug"

    filter "configurations.Release"
        runtime "Release"
    
