
#include <iostream>
#include "./Profiler.h"
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <map>


void parseLog(const char* _entry)
{
	std::string fnc(_entry);

	std::cout << "fnc pre:  " << fnc << '\n';
	size_t firstParanthesis = fnc.find('(');
	// reverse find 1) the start of the fnc, or 2) the space before the fnc name
	size_t lastSpace = fnc.substr(0, firstParanthesis).rfind(' ');
	// use the substring leading up to first paranthesis (since this has shifted, adjustment by lastSpace).
	std::string fncName = lastSpace == std::string::npos ? fnc.substr(0, firstParanthesis) : fnc.substr(lastSpace+1, firstParanthesis-lastSpace);
	// remove argument types, replace with '()'.
	std::cout << "name:     " << fncName << '\n';
	fnc = fncName.substr(0, firstParanthesis) + "()";
	std::cout << "fnc post: " << fnc << "\n\n";
}

float min(float a, float b) { return a < b ? a : b; }
float max(float a, float b) { return a > b ? a : b; }
#define MIN(a, b) ((a)<(b) ? (a) : (b))
#define MAX(a, b) ((a)>(b) ? (a) : (b))
#define CLAMP(x, lo, hi) MIN(MAX(x, lo), hi)

float clamp(float x, float lo, float hi)
{
	float r = MIN(MAX(x, lo), hi);
	std::cout << "(" << x << ", " << lo << ", " << hi << ") = " << r << '\n';
	return r;
}

int main(int argc, char* argv[])
{
	clamp(-0.1f, -0.4f, 10.0f);	// - 0.1
	clamp(0.1, 0.5, 10.0f);		//   0.5
	clamp(9.0f, 0.0f, 10.0f);	//   9.0
	clamp(12.0f, 0.0f, 10.0f);	//  10.0

	std::cout << CLAMP(-0.1f, -0.4f, 10.0f) << std::endl;	// - 0.1
	std::cout << CLAMP(0.1, 0.5, 10.0f) << std::endl;		//   0.5
	std::cout << CLAMP(9.0f, 0.0f, 10.0f) << std::endl;	//   9.0
	std::cout << CLAMP(12.0f, 0.0f, 10.0f) << std::endl;	//  10.0
	std::cout << CLAMP(1.0f, 0.0f, 10.0f) << std::endl;	//  10.0

	//std::vector<std::string> entries;
	//entries.push_back("static void FluidFramebuffer::resize(uint32_t, uint32_t)::RenderCommand89::execute(void *): Framebuffer 'front_velocity' [ 383x258 ] created/resized.");
	//entries.push_back("static void Syn::ShaderLibrary::add(const std::string &, const Ref<Syn::Shader> &): shader 'fluidJacobi' added to library.");
	//entries.push_back("int Syn::Window::init(bool): OpenGL renderer: Mesa Intel(R) UHD Graphics 620 (KBL GT2)");
	//entries.push_back("Syn::Font::Font(const char *, const int &, const Ref<Syn::Shader> &): using static shaders.");
	//
	//for (auto& entry : entries)
	//	parseLog(entry.c_str());

	return 0;
}

