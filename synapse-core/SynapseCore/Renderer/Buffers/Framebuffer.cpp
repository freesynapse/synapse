
#include "../../../pch.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../../External/stb_image/stb_image_write.h"

#include "Framebuffer.hpp"
#include "../Renderer.hpp"

#include "../../Core.hpp"
#include "../../Utils/Noise/Noise.hpp"
#include "../../Utils/Timer/Time.hpp"
#include "../../Event/EventHandler.hpp"


namespace Syn
{

    FramebufferBase::~FramebufferBase()
    {
		SYN_RENDER_S0({
			glDeleteFramebuffers(1, &self->m_framebufferID);
	    });
		
		if (m_colorAttachmentID != nullptr)
			delete m_colorAttachmentID;

    }

    //-----------------------------------------------------------------------------------
    void FramebufferBase::bind(bool _set_viewport) const
    {
		SYN_RENDER_S1(_set_viewport, {
			if (_set_viewport)
				glViewport(0, 0, self->m_size.x, self->m_size.y);
			glBindFramebuffer(GL_FRAMEBUFFER, self->m_framebufferID);
		});
    
	}

    //-----------------------------------------------------------------------------------
    void FramebufferBase::unbind() const
    {
		SYN_RENDER_S0({
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
	    });

    }

    //-----------------------------------------------------------------------------------
    void FramebufferBase::saveAsPNG(const std::string& _file_path/* ="" */)
    {
		// size of texture in bytes, using 3 channels (rgb)
		uint32_t szImage = m_size.x * m_size.y * 3;
		unsigned char* pixels = new unsigned char[szImage];

		// format filename
		std::string fileName;
		if (strcmp(_file_path.c_str(), "") == 0)
		{
			std::string dirName = "./";
				
			std::string time = Time::current_time();
			std::replace(time.begin(), time.end(), ':', '.');
			fileName = dirName + Time::current_date() + '_' + time + '_' + Random::rand_str(24) + ".png";

		}
		else
			fileName = _file_path;

			
		SYN_RENDER_S2(pixels, fileName, {
			// first, bind this buffer
			glBindFramebuffer(GL_FRAMEBUFFER, self->m_framebufferID);

			// glReadPixels can align the first pixel in each row at 1-, 2-, 4- and 8-byte boundaries. We
			// have allocated the exact size needed for the image so we have to use 1-byte alignment
			// (otherwise glReadPixels would write out of bounds)
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glReadPixels(0, 0, self->m_size.x, self->m_size.y, GL_RGB, GL_UNSIGNED_BYTE, pixels);

			// unbind buffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// write image data
			stbi_flip_vertically_on_write(1);
			stbi_write_png(fileName.c_str(), self->m_size.x, self->m_size.y, 3, pixels, 0);

			// cleanup
			delete[] pixels;
		});

		SYN_CORE_TRACE("saved to '", fileName, "'.");

    }

    //-----------------------------------------------------------------------------------
    void FramebufferBase::init(const glm::ivec2& _size)
    {
		// set size
		if (_size == glm::ivec2(0))
			m_size = Renderer::getViewport();
		else
			m_size = _size;


		SYN_RENDER_S0({
			glGenFramebuffers(1, &self->m_framebufferID);
			glBindFramebuffer(GL_FRAMEBUFFER, self->m_framebufferID);

			// generate texture(s) as rendering target
			glGenTextures(self->m_colorAttachmentCount, self->m_colorAttachmentID);
			GLenum drawBufferIDs[self->m_colorAttachmentCount];

			// create textures for the GL_COLOR_ATTACHMENTs
			for (size_t i = 0; i < self->m_colorAttachmentCount; i++)
			{
				glBindTexture(GL_TEXTURE_2D, self->m_colorAttachmentID[i]);

				// set texture with correct formats and type based on the FramebufferFormat specification.
				glTexImage2D(GL_TEXTURE_2D, 
					0, 
					self->m_pxFmt.internalFormat, 
					self->m_size.x, 
					self->m_size.y, 
					0, 
					self->m_pxFmt.storageFormat, 
					self->m_pxFmt.storageType, 
					NULL);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				// attach the texture to the framebuffer
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, self->m_colorAttachmentID[i], 0);
				drawBufferIDs[i] = GL_COLOR_ATTACHMENT0 + i;
			}
			// set n number of draw targets (ie layout(location=n) in fragment shader).
			glDrawBuffers(self->m_colorAttachmentCount, drawBufferIDs);

			// also need depth and stencil targets, so create a render buffer
			if (self->m_hasDepthAttachment)
			{
				glGenTextures(1, &self->m_depthAttachmentID);
				glBindTexture(GL_TEXTURE_2D, self->m_depthAttachmentID);
				// dimensioning of the depth buffer
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, self->m_size.x, self->m_size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
				// attach to framebuffer
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, self->m_depthAttachmentID, 0);
			}

			// check for completeness
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				SYN_CORE_FATAL_ERROR("Framebuffer not complete.");
			}

			#ifdef DEBUG_FRAMEBUFFER
				SYN_CORE_TRACE("Framebuffer '", self->m_name.c_str(), "'; renderTargets=", self->m_colorAttachmentCount, " [ ", self->m_size.x, "x", self->m_size.y, " ] created.");
			#endif

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		});

    }	
	
    //-----------------------------------------------------------------------------------
    void FramebufferBase::resize(const glm::ivec2& _size)
    {
		if (_size.x == m_size.x && _size.y == m_size.y)
			return;
		else
			m_size = _size;

		SYN_RENDER_S0({
			if (self->m_framebufferID)
			{
				glDeleteFramebuffers(1, &self->m_framebufferID);
				glDeleteTextures(self->m_colorAttachmentCount, self->m_colorAttachmentID);
				glDeleteTextures(1, &self->m_depthAttachmentID);
			}

			glGenFramebuffers(1, &self->m_framebufferID);

			glBindFramebuffer(GL_FRAMEBUFFER, self->m_framebufferID);

			// generate texture(s) as rendering target
			glGenTextures(self->m_colorAttachmentCount, self->m_colorAttachmentID);
			GLenum drawBufferIDs[self->m_colorAttachmentCount];

			for (size_t i = 0; i < self->m_colorAttachmentCount; i++)
			{
				glBindTexture(GL_TEXTURE_2D, self->m_colorAttachmentID[i]);

				// set texture with correct formats and type based on the FramebufferFormat specification.
				glTexImage2D(GL_TEXTURE_2D, 
					0, 
					self->m_pxFmt.internalFormat, 
					self->m_size.x, 
					self->m_size.y, 
					0, 
					self->m_pxFmt.storageFormat, 
					self->m_pxFmt.storageType, 
					NULL);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				// attach the texture to the framebuffer
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, self->m_colorAttachmentID[i], 0);
				drawBufferIDs[i] = GL_COLOR_ATTACHMENT0 + i;
			}

			glDrawBuffers(self->m_colorAttachmentCount, drawBufferIDs);

			// also need depth and stencil targets, so create a render buffer
			if (self->m_hasDepthAttachment)
			{
				glGenTextures(1, &self->m_depthAttachmentID);
				glBindTexture(GL_TEXTURE_2D, self->m_depthAttachmentID);
				// dimensioning of the depth buffer
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, self->m_size.x, self->m_size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
				// attach to framebuffer
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, self->m_depthAttachmentID, 0);
			}

			// check for completeness
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				SYN_CORE_FATAL_ERROR("Framebuffer not complete.");
			}

			#ifdef DEBUG_FRAMEBUFFER
				SYN_CORE_TRACE("Framebuffer '", self->m_name, "' resized: [ ", self->m_size.x, "x", self->m_size.y, " ].");
			#endif

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		});

    }

    //-----------------------------------------------------------------------------------
    void FramebufferBase::onResizeEvent(Event* _e)
    {
		Syn::ViewportResizeEvent* e = dynamic_cast<Syn::ViewportResizeEvent*>(_e);
		this->resize(glm::ivec2(e->getViewportX(), e->getViewportY()));
    }

    //-----------------------------------------------------------------------------------
    void FramebufferBase::bindTexture(uint32_t _tex_slot, GLuint _color_attachment_slot) const
    {
		SYN_RENDER_S2(_tex_slot, _color_attachment_slot, {
			glActiveTexture(GL_TEXTURE0 + _tex_slot);
			glBindTexture(GL_TEXTURE_2D, self->m_colorAttachmentID[_color_attachment_slot]);
			//glBindTextureUnit(_tex_slot, self->m_colorAttachmentID[_color_attachment_slot]);
	    });
    }

    //-----------------------------------------------------------------------------------
    void FramebufferBase::clear(uint32_t _buffer_mask) const
    {
		glm::vec4 clearColor = Syn::Renderer::getClearColor();
		SYN_RENDER_S2(clearColor, _buffer_mask, {
			glBindFramebuffer(GL_FRAMEBUFFER, self->m_framebufferID);
			glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
			glClear(_buffer_mask);
	    });

    }

    //-----------------------------------------------------------------------------------
    void FramebufferBase::clear(const glm::vec4& _clear_color, uint32_t _buffer_mask) const
    {
		SYN_RENDER_S2(_clear_color, _buffer_mask, {
			// store current color
			auto &renderer = Renderer::get();
			glm::vec4 renderer_clear_color = renderer.getClearColor();

			glBindFramebuffer(GL_FRAMEBUFFER, self->m_framebufferID);
			glClearColor(_clear_color.r, _clear_color.g, _clear_color.b, _clear_color.a);
			glClear(_buffer_mask);

			renderer.setClearColor(renderer_clear_color);
	    });

		// restore clear color

    }

}
