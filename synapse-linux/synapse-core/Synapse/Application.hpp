#pragma once


#include <memory>

#include "Synapse/Layer.hpp"
#include "Synapse/LayerStack.hpp"
#include "Synapse/ImGui/ImGuiLayer.hpp"
#include "Synapse/Event/Event.hpp"
#include "Synapse/API/Window.hpp"
#include "Synapse/Utils/Timer/TimeStep.hpp"
#include "Synapse/Core.hpp"


namespace Syn {


	class Application
	{
	public:
		Application();
		virtual ~Application() = default;

		void run();
		void renderImGui();

		void onEvent(Event* _event);

		void pushLayer(Layer* _layer);
		void pushOverlay(Layer* _overlay);

		inline Window& getWindow() const { return *m_window; }
		static inline Application& get() { return *s_instance; }

	private:
		Layerstack m_layerStack;
		ImGuiLayer* m_imGuiLayer;
		Scope<Window> m_window = nullptr;
		bool m_bRunning = true;
		
	private:
		static Application* s_instance;

	};


}

