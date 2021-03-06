#pragma once


#include "pch.hpp"

#include "Synapse/Event/Event.hpp"


namespace Syn { 


	//-----------------------------------------------------------------------------------
	class EventHandler
	{
		// Methods declarations .........................................................
	public:
		static void init();

		/* Release all event pointers. This is the job of the EventHandler, even after dispatch to
		registered handlers. */
		static void release();

		/* Put a new event on the queue. */
		static int push_event(Event *_event);

		/* Process all events in the queue. */
		static void process_events();

		// Registration of function pointers that will be alerted by the EventHandler
		// when an event of a certain EventType is dispatched.
		// For now, this will probably only encompass a void function in SynapseCore.
		static void register_callback(EventType _event_type, std::function<void(Event*)> _handler_fnc);

		// DEBUG
		static int queue_length();

	private:
		/* Get next event from the queue. */
		static Event* next_event();



		// Member variables .............................................................
	private:
		static const unsigned char MAX_EVENTS = 32;
		static const unsigned short MAX_CALLBACKS = 256;

		static unsigned char m_queueHead;
		static unsigned char m_queueTail;
		static Event *m_eventQueue[MAX_EVENTS];

		static unsigned short m_numCallbacks;

	};

	/*
	A multimap where the registered handlers are paired to a specific
	EventType (eg. ::TUTORIAL etc.). Upon event handling, process_events()
	will dispatch the event to the respective handlers for that EventType.
	*/
	extern std::multimap<EventType, std::function<void(Event *)> > g_mapHandlerFnc;
	typedef std::multimap<EventType, std::function<void(Event *)> >::iterator mapIterator;


}


