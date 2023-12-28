#pragma once


#include "Event.hpp"
#include "../Core.hpp"
#include "../Renderer/Shader/Shader.hpp"


namespace Syn {


	class ViewportResizeEvent : public Event
	{
	public:
		ViewportResizeEvent(const glm::ivec2& _viewport) :
			m_viewport(_viewport), m_viewportF(glm::vec2(_viewport.x, _viewport.y))
		{}

		inline const glm::ivec2 &getViewport() { return m_viewport; }
		inline const glm::vec2 &getViewportF() { return m_viewportF; }
		inline uint32_t getViewportX() { return m_viewport.x; }
		inline uint32_t getViewportY() { return m_viewport.y; }
		inline float getViewportXF() { return m_viewportF.x; }
		inline float getViewportYF() { return m_viewportF.y; }

		EVENT_TYPE(VIEWPORT_RESIZE);

	private:
		glm::ivec2 m_viewport;
		glm::vec2 m_viewportF;
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


