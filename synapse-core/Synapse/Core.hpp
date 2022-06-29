
#pragma once


#include <assert.h>

#include "Synapse/Debug/Log.hpp"
#include "Synapse/Debug/Error.hpp"
#include "FastNoise/FastNoise.h"


// UTILITY //
//

static constexpr int RETURN_SUCCESS = 0;
static constexpr int RETURN_FAILURE = -1;


// EVENTS //
//

// macros for binding of event handler functions (or methods)
#define SYN_EVENT_STATIC_FNC(f) std::bind(&f, std::placeholders::_1)
#define SYN_EVENT_MEMBER_FNC(f) std::bind(&f, this, std::placeholders::_1)


// LOG //
//

// macro for definition of the function signature used in Log::Log.
#ifdef _MSC_VER
	#define __func__ __FUNCSIG__
#else
	#define __func__ __PRETTY_FUNCTION__
#endif


// RENDERER //
//

// screen size, temporarily as a definition -- set by ImGui
//#define SCREEN_WIDTH	1920
//#define SCREEN_HEIGHT	1080
#define SCREEN_WIDTH	1440
#define SCREEN_HEIGHT	 960

#define SCREEN_WIDTH_F	(float)SCREEN_WIDTH
#define SCREEN_HEIGHT_F	(float)SCREEN_HEIGHT

// static openGL vertex attribute locations
#define VERTEX_ATTRIB_LOCATION_POSITION		0
#define VERTEX_ATTRIB_LOCATION_NORMAL		1
#define VERTEX_ATTRIB_LOCATION_TANGENT		2
#define VERTEX_ATTRIB_LOCATION_BITANGENT	3
#define VERTEX_ATTRIB_LOCATION_UV			4
#define VERTEX_ATTRIB_LOCATION_COLOR		5

// color packing/unpacking macros
#define RGBA8i(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)
#define RGBA8f(r, g, b, a) ((uint32_t)(r * 255) << 24 | (uint32_t)(g * 255) << 16 | (uint32_t)(b * 255) << 8 | (uint32_t)(a * 255))
#define RGBA8_EXTRACT_FLOAT(value, offset) (((value >> offset) & 0xff) / 255.0f)


// DEBUG //
//


// Only run one frame of rendering, then quit (through Application::run()).
// Also turns on all debugging features, including parsing of the rendering
// command queue.

//#define DEBUG_ONE_FRAME

// memory
#define DEBUG_MEMORY_TOTAL				// track total heap usage
#define DEBUG_MEMORY_ALLOCATION			// general memory tracking
#define DEBUG_MEMORY_STL_ALLOCATION		// track STL container access and reallocation

// embedded Python
#define DEBUG_PYTHON_EMBEDDING

// threading
#define DEBUG_THREADPOOL

// profiling of engine performance
#define DEBUG_PROFILING

// log to ImGui instead of stdout
// N.B.! also #define:d in Log.h due to circular inclusions
#define DEBUG_IMGUI_LOG

// input
//#define DEBUG_KEYS_BUTTONS
//#define DEBUG_CURSOR_MOVE

// events
//#define DEBUG_EVENTS

// rendering
//#define DEBUG_RENDERER_2D
#define DEBUG_MESH_NORMAL_SHADER
#define DEBUG_MESH_TANGENT_SHADER
#define DEBUG_MESH_BITANGENT_SHADER
//#define DEBUG_MESH
//#define DEBUG_MESH_TERRAIN
#define DEBUG_FRAMEBUFFER
#define DEBUG_VERTEX_ARRAY
#define DEBUG_VERTEX_BUFFER

// shaders
//#define DEBUG_SHADER_SETUP
//#define DEBUG_UNIFORMS

// materials
//#define DEBUG_TEXTURES
//#define DEBUG_MATERIALS

// OpenGL API
#define DEBUG_OPENGL_API


#ifdef DEBUG_ONE_FRAME
	#define DEBUG_MEMORY_TOTAL
	#define DEBUG_MEMORY_ALLOCATION
	#define DEBUG_MEMORY_STL_ALLOCATION
	#define DEBUG_PROFILING
	#define DEBUG_IMGUI_LOG
	#define DEBUG_KEYS_BUTTONS
	#define DEBUG_CURSOR_MOVE
	#define DEBUG_EVENTS
	#define DEBUG_RENDERER_2D
	#define DEBUG_MESH_NORMAL_SHADER
	#define DEBUG_MESH_TANGENT_SHADER
	#define DEBUG_MESH_BITANGENT_SHADER
	#define DEBUG_MESH
	#define DEBUG_MESH_TERRAIN
	#define DEBUG_FRAMEBUFFER
	#define DEBUG_VERTEX_ARRAY
	#define DEBUG_VERTEX_BUFFER
	#define DEBUG_SHADER_SETUP
	#define DEBUG_UNIFORMS
	#define DEBUG_TEXTURES
	#define DEBUG_MATERIALS
	#define DEBUG_OPENGL_API
	#define DEBUG_PYTHON_EMBEDDING
	#define DEBUG_THREADPOOL
#endif


#define SYNAPSE_DEBUG
#ifdef SYNAPSE_DEBUG
	// core macros
	#define SYN_CORE_TRACE(...)			Syn::Log::setCoreColor();\
										Syn::Log::log(__func__, __VA_ARGS__);\
										Syn::Log::resetColor();

	#define SYN_CORE_FATAL_ERROR(...)	Syn::Log::setErrorColor();\
										Syn::Error::raise_fatal_error(nullptr, __func__, __VA_ARGS__);\
										Syn::Log::resetColor();

	#define SYN_CORE_ERROR(...)			Syn::Log::setErrorColor();\
										Syn::Error::raise_error(nullptr, __func__, __VA_ARGS__);\
										Syn::Log::resetColor();

	#define SYN_CORE_WARNING(...)		Syn::Log::setWarningColor();\
										Syn::Error::raise_warning(nullptr, __func__, __VA_ARGS__);\
										Syn::Log::resetColor();

	// client macros
	#define SYN_TRACE(...)				Syn::Log::setClientColor();\
										Syn::Log::log(__func__, __VA_ARGS__);\
										Syn::Log::resetColor();

	#define SYN_ERROR(...)				Syn::Log::setClientColor();\
										Syn::Error::raise_error(nullptr, __func__, __VA_ARGS__);\
										Syn::Log::resetColor();

	#define SYN_FATAL_ERROR(...)		Syn::Log::setClientColor();\
										Syn::Error::raise_fatal_error(nullptr, __func__, __VA_ARGS__);\
										Syn::Log::resetColor();

	#define SYN_WARNING(...)			Syn::Log::setClientColor();\
										Syn::Error::raise_warning(nullptr, __func__, __VA_ARGS__);\
										Syn::Log::resetColor();
#else
	#define SYN_CORE_TRACE(...)
	#define SYN_CORE_ERROR(...)
	#define SYN_CORE_FATAL_ERROR(...)
	#define SYN_CORE_WARNING(...)

	#define SYN_TRACE(...)
	#define SYN_ERROR(...)
	#define SYN_FATAL_ERROR(...)
	#define SYN_WARNING(...)
#endif	// SYNAPSE_DEBUG


#define SYNAPSE_ASSERT
#ifdef SYNAPSE_ASSERT
	#include "Synapse/Debug/DebugBreak.h"
	#define SYN_CORE_ASSERT_NO_MESSAGE(condition) { if (!(condition)) { SYN_CORE_ERROR("Assertion failed."); debug_break(); } }
	#define SYN_CORE_ASSERT_MESSAGE(condition, ...) { if (!(condition)) { SYN_CORE_ERROR("Assertion failed: ", __VA_ARGS__); debug_break(); } }

	#define SYN_ASSERT_NO_MESSAGE(condition) { if (!(condition)) { SYN_ERROR("Assertion failed."); } }
	#define SYN_ASSERT_MESSAGE(condition, ...) { if (!(condition)) { SYN_ERROR("Assertion failed: ", __VA_ARGS__); } }

	#define SYN_ASSERT_RESOLVE(arg0, arg1, macro, ...) macro

	#define SYN_CORE_ASSERT(...) SYN_ASSERT_RESOLVE(__VA_ARGS__, SYN_CORE_ASSERT_MESSAGE, SYN_CORE_ASSERT_NO_MESSAGE)(__VA_ARGS__)
	#define SYN_ASSERT(...) SYN_ASSERT_RESOLVE(__VA_ARGS__, SYN_ASSERT_MESSAGE, SYN_ASSERT_NO_MESSAGE)(__VA_ARGS__)
#else
	#include <assert.h>
	#define SYN_CORE_ASSERT(condtion) assert(condition)
	#define SYN_ASSERT(condtion) assert(condition)
#endif



