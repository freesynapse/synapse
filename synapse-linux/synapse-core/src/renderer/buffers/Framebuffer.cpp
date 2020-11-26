
#include "pch.hpp"

#include "Framebuffer.h"
#include "src/Core.h"
#include "src/renderer/Renderer.h"
#include "src/external/stb_image/stb_image_write.h"
#include "src/utils/timer/Time.h"
#include "src/utils/noise/Noise.h"
#include "src/event/EventHandler.h"


namespace Syn {

	
	Framebuffer::Framebuffer(uint32_t _width, uint32_t _height, FramebufferFormat _format) :
		m_width(_width), m_height(_height), m_format(_format)
	{
		resize(_width, _height);

		// register function for handling resize events
		EventHandler::register_callback(EventType::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(Framebuffer::onResizeEvent));
	}


	//-----------------------------------------------------------------------------------
	Framebuffer::~Framebuffer()
	{
		SYN_RENDER_S0({
			glDeleteFramebuffers(1, &self->m_framebufferID);
		});
	}


	//-----------------------------------------------------------------------------------
	void Framebuffer::bind() const
	{
		SYN_RENDER_S0({
			glBindFramebuffer(GL_FRAMEBUFFER, self->m_framebufferID);
			glViewport(0, 0, self->m_width, self->m_height);
		});
	}


	//-----------------------------------------------------------------------------------
	void Framebuffer::unbind() const
	{
		SYN_RENDER_S0({
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		});
	}


	//-----------------------------------------------------------------------------------
	void Framebuffer::saveAsPNG(const std::string& _filename)
	{
		// size of texture in bytes, using 3 channels (rgb)
		uint32_t szImage = m_width * m_height * 3;
		unsigned char* pixels = new unsigned char[szImage];

		// format filename
		// TODO: don't hardcode this path!!
		std::string dirName = "../screenshots/";
		
		std::string time = Time::current_time();
		std::replace(time.begin(), time.end(), ':', '.');
		std::string fileName = _filename == "" ? dirName + Time::current_date() + '_' + time + '_' + Random::get_random_string(24) + ".png" : _filename;
		//std::string saveFile = dirName + fileName;
		
		SYN_RENDER_S2(pixels, fileName, {
			// first, bind this buffer
			glBindFramebuffer(GL_FRAMEBUFFER, self->m_framebufferID);

			// glReadPixels can align the first pixel in each row at 1-, 2-, 4- and 8-byte boundaries. We
			// have allocated the exact size needed for the image so we have to use 1-byte alignment
			// (otherwise glReadPixels would write out of bounds)
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glReadPixels(0, 0, self->m_width, self->m_height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

			// unbind buffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// write image data
			stbi_flip_vertically_on_write(1);
			stbi_write_png(fileName.c_str(), self->m_width, self->m_height, 3, pixels, 0);

			// cleanup
			delete[] pixels;
		});

		SYN_CORE_TRACE("saved to '", fileName, "'.");

	}


	//-----------------------------------------------------------------------------------
	void Framebuffer::resize(uint32_t _width, uint32_t _height)
	{
		if (_width == m_width && _height == m_height)
			return;

		m_width = _width;
		m_height = _height;

		SYN_RENDER_S0({ ;
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

			if (self->m_format == FramebufferFormat::RGBA16F)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, self->m_width, self->m_height, 0, GL_RGB, GL_FLOAT, NULL);
			}
			else if (self->m_format == FramebufferFormat::RGBA8)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, self->m_width, self->m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// attach the texture to the framebuffer; the 'color' target
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self->m_colorAttachmentID, 0);

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
				SYN_CORE_FATAL_ERROR("framebuffer is not complete.");
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		});
	}


	//-----------------------------------------------------------------------------------
	void Framebuffer::onResizeEvent(Event* _e)
	{
		Syn::ViewportResizeEvent* e = dynamic_cast<Syn::ViewportResizeEvent*>(_e);
		this->resize(e->getViewportX(), e->getViewportY());
	}


	//-----------------------------------------------------------------------------------
	void Framebuffer::bindTexture(uint32_t _tex_slot) const
	{
		SYN_RENDER_S1(_tex_slot, {
			glActiveTexture(GL_TEXTURE0 + _tex_slot);
			glBindTexture(GL_TEXTURE_2D, self->m_colorAttachmentID);
		});
	}


}