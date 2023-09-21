#pragma once

#include "Event.hpp"


namespace Syn
{

    class ApplicationExitEvent : public Event
    {
    public:
	ApplicationExitEvent() {}
		
	EVENT_TYPE(APPLICATION_EXIT);
    };

}
