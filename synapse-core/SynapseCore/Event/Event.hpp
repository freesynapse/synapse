#pragma once


#include "../../pch.hpp"

#include "../Memory.hpp"


namespace Syn {	


    //---------------------------------------------------------------------------------------
    enum class EventType
    {
	NONE = 0,
	APPLICATION_EXIT,
	WINDOW_CLOSE, WINDOW_RESIZE, WINDOW_TOGGLE_FULLSCREEN, WINDOW_TOGGLE_CURSOR, WINDOW_TOGGLE_FROZEN_CURSOR,
	INPUT_KEY, INPUT_MOUSE_BUTTON, INPUT_MOUSE_SCROLL,
	SHADER_RELOAD, VIEWPORT_RESIZE,
	//FILE_IO_ERROR,
	//RAISE_FATAL_ERROR, RAISE_ERROR, RAISE_WARNING,		// Error.h
	//EVENT_HANDLER_QUEUE_FULL,
	CUSTOM_EVENT,
    };

    inline static const char* strEventType(EventType e)
    {
	switch (e)
	{
	case EventType::NONE: return "EventType::NONE";
	case EventType::APPLICATION_EXIT: return "EventType::APPLICATION_EXIT";
	case EventType::WINDOW_CLOSE: return "EventType::WINDOW_CLOSE";
	case EventType::WINDOW_RESIZE: return "EventType::WINDOW_RESIZE";
	case EventType::WINDOW_TOGGLE_FULLSCREEN: return "EventType::WINDOW_TOGGLE_FULLSCREEN";
	case EventType::WINDOW_TOGGLE_CURSOR: return "EventType::WINDOW_TOGGLE_CURSOR";
	case EventType::WINDOW_TOGGLE_FROZEN_CURSOR: return "EventType::WINDOW_TOGGLE_FROZEN_CURSOR";
	case EventType::INPUT_KEY: return "EventType::INPUT_KEY";
	case EventType::INPUT_MOUSE_BUTTON: return "EventType::INPUT_MOUSE_BUTTON";
	case EventType::INPUT_MOUSE_SCROLL: return "EventType::INPUT_MOUSE_SCROLL";
	case EventType::SHADER_RELOAD: return "EventType::SHADER_RELOAD";
	case EventType::VIEWPORT_RESIZE: return "EventType::VIEWPORT_RESIZE";
	    //case EventType::FILE_IO_ERROR:				return "EventType::FILE_IO_ERROR";
	    //case EventType::RAISE_FATAL_ERROR:			return "EventType::RAISE_FATAL_ERROR";
	    //case EventType::RAISE_ERROR:					return "EventType::RAISE_ERROR";
	    //case EventType::RAISE_WARNING:				return "EventType::RAISE_WARNING";
	    //case EventType::EVENT_HANDLER_QUEUE_FULL:		return "EventType::EVENT_HANDLER_QUEUE_FULL";
	case EventType::CUSTOM_EVENT: return "EventType::CUSTOM_EVENT";
	default: return "Unknown EventType.";
	}

    }


    // macro for ostream << operator overloading
#define EVENT_TYPE_OSTREAM(_type) #_type

    // macro for class definition of standard functions
#ifdef _WIN64
#define EVENT_TYPE(_type) static EventType getStaticType() { return EventType::##_type; } \
    virtual EventType getEventType() const override { return getStaticType(); } \
    virtual const char* getName() const override { return #_type; }
#else
#define EVENT_TYPE(_type) static EventType getStaticType() { return EventType::_type; } \
    virtual EventType getEventType() const override { return getStaticType(); } \
    virtual const char* getName() const override { return #_type; }
#endif

    class Event
    {
    public:
	virtual ~Event() {};
	inline virtual EventType getEventType() const = 0;
	inline virtual const char* getName() const = 0;
	inline virtual std::string toString() const { return getName(); }
	bool m_handled = false;
	inline virtual const bool& isHandled() const { return m_handled; }
    };


    /*
      class HandlerQueueFullEvent : public Event
      {
      public:
      HandlerQueueFullEvent() {}

      EVENT_TYPE(EVENT_HANDLER_QUEUE_FULL)
      };


      class FileIOErrorEvent : public Event
      {
      public:
      FileIOErrorEvent(const std::string& _filename) :
      m_filename(_filename)
      {}

      EVENT_TYPE(FILE_IO_ERROR)

      private:
      std::string m_filename;
      };
    */


    inline std::ostream& operator<<(std::ostream& _os, const Event& _e)
    {
	return _os << _e.toString();
    }

    inline std::ostream& operator<<(std::ostream& _os, const EventType& _event_type)
    {
	return _os << EVENT_TYPE_OSTREAM(_event_type);
    }



}

