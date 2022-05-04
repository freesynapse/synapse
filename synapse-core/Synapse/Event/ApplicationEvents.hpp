#pragma once

#include "Synapse/Event/Event.hpp"


namespace Syn
{

	class ApplicationExitEvent : public Event
	{
	public:
		ApplicationExitEvent() {}
		
		EVENT_TYPE(APPLICATION_EXIT);
	};

}