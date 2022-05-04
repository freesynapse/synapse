
#pragma once

#include "Synapse/Event/Event.hpp"


namespace Syn
{

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}
		
		EVENT_TYPE(WINDOW_CLOSE)
	};


	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int _width, unsigned int _height) :
			m_width(_width), m_height(_height)
		{
		}

		EVENT_TYPE(WINDOW_RESIZE)

	private:
		unsigned int m_width = 0;
		unsigned int m_height = 0;
	};


	class WindowToggleFullscreenEvent : public Event
	{
	public:
		WindowToggleFullscreenEvent() {}

		EVENT_TYPE(WINDOW_TOGGLE_FULLSCREEN)
	};


	class WindowToggleFrozenCursorEvent : public Event
	{
	public:
		WindowToggleFrozenCursorEvent(bool _frozen) :
			m_frozen(_frozen)
		{}

		inline const bool getState() const { return m_frozen; }

		EVENT_TYPE(WINDOW_TOGGLE_FROZEN_CURSOR)

	private:
		bool m_frozen = false;
	};


	class WindowToggleCursorEvent : public Event
	{
	public:
		WindowToggleCursorEvent(bool _visible) :
			m_visible(_visible) 
		{}

		inline const bool getState() const { return m_visible; }

		EVENT_TYPE(WINDOW_TOGGLE_CURSOR)

	private:
		bool m_visible = true;
	};

}

