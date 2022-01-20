#pragma once


#include <GLFW/glfw3.h>

#include "Synapse/Input/KeyCodes.hpp"


namespace Syn { 


	class InputManager
	{
	public:
		static void init();
		static void process_input();

		static void key_callback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);
		static void mouse_button_callback(GLFWwindow* _window, int _button, int _action, int _mods);
		static void mouse_scroll_callback(GLFWwindow* _window, double _offset_x, double _offset_y);
		static void cursor_position_callback(GLFWwindow* _window, double _x, double _y);

		// accessors
		static bool is_key_pressed(unsigned int _key);
		static bool is_button_pressed(unsigned int _button);
		static const glm::vec2& get_mouse_position();
		static const glm::vec2& get_mouse_scroll_position();



	public:
		static constexpr int MAX_KEYS = 1024;
		static constexpr int MAX_MOUSE_BUTTONS = 32;
		
		static GLFWwindow* m_window;
		static bool m_bKeys[MAX_KEYS];
		static bool m_bMouseButtons[MAX_MOUSE_BUTTONS];
		static glm::vec2 m_mousePosition;
		static glm::vec2 m_mouseScrollPos;
	};


}
