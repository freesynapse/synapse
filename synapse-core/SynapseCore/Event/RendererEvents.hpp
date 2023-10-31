#pragma once


#include "Event.hpp"
#include "../Core.hpp"
#include "../Renderer/Shader/Shader.hpp"


namespace Syn {


	class ViewportResizeEvent : public Event
	{
	public:
		ViewportResizeEvent(const glm::ivec2& _viewport) :
			m_viewport(_viewport)
		{}

		inline const glm::ivec2& getViewport() { return m_viewport; }
		inline uint32_t getViewportX() { return m_viewport.x; }
		inline uint32_t getViewportY() { return m_viewport.y; }


		EVENT_TYPE(VIEWPORT_RESIZE);

	private:
		glm::ivec2 m_viewport;
	};


	//
	class ShaderReloadEvent : public Event
	{
	public:
		ShaderReloadEvent(Ref<Shader> _shader_ptr) :
			m_shader(_shader_ptr)
		{}

		inline const Ref<Shader> getShader() const { return m_shader; }


		EVENT_TYPE(SHADER_RELOAD);

	private:
		Ref<Shader> m_shader = nullptr;
	};


}


