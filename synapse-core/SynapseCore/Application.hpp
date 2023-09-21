
#pragma once


#include <memory>

#include "Core.hpp"
#include "Memory.hpp"
#include "Layer.hpp"
#include "LayerStack.hpp"
#include "./ImGui/ImGuiLayer.hpp"
#include "./Event/Event.hpp"
#include "./API/Window.hpp"
#include "./Utils/Timer/TimeStep.hpp"


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
		inline void setMaxFPS(float _fps) { m_maxFPS = _fps; }

		inline void disableQuitOnEscape() { m_quitOnEscape = false; }
		inline const bool ImGuiEnabled() { return m_useImGui; }

    protected:
		bool m_quitOnEscape = true;
		bool m_useImGui = true;

    private:
		Layerstack m_layerStack;
		ImGuiLayer* m_imGuiLayer;
		Ref<Window> m_window = nullptr;
		bool m_bRunning = true;
		float m_frameTime = 0.0f;
		float m_maxFPS = -1.0f;
		float m_renderTime = 0.0f;
		bool m_firstFrame = true;

    private:
		static Application* s_instance;

    };


}

