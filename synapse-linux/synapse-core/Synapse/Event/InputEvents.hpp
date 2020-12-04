#pragma once


#include "Synapse/Event/Event.hpp"


namespace Syn {


	class KeyDownEvent : public Event
	{
	public:
		KeyDownEvent(int _key, int _action) :
			m_key(_key), m_action(_action)
		{}

		inline const int getKey() const { return m_key; }
		inline const int getAction() const { return m_action; }

		EVENT_TYPE(INPUT_KEY)

	private:
		int m_key = 0;
		int m_action = 0;
	};


	class MouseButtonEvent : public Event
	{
	public:
		MouseButtonEvent(int _button, int _action, int _mods) :
			m_button(_button), m_action(_action), m_mods(_mods)
		{}

		inline const int getButton() const { return m_button; }
		inline const int getAction() const { return m_action; }
		inline const int getMods() const { return m_mods; }

		EVENT_TYPE(INPUT_MOUSE_BUTTON);

	private:
		int m_button = 0;
		int m_action = 0;
		int m_mods = 0;
	};


	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(const float _offset_x, const float _offset_y) :
			m_XOffset(_offset_x), m_YOffset(_offset_y)
		{}

		inline const float getXOffset() const { return m_XOffset; }
		inline const float getYOffset() const { return m_YOffset; }

		EVENT_TYPE(INPUT_MOUSE_SCROLL);

	private:
		float m_XOffset;
		float m_YOffset;
	};


}