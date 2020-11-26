
#include "pch.hpp"

#include "src/Application.h"
#include "src/Core.h"
#include "src/debug/Error.h"
#include "src/event/EventHandler.h"
#include "src/event/InputEvents.h"
#include "src/input/InputManager.h"
#include "src/API/Window.h"
#include "src/utils/timer/TimeStep.h"
#include "src/renderer/Renderer.h"

#include "external/imgui/imgui.h"


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
		m_window = Scope<Window>(new Window(SCREEN_WIDTH, SCREEN_HEIGHT, "Synapse Engine"));

		// initialize the renderer and the render command queue
		Renderer::create();
		Renderer::initOpenGL();

		// create a ImGui overlay
		m_imGuiLayer = new ImGuiLayer();
		m_layerStack.pushOverlay(m_imGuiLayer);

		// init the input manager
		InputManager::init();


		// event listener
		EventHandler::register_callback(EventType::APPLICATION_EXIT, SYN_EVENT_MEMBER_FNC(Application::onEvent));
		EventHandler::register_callback(EventType::WINDOW_CLOSE, SYN_EVENT_MEMBER_FNC(Application::onEvent));
		EventHandler::register_callback(EventType::INPUT_KEY, SYN_EVENT_MEMBER_FNC(Application::onEvent));


		// execute pending render commands before starting main loop
		Renderer::get().executeRenderCommands();


	}


	//-----------------------------------------------------------------------------------
	void Application::renderImGui()
	{
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

		}

	}


	//-----------------------------------------------------------------------------------
	void Application::onEvent(Event* _event)
	{
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




