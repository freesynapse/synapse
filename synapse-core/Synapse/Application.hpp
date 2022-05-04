
#pragma once


#include <memory>

#include "Synapse/Layer.hpp"
#include "Synapse/LayerStack.hpp"
#include "Synapse/ImGui/ImGuiLayer.hpp"
#include "Synapse/Event/Event.hpp"
#include "Synapse/API/Window.hpp"
#include "Synapse/Utils/Timer/TimeStep.hpp"
#include "Synapse/Core.hpp"
#include "Synapse/Memory.hpp"


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
		inline float getFrameTime() { return (*s_instance).m_frameTime; }
		inline float getRenderTime() { return (*s_instance).m_renderTime; }

	private:
		Layerstack m_layerStack;
		ImGuiLayer* m_imGuiLayer;
		Ref<Window> m_window = nullptr;
		bool m_bRunning = true;
		float m_frameTime = 0.0f;
		float m_renderTime = 0.0f;
		bool m_firstFrame = true;
		
	private:
		static Application* s_instance;

	};


}

