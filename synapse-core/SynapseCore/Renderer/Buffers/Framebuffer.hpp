
#pragma once

#include "../../Core.hpp"
#include "../../Memory.hpp"
#include "../../Event/EventHandler.hpp"
#include "../../Event/EventTypes.hpp"
#include "../Material/Texture.hpp"


namespace Syn
{

	class FramebufferBase
	{
	public:
		FramebufferBase() {};
		virtual ~FramebufferBase();

		/* Binds the Framebuffer as the current GL_FRAMEBUFFER. */
		virtual void bind(bool _set_viewport=true) const;
		/* Unbinds, through binding GL_FRAMEBUFFER to 0. */
		virtual void unbind() const;
		/* Unbinds, through binding GL_FRAMEBUFFER to 0. */
		virtual inline void bindDefaultFramebuffer() { unbind(); }

		virtual void saveAsPNG(const std::string& _file_path="");

	protected:
		/* Called on Syn::ViewportResizeEvent and also upon instantiation of this
		 * Framebuffer (called from constructor). The _depth flag controls if creation
		 * of a depth and stencil buffer should be omitted. The private variable
		 * m_hasDepthAttachment is used. The size, if not specified (ie glm::ivec2(0)),
		 * will default to Syn::Renderer::getViewport().
		 * 
		 * Update: Depth buffer flag set at (m_hasDepthBuffer) contruction and used in
		 * resize() to control the creation of a depth buffer.
		 * Update: Removed this function as initializer of the class, created separate
		 * init() private function for this.
		 */
		virtual void resize(const glm::ivec2& _size);
		
		/* Initialization of class. See resize() for more information. */
		virtual void init(const glm::ivec2& _size);

		/* Callback function for resize events (Syn::ViewportResizeEvent). */
		virtual void onResizeEvent(Event* _e);

	public:
		/* Binds specified COLOR_ATTACHMENT_N (_color_attachment_slot) to the specified
		 * slot GL_TEXTURE0+_tex_slot as a GL_TEXTURE_2D.
		 */
		virtual void bindTexture(uint32_t _tex_slot=0, GLuint _color_attachment_slot=0) const;
		/* Same as above, but with the added option of changing the interpolation
		 * parameters.
		 */
		virtual void bindTexture(uint32_t _tex_slot, GLuint _color_attachment_slot, GLint _interpolation) const;

		/* Clears buffer using Syn::Renderer::getClearColor(). */
		virtual void clear(uint32_t _buffer_mask=GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT) const;
		/* Clears with specified rgba _clear_color. */
		virtual void clear(const glm::vec4& _clear_color, 
						   uint32_t _buffer_mask=GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT) const;

		/* Accessors */
		GLuint  getFramebufferID() 				 { 	return m_framebufferID; 		}
		GLuint* getColorAttachmentIDs() 		 { 	return m_colorAttachmentID; 	}
		GLuint  getColorAttachmentIDn(size_t _n) { 	return m_colorAttachmentID[_n]; }
		GLuint  getDepthAttachmentID() 			 { 	return m_depthAttachmentID; 	}

		const glm::ivec2& 	getSize() 	{ return m_size; 	}
		uint32_t 			getWidth() 	{ return m_size.x; 	}
		uint32_t 			getHeight() { return m_size.y; 	}
		const ColorFormat& 	getFormat() { return m_format; 	}
		const std::string& 	getName() 	{ return m_name; 	}

	protected:
		GLuint 	m_framebufferID 		= 0;			// ID of Framebuffer
		GLuint* m_colorAttachmentID 	= nullptr;		// Multiple attachments allowed, ie. when 
														// multiple draw targets are needed.
		size_t  m_colorAttachmentCount 	= 1;			// Number of color attachments.
		GLuint  m_depthAttachmentID 	= 0;			// ID of depth buffer.
		bool	m_hasDepthAttachment	= true;			// Flag used in resize() to determine depth
														// buffer creation.
		GLuint m_colorChannel = GL_COLOR_ATTACHMENT0;

		glm::ivec2 m_size = glm::ivec2(-1);				// Size (in px), if not set through during
														// construction defaults to Syn::Renderer
														// viewport size.

		ColorFormat m_format = ColorFormat::NONE;		// Pixel format and precision (eg RGBA16F).
		OpenGLPixelFormat m_pxFmt;						// OpenGL internal px format.

		std::string m_name = "";						// String ID, used for debug.
	};


	//-----------------------------------------------------------------------------------
	/* Main instantiation of base class. */
	class Framebuffer : public FramebufferBase
	{
	public:
		Framebuffer(const ColorFormat& _format=ColorFormat::RGBA16F, 
					const glm::ivec2& _size=glm::ivec2(0),
					size_t _n_drawbuffers=1,
					bool _use_depthbuffer=true,
					bool _update_on_resize=true,
					const std::string& _name="")
		{
			m_format = _format;
			m_pxFmt = getOpenGLPixelFormat(m_format);

			m_colorAttachmentCount = _n_drawbuffers;
			m_colorAttachmentID = new GLuint[m_colorAttachmentCount];
			m_hasDepthAttachment = _use_depthbuffer;

			m_name = (_name.compare("") != 0) ? _name : "unknown";

			// create the framebuffer
			init(_size);

			// register function for handling resize events
			if (_update_on_resize)
				EventHandler::register_callback(EventType::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(Framebuffer::onResizeEvent));

		}

	};


}
