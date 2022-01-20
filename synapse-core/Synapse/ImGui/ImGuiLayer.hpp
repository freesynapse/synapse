#pragma once


#include "Synapse/Layer.hpp"


/* Suppression of casting errors from GLuint to void *. */
#ifdef __clang__
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif



namespace Syn {


	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer(const std::string& _name="ImGui control layer") 
		{ 
			m_name = _name; 
		}
		virtual ~ImGuiLayer() override {}

		virtual void onAttach() override;
		virtual void onDetach() override;

		void begin();
		void end();

		virtual void onUpdate(float _dt) override;
		virtual void onEvent(Event* _event) override;
		virtual void onImGuiRender() override;
	};


}
