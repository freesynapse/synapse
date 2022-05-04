
#pragma once


#include "Synapse/Application.hpp"
#include "Synapse/Core.hpp"
#include "Synapse/Debug/Log.hpp"
#include "Synapse/Debug/Profiler.hpp"
#include "Synapse/Event/EventHandler.hpp"
#include "Synapse/Utils/Random/Random.hpp"
#include "Synapse/Memory.hpp"
#include "Synapse/Utils/Thread/ThreadPool.hpp"

#ifdef ENABLE_ADDONS
	#include "SynapseAddons/AddonsCore.hpp"
#endif
#ifdef ENABLE_ADDONS_PY_EMBEDDING
	#include "SynapseAddons/Python/PyWrapper.hpp"
#endif

#ifdef ENABLE_ADDONS_VOXEL
	#include "SynapseAddons/Voxel/VxNoise.hpp"
#endif




extern Syn::Application* CreateSynapseApplication();

int main(int argc, char* argv[])
{
	// initialize core modules
	//

	// logging
	Syn::Log::open("./log.txt");
	Syn::Log::output_new_line(true);
	SYN_CORE_TRACE("logging enabled.");

	// total memory tracking
	#ifdef DEBUG_MEMORY_TOTAL
		Syn::ProcessInfo::init();
	#endif

	// init profiling
	#ifdef DEBUG_PROFILING
		Syn::Profiler::get().beginSession("SYN_PROFILE");
	#endif

	// output log to ImGui
	//#ifndef DEBUG_IMGUI_LOG
	//	Syn::Log::use_stdout(true);
	//#else
	//	Syn::Log::use_stdout(false);
	//#endif
	Syn::Log::use_stdout(true);


	
	#ifndef DEBUG_IMGUI_LOG
		// position debug console
		//
		//HWND console = GetConsoleWindow();
		//RECT r;
		//GetWindowRect(console, &r);
		//MoveWindow(console, 0, 0, 1440, 960, TRUE);
	#endif

	// init the random number engine -- singleton
	Syn::Random::init();

	// init events -- singleton
	Syn::EventHandler::init();

	// create worker threads
	Syn::ThreadPool::get().init();

	#ifdef ENABLE_ADDONS_VOXEL
		// init voxel noise
		Syn::VxNoise::init();
	#endif

	#ifdef ENABLE_ADDONS_PY_EMBEDDING
		Syn::PyWrapper::init();
	#endif
		

	// start the application
	//

	auto app = CreateSynapseApplication();
	app->run();

	Syn::EventHandler::push_event(new Syn::ApplicationExitEvent());
	Syn::EventHandler::process_events();

	// stop application
	//
	delete app;
	

	// shutdown core modules
	//

	Syn::EventHandler::shutdown();
	Syn::FileIOHandler::shutdown();
	Syn::ThreadPool::get().shutdown();

	#ifdef ENABLE_ADDONS_PY_EMBEDDING
		Syn::PyWrapper::shutdown();
	#endif
	
	#ifdef DEBUG_PROFILING
		Syn::Profiler::get().endSession();
	#endif

	SYN_CORE_TRACE("closing log.");
	Syn::Log::close();


	return EXIT_SUCCESS;

}




