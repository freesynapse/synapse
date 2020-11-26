#pragma once


#include "src/Application.h"
#include "src/Core.h"
#include "src/debug/Log.h"
#include "src/debug/Profiler.h"
#include "src/event/EventHandler.h"


extern Syn::Application* CreateSynapseApplication();

int main(int argc, char* argv[])
{
	// initialize core modules
	//

	// logging
	Syn::Log::open("./log.txt");
	Syn::Log::output_new_line(true);
	SYN_CORE_TRACE("logging enabled.");

	// init profiling
	#ifdef DEBUG_PROFILING
		Syn::Profiler::get().beginSession("SYN_PROFILE");
	#endif

	// output log to ImGui
	#ifndef DEBUG_IMGUI_LOG
		Syn::Log::use_stdout(true);
	#else
		Syn::Log::use_stdout(false);
	#endif
	
	#ifndef DEBUG_IMGUI_LOG
		// position debug console
		//
		HWND console = GetConsoleWindow();
		RECT r;
		GetWindowRect(console, &r);
		MoveWindow(console, 0, 0, 1440, 960, TRUE);
	#endif

	// init events
	Syn::EventHandler::init();


	// start the application
	//

	auto app = CreateSynapseApplication();
	app->run();

	delete app;


	// shutdown core modules
	//

	Syn::EventHandler::release();

	Syn::FileIOHandler::close();
	
	#ifdef DEBUG_PROFILING
		Syn::Profiler::get().endSession();
	#endif

	SYN_CORE_TRACE("closing log.");
	Syn::Log::close();


	return EXIT_SUCCESS;

}




