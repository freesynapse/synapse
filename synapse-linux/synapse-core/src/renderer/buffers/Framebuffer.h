#pragma once


#include "src/event/EventTypes.h"
#include "src/renderer/material/Texture.h"


namespace Syn {


	class Framebuffer
	{
	public:
		Framebuffer(uint32_t _width, uint32_t _height, ColorFormat _format=ColorFormat::RGBA8, bool _update_on_resize=true);
		~Framebuffer();

		void bind() const;
		inline void bindDefaultFramebuffer() { unbind(); }
		void unbind() const;

		void saveAsPNG(const std::string& _filename="");

		void resize(uint32_t _width, uint32_t _height);
		void onResizeEvent(Event* _e);

		void bindTexture(uint32_t _tex_slot=0) const;

		void clearFramebuffer(const glm::vec4& _clear_color, uint32_t _buffer_mask=GL_COLOR_BUFFER_BIT) const;


		// accessors
		GLuint getFramebufferID() { return m_framebufferID; }
		GLuint getColorAttachmentID() { return m_colorAttachmentID; }
		GLuint getDepthAttachmentID() { return m_depthAttachmentID; }

		uint32_t getWidth() { return m_width; }
		uint32_t getHeight() { return m_height; }
		ColorFormat getFormat() { return m_format; }


	private:
		GLuint m_framebufferID = 0;
		GLuint m_colorAttachmentID = 0;
		GLuint m_depthAttachmentID = 0;

		uint32_t m_width = 0;
		uint32_t m_height = 0;

		ColorFormat m_format = ColorFormat::NONE;
		OpenGLPixelFormat m_pxFmt;

	};


}
