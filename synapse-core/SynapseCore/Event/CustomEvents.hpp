
#pragma once

#include "Event.hpp"


namespace Syn
{
    /*
     * Extendable custom event class. 
     *
     */
    class CustomEvent : public Event
    {
    public:
        CustomEvent() = default;

        EVENT_TYPE(CUSTOM_EVENT)

        const std::string& getID() { return m_ID; }
    
    protected:
        std::string m_ID;
    };


}