#pragma once


#include "src/event/EventTypes.h"


namespace Syn {


	enum class FramebufferFormat
	{
		NONE	= 0,
		RGBA8	= 1,
		RGBA16F = 2,
	};


	//--------------------------------------------------------------------------------------
	class Framebuffer
	{
	public:
		Framebuffer(uint32_t _width, uint32_t _height, FramebufferFormat _format = FramebufferFormat::RGBA8);
		~Framebuffer();

		void bind() const;
		inline void bindDefaultFramebuffer() { unbind(); }
		void unbind() const;

		void saveAsPNG(const std::string& _filename="");

		void resize(uint32_t _width, uint32_t _height);
		void onResizeEvent(Event* _e);

		void bindTexture(uint32_t _tex_slot) const;



		// accessors
		GLuint getFramebufferID() { return m_framebufferID; }
		GLuint getColorAttachmentID() { return m_colorAttachmentID; }
		GLuint getDepthAttachmentID() { return m_depthAttachmentID; }

		uint32_t getWidth() { return m_width; }
		uint32_t getHeight() { return m_height; }
		FramebufferFormat getFormat() { return m_format; }


	private:
		GLuint m_framebufferID = 0;
		GLuint m_colorAttachmentID = 0;
		GLuint m_depthAttachmentID = 0;

		uint32_t m_width = 0;
		uint32_t m_height = 0;

		FramebufferFormat m_format = FramebufferFormat::NONE;

	};


}
