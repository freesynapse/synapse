
#include "../../pch.hpp"

#include "EventHandler.hpp"
#include "../Core.hpp"
#include "../Debug/Profiler.hpp"


namespace Syn { 


    // declaration of static member variables
    //
    unsigned char EventHandler::m_queueHead;
    unsigned char EventHandler::m_queueTail;
    Event *EventHandler::m_eventQueue[EventHandler::MAX_EVENTS];
    unsigned short EventHandler::m_numCallbacks;
    std::multimap<EventType, std::function<void(Event *)>> g_mapHandlerFnc;

    //-----------------------------------------------------------------------------------
    void EventHandler::init()
    {
	m_queueHead = 0;
	m_queueTail = 0;
	m_numCallbacks = 0;

	SYN_CORE_TRACE("listening.");
    }
    //-----------------------------------------------------------------------------------
    void EventHandler::shutdown()
    {
	SYN_CORE_TRACE("clearing event queue.");
	int numCleared = 0;
	for (size_t i = 0; i < EventHandler::MAX_EVENTS; i++)
	{
	    if (m_eventQueue[i])
	    {
		delete m_eventQueue[i];
		numCleared++;
	    }
	}
	//SYN_CORE_TRACE(numCleared, " event(s) deleted.");

    }
    //-----------------------------------------------------------------------------------
    int EventHandler::push_event(Event *_event)
    {
	assert((m_queueTail + 1) % MAX_EVENTS != m_queueHead);

	// release stored event (if any)
	if (m_eventQueue[m_queueTail])
	{
#ifdef DEBUG_EVENTS
	    SYN_CORE_TRACE("m_eventQueue full; deleting last event [", strEventType(m_eventQueue[m_queueTail]->getEventType()), "]");
#endif

	    delete m_eventQueue[m_queueTail];
	    m_eventQueue[m_queueTail] = nullptr;
	}

	// add event to the end of the list
	m_eventQueue[m_queueTail] = _event;
	m_queueTail = (m_queueTail + 1) % MAX_EVENTS;

#ifdef DEBUG_EVENTS
	SYN_CORE_TRACE(_event->getName());
#endif
		
	return RETURN_SUCCESS;
    }
    //-----------------------------------------------------------------------------------
    int EventHandler::queue_length()
    {
	return (m_queueTail % MAX_EVENTS) - (m_queueHead % MAX_EVENTS);
    }
    //-----------------------------------------------------------------------------------
    /* TODO: Eventually, different systems may register with the EventHandler to process events
     * of the EventType (Event.h) corresponding to task of the registered class. Each class that
     * registers for an event type have to provide a function pointer to the function used to
     * process events of a certain EventType.
     */
    //unsigned int ProcessEvent() {}
    Event *EventHandler::next_event()
    {
	if (m_queueHead == m_queueTail)
	    return nullptr;

	Event *ptr;
	if (m_eventQueue[m_queueHead])
	{
	    ptr = m_eventQueue[m_queueHead];
	    m_queueHead = (m_queueHead + 1) % MAX_EVENTS;
	}
	else
	    ptr = nullptr;

	return ptr;

    }
    //-----------------------------------------------------------------------------------
    /*
      Process events pushed by subsystems of SynapseCore.
    */
    void EventHandler::process_events()
    {
	SYN_PROFILE_FUNCTION();

	Event *e;
	while ((e = EventHandler::next_event()))
	{
	    // find all values corresponding to key EventType of the current Event.
	    std::pair<mapIterator, mapIterator> res = g_mapHandlerFnc.equal_range(e->getEventType());

	    //std::cout << "EventType: " << (int)e->eventType() << ": funcs:" << std::endl;
	    // step through result and dispatch to handler functions
	    for (mapIterator it = res.first; it != res.second; it++)
	    {
		// call the function with the current event
		it->second(e);
	    }

	}


    }
    //-----------------------------------------------------------------------------------
    void EventHandler::register_callback(EventType _event_type, std::function<void(Event *)> _handler_fnc)
    {
#ifdef DEBUG_EVENTS
	SYN_CORE_TRACE("new callback registered: ", strEventType(_event_type), ", 0x", *(long*)(char*)& _handler_fnc, ".");
#endif

	if (m_numCallbacks < MAX_CALLBACKS)
	{
	    // add the new function to the multimap under the correct key
	    g_mapHandlerFnc.insert(std::pair<EventType, std::function<void(Event *)> >(_event_type, _handler_fnc));

	    m_numCallbacks++;
	}

    }

}
