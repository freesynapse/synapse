
#include "pch.hpp"

#ifndef _WIN_32
	#include <time.h>
#endif

#include "Synapse/Application.hpp"
#include "Synapse/Core.hpp"
#include "Synapse/Debug/Error.hpp"
#include "Synapse/Debug/Profiler.hpp"
#include "Synapse/Event/EventHandler.hpp"
#include "Synapse/Event/InputEvents.hpp"
#include "Synapse/Input/InputManager.hpp"
#include "Synapse/API/Window.hpp"
#include "Synapse/Utils/Timer/TimeStep.hpp"
#include "Synapse/Utils/Random/Random.hpp"
#include "Synapse/Renderer/Renderer.hpp"

#include "Synapse/External/imgui/imgui.h"
#include "Synapse/External/imgui/imgui_internal.h"


namespace Syn {


	// static declaration
	Application* Application::s_instance = nullptr;


	//-----------------------------------------------------------------------------------
	Application::Application()
	{
		if (s_instance != nullptr)
		{
			SYN_CORE_FATAL_ERROR("application already instantiated.");
		}

		// pointers
		s_instance = this;
		m_window = MakeRef<Window>("SYNAPSE", SCREEN_WIDTH, SCREEN_HEIGHT, false);

		// initialize the renderer and the render command queue
		Renderer::create();
		Renderer::initOpenGL();

		// initialize random number generator
		Random::init();
		// set Renderer parameters from ImGui
		Renderer::initImGui();

		// create a ImGui overlay
		m_imGuiLayer = new ImGuiLayer();
		m_layerStack.pushOverlay(m_imGuiLayer);


		// init the input manager
		InputManager::init();


		// event listener
		EventHandler::register_callback(EventType::APPLICATION_EXIT, SYN_EVENT_MEMBER_FNC(Application::onEvent));
		EventHandler::register_callback(EventType::WINDOW_CLOSE, SYN_EVENT_MEMBER_FNC(Application::onEvent));
		EventHandler::register_callback(EventType::INPUT_KEY, SYN_EVENT_MEMBER_FNC(Application::onEvent));
		EventHandler::register_callback(EventType::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(Application::onEvent));


		// execute pending render commands before starting main loop
		Renderer::get().executeRenderCommands();


	}


	//-----------------------------------------------------------------------------------
	void Application::renderImGui()
	{
		SYN_PROFILE_FUNCTION();
		
		m_imGuiLayer->begin();

		//ImGui::Begin("Renderer");
		//auto& caps = Renderer::getCapabilities();
		//ImGui::Text("Vendor: %s", caps.vendor.c_str());
		//ImGui::Text("Renderer: %s", caps.renderer.c_str());
		//ImGui::Text("Version: %s", caps.version.c_str());
		//ImGui::End();

		for (Layer* layer : m_layerStack)
			layer->onImGuiRender();

		m_imGuiLayer->end();
		
	}


	//-----------------------------------------------------------------------------------
	void Application::run()
	{
		SYN_PROFILE_FUNCTION();

		#ifdef DEBUG_IMGUI_LOG
			Log::imgui_log_update();
		#endif

		while (m_bRunning)
		{			
			TimeStep::update();
			EventHandler::process_events();

			// update all layers
			for (Layer* layer : m_layerStack)
				layer->onUpdate(TimeStep::getDeltaTime());


			// periodically update ImGui log if enabled
			#ifdef DEBUG_IMGUI_LOG
				if (TimeStep::getFrameCount() % 15 == 0)
					Log::imgui_log_update();
			#endif


			// render imGui stuff (in render queue)
			static Application* app = this;
			SYN_RENDER_1(app, {
				app->renderImGui();
			});
			

			// execute the render command queue
			Renderer::get().executeRenderCommands();


			// update GLFW
			m_window->onUpdate();


			#ifdef DEBUG_ONE_FRAME
				SYN_CORE_TRACE("DEBUG_ONE_FRAME defined. Exit.");
				m_bRunning = false;
			#endif

		}

	}


	//-----------------------------------------------------------------------------------
	void Application::onEvent(Event* _event)
	{
		static ImVec2 p;
		static ImGuiWindow* window;

		switch (_event->getEventType())
		{
			case EventType::APPLICATION_EXIT:
			case EventType::WINDOW_CLOSE:
				SYN_CORE_TRACE(_event->getName(), " event recieved. Exit.");
				m_bRunning = false;
				break;
			case EventType::INPUT_KEY:
				if (dynamic_cast<KeyDownEvent*>(_event)->getKey() == SYN_KEY_ESCAPE)
				{
					m_bRunning = false;
				}
				break;
			case EventType::VIEWPORT_RESIZE:
				/* 
				* Get the minimum positions of the docker window, to be able to use correct mouse
				* positions. Otherwise, when clicked, the mouse position will be relative to the 
				* framebuffers', in pixels. More specifically (if not accounting for bar heights),
				* on this setup (Ubunutu), the mouse y coordinate will be off by 44.
				* 
				*   +---------------------------------
				*   | pos_min = (0, 22)  <--- this is needed!
				*   +---------------------------------
				*   | pos_min = (0, 22)
				*   +---------------------------------
				*   | fps: 60  VSYNC=ON
				*   | ... and so on...
				*   |
				*   |
				* 
				* I've tried to find this in the (lacking) ImGui docs, but to no avail. However,
				* I think that it may be hacked like this:
				*
				*/ 
				window = ImGui::FindWindowByName("synapse-core");
				p = window->Pos;
				Renderer::setImGuiWindowPosition(glm::ivec2(p.x, p.y));
				//SYN_CORE_TRACE("p = (", p.x, ", ", p.y, ")");
				break;
				
			default:
				SYN_CORE_TRACE(_event->getName(), " event received.");
		}

	}


	//-----------------------------------------------------------------------------------
	void Application::pushLayer(Layer* _layer)
	{
		m_layerStack.pushLayer(_layer);
	}


	//-----------------------------------------------------------------------------------
	void Application::pushOverlay(Layer* _overlay)
	{
		m_layerStack.pushOverlay(_overlay);
	}




}




