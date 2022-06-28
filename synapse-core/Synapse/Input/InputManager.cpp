
#include "pch.hpp"

#include "Synapse/Input/InputManager.hpp"

#include "Synapse/Core.hpp"
#include "Synapse/Debug/Log.hpp"
#include "Synapse/Event/EventHandler.hpp"
#include "Synapse/Event/WindowEvents.hpp"
#include "Synapse/Event/InputEvents.hpp"


namespace Syn
{

	// static declarations
	bool InputManager::m_bKeys[InputManager::MAX_KEYS];
	bool InputManager::m_bMouseButtons[InputManager::MAX_MOUSE_BUTTONS];
	glm::vec2 InputManager::m_mousePosition;
	glm::vec2 InputManager::m_mouseScrollPos;


	//-----------------------------------------------------------------------------------
	void InputManager::init()
	{
		// reset input maps
		memset(m_bKeys, 0, sizeof(bool) * MAX_KEYS);
		memset(m_bMouseButtons, 0, sizeof(bool) * MAX_MOUSE_BUTTONS);
		m_mousePosition = glm::vec2(0.0, 0.0);
		m_mouseScrollPos = glm::vec2(0.0, 0.0);

		SYN_CORE_TRACE("listening.");
	}


	//-----------------------------------------------------------------------------------
	void InputManager::process_input()
	{
		/*
		if (is_key_pressed(SYN_KEY_ESCAPE))
			EventHandler::push_event(new WindowCloseEvent());
		*/

		/*
		if (is_key_pressed(SYN_KEY_F))
			EventHandler::push_event(new WindowToggleFullscreenEvent());
		*/
	}

	
	//-----------------------------------------------------------------------------------
	bool InputManager::is_key_pressed(unsigned int _key)
	{
		if (_key >= MAX_KEYS)
			return false;
		return m_bKeys[_key];
	}


	//-----------------------------------------------------------------------------------
	bool InputManager::is_button_pressed(unsigned int _button)
	{
		if (_button >= MAX_MOUSE_BUTTONS)
			return false;
		return m_bMouseButtons[_button];
	}


	//-----------------------------------------------------------------------------------
	const glm::vec2& InputManager::get_mouse_position()
	{
		return m_mousePosition;
	}


	//-----------------------------------------------------------------------------------
	const glm::vec2& InputManager::get_mouse_scroll_position()
	{
		return m_mouseScrollPos;
	}


	//-----------------------------------------------------------------------------------
	void InputManager::key_callback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
	{
		m_bKeys[_key] = _action != GLFW_RELEASE;
		
		EventHandler::push_event(new KeyDownEvent(_key, _action));

		#ifdef DEBUG_KEYS_BUTTONS
			SYN_CORE_TRACE("key: ", _key, "  action: ", _action);
		#endif
	}
	

	//-----------------------------------------------------------------------------------
	void InputManager::mouse_button_callback(GLFWwindow* _window, int _button, int _action, int _mods)
	{
		m_bMouseButtons[_button] = _action != GLFW_RELEASE;

		EventHandler::push_event(new MouseButtonEvent(_button, _action, _mods));

		#ifdef DEBUG_KEYS_BUTTONS
			SYN_CORE_TRACE("button: ", _button, "  action: ", _action);
		#endif
	}


	//-----------------------------------------------------------------------------------
	void InputManager::mouse_scroll_callback(GLFWwindow* _window, double _offset_x, double _offset_y)
	{
		m_mouseScrollPos.x = (float)_offset_x;
		m_mouseScrollPos.y = (float)_offset_y;

		EventHandler::push_event(new MouseScrolledEvent(m_mouseScrollPos.x, m_mouseScrollPos.y));

		#ifdef DEBUG_KEYS_BUTTONS
			SYN_CORE_TRACE("scroll: ", _offset_x, ", ", _offset_y);
		#endif
	}
	
	
	//-----------------------------------------------------------------------------------
	void InputManager::cursor_position_callback(GLFWwindow* _window, double _x, double _y)
	{
		m_mousePosition.x = (float)_x;
		m_mousePosition.y = (float)_y;

		#ifdef DEBUG_CURSOR_MOVE
			SYN_CORE_TRACE(_x, ", ", _y);
		#endif

	}


}
