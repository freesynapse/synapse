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

    -- for using OpenMP
    buildoptions { "-fopenmp" }

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

    -- precompiled headers
    pchheader "./synapse-core/pch.hpp"
    pchsource "./synapse-core/pch.cpp"


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
IncludeDirs["muparser"] = "%{wks.location}/../libraries/muparser/include"
-- header inclusions
IncludeDirs["boost"] = "%{wks.location}/../include/boost"
IncludeDirs["glm"] = "%{wks.location}/../include/glm"
    -- ugly hack to keep everything within the project, removes squigglies from PyWrapper.hpp|.cpp
IncludeDirs["python"] = "/usr/include/python3.8/"
IncludeDirs["cpython"] = "/usr/include/python3.8/cpython/"
IncludeDirs["numpy"] = "/usr/local/lib/python3.8/dist-packages/numpy/core/include"


-- static libraries for linking
LibDirs = {}
LibDirs["glad"] = "%{wks.location}/../libraries/glad/lib"
LibDirs["GLFW"] = "%{wks.location}/../libraries/GLFW/lib"
LibDirs["assimp"] = "%{wks.location}/../libraries/assimp/lib"
LibDirs["freetype2"] = "%{wks.location}/../libraries/freetype2/lib"
LibDirs["libpng"] = "%{wks.location}/../libraries/libpng/lib"
LibDirs["muparser"] = "%{wks.location}/../libraries/muparser/lib"
    -- same as above
LibDirs["python"] = "%{wks.location}/../libraries/python3.8/config-3.8-x86_64-linux-gnu/"


-- projects included in this workspace -- inheriting globals from this file
include "synapse-core"
--include "./projects/2D-sandbox"
--include "./projects/3D-sandbox"
--include "./projects/fluid-sim"
--include "./projects/graph-visualization"
--include "./projects/smoke-sim"
--include "./projects/bloom-filter"
--include "./projects/shader-toy"
--include "./projects/particles"
--include "./projects/integrating-python3"
include "./projects/core-tests"
include "./projects/function-explorer"
--include "./projects/sand"
--include "./projects/sand_v2"
--include "./projects/marching-squares"
--include "./projects/marching-cubes"
--include "./projects/PBR-spheres"

-- vx projects -- 
--include "./projects/vx/chunk-manager_old"
--include "./projects/vx/noise-explorer"
--include "./projects/vx/chunk-3d-noise"
--include "./projects/vx/chunk-2d-noise"
--include "./projects/vx/frustum-culling"


