#pragma once


#include <memory>

#include "src/Layer.h"
#include "src/LayerStack.h"
#include "src/imGui/ImGuiLayer.h"
#include "src/event/Event.h"
#include "src/API/Window.h"
#include "src/utils/timer/TimeStep.h"
#include "src/Core.h"


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

