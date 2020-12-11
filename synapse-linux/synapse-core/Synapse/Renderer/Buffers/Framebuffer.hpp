#pragma once


#include "Synapse/Core.hpp"
#include "Synapse/Event/EventHandler.hpp"
#include "Synapse/Event/EventTypes.hpp"
#include "Synapse/Renderer/Material/Texture.hpp"


namespace Syn {

	class FramebufferBase
	{
	public:
		FramebufferBase() {};
		virtual ~FramebufferBase();

		virtual void bind(bool _set_viewport=true) const;
		virtual inline void bindDefaultFramebuffer() { unbind(); }
		virtual void unbind() const;

		virtual void saveAsPNG(const std::string& _filename="");

		virtual void resize(uint32_t _width, uint32_t _height);
		virtual void onResizeEvent(Event* _e);

		virtual void bindTexture(uint32_t _tex_slot=0) const;

		virtual void clear(const glm::vec4& _clear_color, 
						   uint32_t _buffer_mask=GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT) const;

		// accessors
		GLuint getFramebufferID() { return m_framebufferID; }
		GLuint getColorAttachmentID() { return m_colorAttachmentID; }
		GLuint getDepthAttachmentID() { return m_depthAttachmentID; }

		uint32_t getWidth() { return m_width; }
		uint32_t getHeight() { return m_height; }
		const ColorFormat& getFormat() { return m_format; }
		const std::string& getName() { return m_name; }

	protected:
		GLuint m_framebufferID = 0;
		GLuint m_colorAttachmentID = 0;
		GLuint m_depthAttachmentID = 0;

		uint32_t m_width = 0;
		uint32_t m_height = 0;

		ColorFormat m_format = ColorFormat::NONE;
		OpenGLPixelFormat m_pxFmt;

		std::string m_name = "";
	};


	//-----------------------------------------------------------------------------------
	/* Main instantiation of base class. */

	class Framebuffer : public FramebufferBase
	{
	public:
		Framebuffer(uint32_t _width, 
					uint32_t _height, 
					const ColorFormat& _format=ColorFormat::RGBA8, 
					bool _update_on_resize=true,
					const std::string& _name="")
		{
			m_format = _format;
			m_pxFmt = getOpenGLPixelFormat(m_format);

			resize(_width, _height);

			// register function for handling resize events
			if (_update_on_resize)
				EventHandler::register_callback(EventType::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(Framebuffer::onResizeEvent));

			m_name = (_name.compare("") != 0) ? _name : "unknown";

		}

	};


}
