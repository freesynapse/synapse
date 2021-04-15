-- global settings for all projects in this workspace
--

workspace "Synapse"
    -- location of generated solution/make and build files
    location "build"

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
        optimize "Debug"
        symbols "On"

    -- enable optimization for Release builds
    filter "configurations:Release"
        defines "NDEBUG"
        --optimize "On" --> -O2
        optimize "Speed" -- --> -O3

    -- reset filter
    filter { }


-- used by downstream projects to set paths correctly
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


-- Library specific functions, called by projects as needed to 
-- setup correct linking.

-- static library inclusions
IncludeDirs = {}
IncludeDirs["glad"] = "%{wks.location}/../libraries/glad/include"
IncludeDirs["GLFW"] = "%{wks.location}/../libraries/GLFW/include"
IncludeDirs["assimp"] = "%{wks.location}/../libraries/assimp/include"
IncludeDirs["freetype2"] = "%{wks.location}/../libraries/freetype2/include"
IncludeDirs["libpng"] = "%{wks.location}/../libraries/libpng/include"
-- header inclusions
IncludeDirs["boost"] = "%{wks.location}/../include/boost"
IncludeDirs["glm"] = "%{wks.location}/../include/glm"

-- static libraries for linking
LibDirs = {}
LibDirs["glad"] = "%{wks.location}/../libraries/glad/lib"
LibDirs["GLFW"] = "%{wks.location}/../libraries/GLFW/lib"
LibDirs["assimp"] = "%{wks.location}/../libraries/assimp/lib"
LibDirs["freetype2"] = "%{wks.location}/../libraries/freetype2/lib"
LibDirs["libpng"] = "%{wks.location}/../libraries/libpng/lib"


-- projects included in this workspace -- inheriting globals from this file
include "synapse-core"
--include "./projects/2D-sandbox"
include "./projects/3D-sandbox"
--include "./projects/core-tests"
--include "./projects/fluid-sim"
--include "./projects/graph-visualization"
--include "./projects/smoke-sim"
include "./projects/bloom-filter"
--include "./projects/shader-toy"
--include "./projects/particles"
