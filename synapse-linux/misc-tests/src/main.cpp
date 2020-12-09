
#include <iostream>
#include "./Profiler.h"
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <map>


std::string g_str;
std::string errMsg = "";
std::string src = "";


void setErrorMsg(std::string& _err_msg)
{
	_err_msg += "0:17(63): error: no function with name 'ivec'\n";
	_err_msg += "0:17(12): error: no matching function for call to `texelFetchOffset(sampler2D, ivec2, int, error)'; candidates are:\n";
	_err_msg += "0:17(12): error:    vec4 texelFetchOffset(sampler1D, int, int, int)\n";
	_err_msg += "0:17(12): error:    ivec4 texelFetchOffset(isampler1D, int, int, int)\n";
	_err_msg += "0:17(12): error:    uvec4 texelFetchOffset(usampler1D, int, int, int)\n";
	_err_msg += "0:17(12): error:    vec4 texelFetchOffset(sampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:17(12): error:    ivec4 texelFetchOffset(isampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:17(12): error:    uvec4 texelFetchOffset(usampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:17(12): error:    vec4 texelFetchOffset(sampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:17(12): error:    ivec4 texelFetchOffset(isampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:17(12): error:    uvec4 texelFetchOffset(usampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:17(12): error:    vec4 texelFetchOffset(sampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:17(12): error:    ivec4 texelFetchOffset(isampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:17(12): error:    uvec4 texelFetchOffset(usampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:17(12): error:    vec4 texelFetchOffset(sampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:17(12): error:    ivec4 texelFetchOffset(isampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:17(12): error:    uvec4 texelFetchOffset(usampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:17(12): error:    vec4 texelFetchOffset(sampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:17(12): error:    ivec4 texelFetchOffset(isampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:17(12): error:    uvec4 texelFetchOffset(usampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:18(63): error: no function with name 'ivec'\n";
	_err_msg += "0:18(12): error: no matching function for call to `texelFetchOffset(sampler2D, ivec2, int, error)'; candidates are:\n";
	_err_msg += "0:18(12): error:    vec4 texelFetchOffset(sampler1D, int, int, int)\n";
	_err_msg += "0:18(12): error:    ivec4 texelFetchOffset(isampler1D, int, int, int)\n";
	_err_msg += "0:18(12): error:    uvec4 texelFetchOffset(usampler1D, int, int, int)\n";
	_err_msg += "0:18(12): error:    vec4 texelFetchOffset(sampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:18(12): error:    ivec4 texelFetchOffset(isampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:18(12): error:    uvec4 texelFetchOffset(usampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:18(12): error:    vec4 texelFetchOffset(sampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:18(12): error:    ivec4 texelFetchOffset(isampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:18(12): error:    uvec4 texelFetchOffset(usampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:18(12): error:    vec4 texelFetchOffset(sampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:18(12): error:    ivec4 texelFetchOffset(isampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:18(12): error:    uvec4 texelFetchOffset(usampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:18(12): error:    vec4 texelFetchOffset(sampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:18(12): error:    ivec4 texelFetchOffset(isampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:18(12): error:    uvec4 texelFetchOffset(usampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:18(12): error:    vec4 texelFetchOffset(sampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:18(12): error:    ivec4 texelFetchOffset(isampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:18(12): error:    uvec4 texelFetchOffset(usampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:19(63): error: no function with name 'ivec'\n";
	_err_msg += "0:19(12): error: no matching function for call to `texelFetchOffset(sampler2D, ivec2, int, error)'; candidates are:\n";
	_err_msg += "0:19(12): error:    vec4 texelFetchOffset(sampler1D, int, int, int)\n";
	_err_msg += "0:19(12): error:    ivec4 texelFetchOffset(isampler1D, int, int, int)\n";
	_err_msg += "0:19(12): error:    uvec4 texelFetchOffset(usampler1D, int, int, int)\n";
	_err_msg += "0:19(12): error:    vec4 texelFetchOffset(sampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:19(12): error:    ivec4 texelFetchOffset(isampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:19(12): error:    uvec4 texelFetchOffset(usampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:19(12): error:    vec4 texelFetchOffset(sampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:19(12): error:    ivec4 texelFetchOffset(isampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:19(12): error:    uvec4 texelFetchOffset(usampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:19(12): error:    vec4 texelFetchOffset(sampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:19(12): error:    ivec4 texelFetchOffset(isampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:19(12): error:    uvec4 texelFetchOffset(usampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:19(12): error:    vec4 texelFetchOffset(sampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:19(12): error:    ivec4 texelFetchOffset(isampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:19(12): error:    uvec4 texelFetchOffset(usampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:19(12): error:    vec4 texelFetchOffset(sampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:19(12): error:    ivec4 texelFetchOffset(isampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:19(12): error:    uvec4 texelFetchOffset(usampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:20(63): error: no function with name 'ivec'\n";
	_err_msg += "0:20(12): error: no matching function for call to `texelFetchOffset(sampler2D, ivec2, int, error)'; candidates are:\n";
	_err_msg += "0:20(12): error:    vec4 texelFetchOffset(sampler1D, int, int, int)\n";
	_err_msg += "0:20(12): error:    ivec4 texelFetchOffset(isampler1D, int, int, int)\n";
	_err_msg += "0:20(12): error:    uvec4 texelFetchOffset(usampler1D, int, int, int)\n";
	_err_msg += "0:20(12): error:    vec4 texelFetchOffset(sampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:20(12): error:    ivec4 texelFetchOffset(isampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:20(12): error:    uvec4 texelFetchOffset(usampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:20(12): error:    vec4 texelFetchOffset(sampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:20(12): error:    ivec4 texelFetchOffset(isampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:20(12): error:    uvec4 texelFetchOffset(usampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:20(12): error:    vec4 texelFetchOffset(sampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:20(12): error:    ivec4 texelFetchOffset(isampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:20(12): error:    uvec4 texelFetchOffset(usampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:20(12): error:    vec4 texelFetchOffset(sampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:20(12): error:    ivec4 texelFetchOffset(isampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:20(12): error:    uvec4 texelFetchOffset(usampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:20(12): error:    vec4 texelFetchOffset(sampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:20(12): error:    ivec4 texelFetchOffset(isampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:20(12): error:    uvec4 texelFetchOffset(usampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:21(12): error: no function with name 'textlFetch'\n";
	_err_msg += "0:24(64): error: no function with name 'ivec'\n";
	_err_msg += "0:24(12): error: no matching function for call to `texelFetchOffset(sampler2D, ivec2, int, error)'; candidates are:\n";
	_err_msg += "0:24(12): error:    vec4 texelFetchOffset(sampler1D, int, int, int)\n";
	_err_msg += "0:24(12): error:    ivec4 texelFetchOffset(isampler1D, int, int, int)\n";
	_err_msg += "0:24(12): error:    uvec4 texelFetchOffset(usampler1D, int, int, int)\n";
	_err_msg += "0:24(12): error:    vec4 texelFetchOffset(sampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:24(12): error:    ivec4 texelFetchOffset(isampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:24(12): error:    uvec4 texelFetchOffset(usampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:24(12): error:    vec4 texelFetchOffset(sampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:24(12): error:    ivec4 texelFetchOffset(isampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:24(12): error:    uvec4 texelFetchOffset(usampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:24(12): error:    vec4 texelFetchOffset(sampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:24(12): error:    ivec4 texelFetchOffset(isampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:24(12): error:    uvec4 texelFetchOffset(usampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:24(12): error:    vec4 texelFetchOffset(sampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:24(12): error:    ivec4 texelFetchOffset(isampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:24(12): error:    uvec4 texelFetchOffset(usampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:24(12): error:    vec4 texelFetchOffset(sampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:24(12): error:    ivec4 texelFetchOffset(isampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:24(12): error:    uvec4 texelFetchOffset(usampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:24(12): error: type mismatch\n";
	_err_msg += "0:25(64): error: no function with name 'ivec'\n";
	_err_msg += "0:25(12): error: no matching function for call to `texelFetchOffset(sampler2D, ivec2, int, error)'; candidates are:\n";
	_err_msg += "0:25(12): error:    vec4 texelFetchOffset(sampler1D, int, int, int)\n";
	_err_msg += "0:25(12): error:    ivec4 texelFetchOffset(isampler1D, int, int, int)\n";
	_err_msg += "0:25(12): error:    uvec4 texelFetchOffset(usampler1D, int, int, int)\n";
	_err_msg += "0:25(12): error:    vec4 texelFetchOffset(sampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:25(12): error:    ivec4 texelFetchOffset(isampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:25(12): error:    uvec4 texelFetchOffset(usampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:25(12): error:    vec4 texelFetchOffset(sampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:25(12): error:    ivec4 texelFetchOffset(isampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:25(12): error:    uvec4 texelFetchOffset(usampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:25(12): error:    vec4 texelFetchOffset(sampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:25(12): error:    ivec4 texelFetchOffset(isampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:25(12): error:    uvec4 texelFetchOffset(usampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:25(12): error:    vec4 texelFetchOffset(sampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:25(12): error:    ivec4 texelFetchOffset(isampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:25(12): error:    uvec4 texelFetchOffset(usampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:25(12): error:    vec4 texelFetchOffset(sampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:25(12): error:    ivec4 texelFetchOffset(isampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:25(12): error:    uvec4 texelFetchOffset(usampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:25(12): error: type mismatch\n";
	_err_msg += "0:26(64): error: no function with name 'ivec'\n";
	_err_msg += "0:26(12): error: no matching function for call to `texelFetchOffset(sampler2D, ivec2, int, error)'; candidates are:\n";
	_err_msg += "0:26(12): error:    vec4 texelFetchOffset(sampler1D, int, int, int)\n";
	_err_msg += "0:26(12): error:    ivec4 texelFetchOffset(isampler1D, int, int, int)\n";
	_err_msg += "0:26(12): error:    uvec4 texelFetchOffset(usampler1D, int, int, int)\n";
	_err_msg += "0:26(12): error:    vec4 texelFetchOffset(sampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:26(12): error:    ivec4 texelFetchOffset(isampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:26(12): error:    uvec4 texelFetchOffset(usampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:26(12): error:    vec4 texelFetchOffset(sampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:26(12): error:    ivec4 texelFetchOffset(isampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:26(12): error:    uvec4 texelFetchOffset(usampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:26(12): error:    vec4 texelFetchOffset(sampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:26(12): error:    ivec4 texelFetchOffset(isampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:26(12): error:    uvec4 texelFetchOffset(usampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:26(12): error:    vec4 texelFetchOffset(sampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:26(12): error:    ivec4 texelFetchOffset(isampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:26(12): error:    uvec4 texelFetchOffset(usampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:26(12): error:    vec4 texelFetchOffset(sampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:26(12): error:    ivec4 texelFetchOffset(isampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:26(12): error:    uvec4 texelFetchOffset(usampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:26(12): error: type mismatch\n";
	_err_msg += "0:27(64): error: no function with name 'ivec'\n";
	_err_msg += "0:27(12): error: no matching function for call to `texelFetchOffset(sampler2D, ivec2, int, error)'; candidates are:\n";
	_err_msg += "0:27(12): error:    vec4 texelFetchOffset(sampler1D, int, int, int)\n";
	_err_msg += "0:27(12): error:    ivec4 texelFetchOffset(isampler1D, int, int, int)\n";
	_err_msg += "0:27(12): error:    uvec4 texelFetchOffset(usampler1D, int, int, int)\n";
	_err_msg += "0:27(12): error:    vec4 texelFetchOffset(sampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:27(12): error:    ivec4 texelFetchOffset(isampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:27(12): error:    uvec4 texelFetchOffset(usampler2D, ivec2, int, ivec2)\n";
	_err_msg += "0:27(12): error:    vec4 texelFetchOffset(sampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:27(12): error:    ivec4 texelFetchOffset(isampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:27(12): error:    uvec4 texelFetchOffset(usampler3D, ivec3, int, ivec3)\n";
	_err_msg += "0:27(12): error:    vec4 texelFetchOffset(sampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:27(12): error:    ivec4 texelFetchOffset(isampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:27(12): error:    uvec4 texelFetchOffset(usampler2DRect, ivec2, ivec2)\n";
	_err_msg += "0:27(12): error:    vec4 texelFetchOffset(sampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:27(12): error:    ivec4 texelFetchOffset(isampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:27(12): error:    uvec4 texelFetchOffset(usampler1DArray, ivec2, int, int)\n";
	_err_msg += "0:27(12): error:    vec4 texelFetchOffset(sampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:27(12): error:    ivec4 texelFetchOffset(isampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:27(12): error:    uvec4 texelFetchOffset(usampler2DArray, ivec3, int, ivec2)\n";
	_err_msg += "0:27(12): error: type mismatch\n";
	_err_msg += "0:30(6): warning: `oN' used uninitialized\n";
	_err_msg += "0:30(21): warning: `pC' used uninitialized\n";
	_err_msg += "0:31(6): warning: `oS' used uninitialized\n";
	_err_msg += "0:31(21): warning: `pC' used uninitialized\n";
	_err_msg += "0:32(6): warning: `oE' used uninitialized\n";
	_err_msg += "0:32(21): warning: `pC' used uninitialized\n";
	_err_msg += "0:33(6): warning: `oW' used uninitialized\n";
	_err_msg += "0:33(21): warning: `pC' used uninitialized\n";
}
void setSource(std::string& _src)
{
	_src += "#version 330 core\n";
	_src += "\n";
	_src += "layout(location = 0) out vec4 frag_color;\n";
	_src += "\n";
	_src += "uniform sampler2D u_pressure_texture_sampler;\n";
	_src += "uniform sampler2D u_divergence_texture_sampler;\n";
	_src += "uniform sampler2D u_obstacles_texture_sampler;\n";
	_src += "\n";
	_src += "uniform float u_alpha;\n";
	_src += "uniform float u_inverse_beta;\n";
	_src += "\n";
	_src += "void main()\n";
	_src += "{\n";
	_src += "    ivec2 T = ivec2(gl_FragCoord.xy);\n";
	_src += "\n";
	_src += "    // find neighbours in the pressure field\n";
	_src += "    vec4 pN = texelFetchOffset(u_pressure_texture_sampler, T, 0, ivec( 0,  1));\n";
	_src += "    vec4 pS = texelFetchOffset(u_pressure_texture_sampler, T, 0, ivec( 0, -1));\n";
	_src += "    vec4 pE = texelFetchOffset(u_pressure_texture_sampler, T, 0, ivec( 1,  0));\n";
	_src += "    vec4 pW = texelFetchOffset(u_pressure_texture_sampler, T, 0, ivec(-1,  0));\n";
	_src += "    vec4 pC = textlFetch(u_pressure_texture_sampler, T, 0);\n";
	_src += "\n";
	_src += "    // find neighbouring obstacles\n";
	_src += "    vec3 oN = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec( 0,  1)).xyz;\n";
	_src += "    vec3 oS = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec( 0, -1)).xyz;\n";
	_src += "    vec3 oE = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec( 1,  0)).xyz;\n";
	_src += "    vec3 oW = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec(-1,  0)).xyz;\n";
	_src += "\n";
	_src += "    // use pressure center for solid cells\n";
	_src += "    if (oN.x > 0) pN = pC;\n";
	_src += "    if (oS.x > 0) pS = pC;\n";
	_src += "    if (oE.x > 0) pE = pC;\n";
	_src += "    if (oW.x > 0) pW = pC;\n";
	_src += "\n";
	_src += "    vec4 bC = texelFetch(u_divergence_texture_sampler, T, 0);\n";
	_src += "    frag_color = (pN + pS + pE + pW + u_alpha * bC) * u_inverse_beta;\n";
	_src += "\n";
	_src += "}\n";
	_src += "\n";
	_src += "\n";
}


inline const char* leading_blank_spaces(int _line_num, int _error_code=0)
{
	/* error code 1 = error, 2 = warning */
	
	// obs: max 10000 lines of shader code permitted
	int n = 0;
	// get number of digits in _line_num
	int numChars = _line_num > 0 ? (int)log10((double)_line_num) + 1 : 1;
	switch (_error_code)
	{
		case 0: g_str = "   "; break;
		case 1: g_str = "  E"; break;
		case 2: g_str = "  W"; break;
	}
	
	for (int i = numChars; i < 5; i++)
		g_str.append(" ");

	return g_str.c_str();
}

/* 
Function for parsing the error message for an erronous shader source (failed
compilation). The source is outputted and the lines raising errors are marked.
*/
std::string annotateShaderErrorMsg(const std::string& _source, const std::string& _error_msg)
{
	// error messages line number are given as 0:line_number(character_number). 
	// First, let's find that line number.
	// Example:
	// 0:19(2): error: value of type vec4 cannot be assigned to variable of type vec2
	
	// map of the line number and the error code (0 = error, 1 = warning)
	std::map<int, int> errorLinesMap;
	std::istringstream errorLineStream(_error_msg);
	std::string line;
	// split error message into lines and parse each line for 
	// erronous line and error code (warning or error).
	while (std::getline(errorLineStream, line, '\n'))
	{
		size_t lineNumStart = line.find('(');
		size_t lineNumEnd =  line.find(':') - 1;
		if (lineNumStart == std::string::npos || lineNumEnd == std::string::npos)
			continue;
		
		// extract the line number
		std::string s = line.substr(line.find(':')+1, lineNumEnd - lineNumStart);
		int errLineNumber = atoi(s.c_str());
		
		// find if warning or error
		int errCode;
		if (line.find("error") != std::string::npos)
			errCode = 1;
		else if (line.find("warning") != std::string::npos)
			errCode = 2;
		else
			errCode = 0;

		errorLinesMap.insert(std::pair<int, int>(errLineNumber, errCode));
	}

	// read the source line by line
	std::vector<std::string> srcLines;
	std::istringstream sstream(_source);
	int lineNumber = 1;
	
	while (std::getline(sstream, line, '\n'))
	{
		// add leading blanks and the line number to each line.
		// the erronous line will be marked with 'E' or 'W'.

		// scan the marked errors for the current line number
		int errCode = 0;
		for (auto& item : errorLinesMap)
		{
			if (item.first == lineNumber)
				errCode = item.second;
		}

		// create a line prefix with line number and optional 'E' or 'W'
		std::string linePrefix = std::string(leading_blank_spaces(lineNumber, errCode));
		linePrefix += std::to_string(lineNumber);
		linePrefix += ": ";

		line.insert(0, linePrefix);
		srcLines.push_back(line);
		lineNumber++;
		
	}

	// concatenate message for logging
	std::string logMsg = "";
	for (auto& line : srcLines)
		logMsg = logMsg + line + '\n';


	//Log::log_msg(logMsg)
	return logMsg;

}




int main(int argc, char* argv[])
{
	setErrorMsg(errMsg);
	std::cout << errMsg << '\n';
	setSource(src);
	std::cout << src << '\n';

	std::string annotated = annotateShaderErrorMsg(src, errMsg);
	std::cout << annotated << "\n\n";
	
	return 0;
}

