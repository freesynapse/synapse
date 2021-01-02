#pragma once


#include "pch.hpp"

#include <GLFW/glfw3.h>

#include "Synapse/Event/WindowEvents.hpp"


namespace Syn { 

	class Window
	{
	public:
		Window(const char* _name, const int& _width, const int& _height, bool _fixed_size=false);
		~Window();

		void onUpdate();
		void onEvent(Event* _event);
		void centerCursor();

		static void resize_callback(GLFWwindow* _window, int _width, int _height);

		// accessors
		inline const int& getWidth() { return m_width; }
		inline const int& getHeight() { return m_height; }
		inline void* getNativeWindow() const { return m_window; }
		inline GLFWwindow* getWindowPtr() { return m_window; }
		inline bool isCursorFrozen() { return m_frozenCursor; }
		inline bool isCursorVisible() { return m_visibleCursor; }
		inline bool isVSYNCenabled() { return m_vsync; }
		bool toClose() const { return (glfwWindowShouldClose(m_window) || m_toCloseWindow); }
		void setVSYNC(bool _b) { glfwSwapInterval((int)_b); m_vsync = _b; }
		inline void setCursorPos(const glm::vec2& _pos) { glfwSetCursorPos(m_window, _pos.x, _pos.y); }

	private:
		int init(bool _fixed_size);
		void setFullscreen(const bool& _fullscreen);


	private:
		int m_width = 0;
		int m_height = 0;
		const char* m_title = "SYNAPSE";

		GLFWwindow* m_window = nullptr;
		GLFWmonitor* m_primaryMonitor = nullptr;
		const GLFWvidmode* m_vidMode;
		
		// flags
		bool m_isClosed = false;
		bool m_toCloseWindow = false;
		bool m_isFullscreen = false;

		bool m_frozenCursor = false;
		bool m_visibleCursor = true;
		bool m_vsync = true;

	};

}



