
#include "Synapse/Renderer/Renderer.hpp"
#include "FluidFramebuffer.hpp"


FluidFramebuffer::FluidFramebuffer(uint32_t _width, 
								   uint32_t _height, 
								   const ColorFormat& _format, 
								   bool _update_on_resize,
								   const std::string& _name)
{
	m_format = _format;
	m_pxFmt = getOpenGLPixelFormat(m_format);

	resize(_width, _height);

	// register function for handling resize events
	if (_update_on_resize)
		EventHandler::register_callback(EventType::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(FluidFramebuffer::onResizeEvent));

	m_name = (_name.compare("") != 0) ? _name : "unknown";

}

void FluidFramebuffer::resize(uint32_t _width, uint32_t _height)
{
	if (_width == m_width && _height == m_height)
		return;

	m_width = _width;
	m_height = _height;

	SYN_RENDER_S0({
		if (self->m_framebufferID)
		{
			glDeleteFramebuffers(1, &self->m_framebufferID);
			glDeleteTextures(1, &self->m_colorAttachmentID);
			glDeleteTextures(1, &self->m_depthAttachmentID);
		}

		glGenFramebuffers(1, &self->m_framebufferID);

		glBindFramebuffer(GL_FRAMEBUFFER, self->m_framebufferID);

		// generate texture as rendering target
		glGenTextures(1, &self->m_colorAttachmentID);
		glBindTexture(GL_TEXTURE_2D, self->m_colorAttachmentID);

		// set texture with correct formats and type based on the FramebufferFormat specification.
		glTexImage2D(GL_TEXTURE_2D, 
						0, 
						self->m_pxFmt.internalFormat, 
						self->m_width, 
						self->m_height, 
						0, 
						self->m_pxFmt.storageFormat, 
						self->m_pxFmt.storageType, 
						NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// attach the texture to the framebuffer; the 'color' target
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self->m_colorAttachmentID, 0);
		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);

		// also need depth and stencil targets, so create a render buffer
		glGenTextures(1, &self->m_depthAttachmentID);
		glBindTexture(GL_TEXTURE_2D, self->m_depthAttachmentID);
		// dimensioning of the depth buffer
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, self->m_width, self->m_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		// attach to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, self->m_depthAttachmentID, 0);

		// check for completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			SYN_CORE_FATAL_ERROR("Framebuffer not complete.");
		}

		#ifdef DEBUG_FRAMEBUFFER
			SYN_CORE_TRACE("Framebuffer '", self->m_name, "' [ ", self->m_width, "x", self->m_height, " ] created/resized.");
		#endif

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	});
}





