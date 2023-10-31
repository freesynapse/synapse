#pragma once


#include "../pch.hpp"

#include "./Event/Event.hpp"


namespace Syn {	


    class Layer
    {
    public:
		Layer(const std::string& _name = "defaultLayer") : m_name(_name) {}
		virtual ~Layer() = default;

		virtual void onAttach() {}
		virtual void onDetach() {}

		virtual void onUpdate(float _dt) {}
		virtual void onEvent(Event* _event) {}
		virtual void onImGuiRender() {}

    protected:
		std::string m_name;
    };


}
