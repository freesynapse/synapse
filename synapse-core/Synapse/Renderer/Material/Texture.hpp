
#pragma once

#include <stdint.h>

#include "Synapse/Core.hpp"


namespace Syn {

	enum class ColorFormat
	{
		NONE	=  0,
		R8		=  1,
		RG8		=  2,
		RGB8 	=  3,
		RGBA8	=  4,
		R16F	=  5,
		RG16F	=  6,
		RGB16F	=  7,
		RGBA16F =  8,
		R32F	=  9,
		RG32F	= 10,
		RGB32F	= 11,
		RGBA32F	= 12
	};


	//
	typedef struct opengl_pixel_format_
	{
		GLint internalFormat;
		GLenum storageFormat;
		GLenum storageType;

		opengl_pixel_format_() {}
		opengl_pixel_format_(GLint _i, GLenum _s, GLenum _t) :
			internalFormat(_i), storageFormat(_s), storageType(_t) 
		{}

	} OpenGLPixelFormat;


	static inline OpenGLPixelFormat getOpenGLPixelFormat(const ColorFormat& _fmt)
	{
		OpenGLPixelFormat fmt;

		switch (_fmt)
		{
			case ColorFormat::NONE:		fmt = OpenGLPixelFormat(GL_NONE, 	GL_NONE, GL_NONE); 			break;
			case ColorFormat::R8:		fmt = OpenGLPixelFormat(GL_R8,    	GL_RED,  GL_UNSIGNED_BYTE);	break;
			case ColorFormat::RG8:		fmt = OpenGLPixelFormat(GL_RG8,   	GL_RG,   GL_UNSIGNED_BYTE);	break;
			case ColorFormat::RGB8:		fmt = OpenGLPixelFormat(GL_RGB8,  	GL_RGB,  GL_UNSIGNED_BYTE);	break;
			case ColorFormat::RGBA8:	fmt = OpenGLPixelFormat(GL_RGBA8, 	GL_RGBA, GL_UNSIGNED_BYTE);	break;
			case ColorFormat::R16F:		fmt = OpenGLPixelFormat(GL_R16F,    GL_RED,  GL_HALF_FLOAT);	break;
			case ColorFormat::RG16F:	fmt = OpenGLPixelFormat(GL_RG16F,   GL_RG,   GL_HALF_FLOAT);	break;
			case ColorFormat::RGB16F:	fmt = OpenGLPixelFormat(GL_RGB16F,  GL_RGB,  GL_HALF_FLOAT);	break;
			case ColorFormat::RGBA16F:	fmt = OpenGLPixelFormat(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);	break;
			case ColorFormat::R32F:		fmt = OpenGLPixelFormat(GL_R32F,    GL_RED,  GL_FLOAT);			break;
			case ColorFormat::RG32F:	fmt = OpenGLPixelFormat(GL_RG32F,   GL_RG,   GL_FLOAT);			break;
			case ColorFormat::RGB32F:	fmt = OpenGLPixelFormat(GL_RGB32F,  GL_RGB,  GL_FLOAT);			break;
			case ColorFormat::RGBA32F:	fmt = OpenGLPixelFormat(GL_RGBA32F, GL_RGBA, GL_FLOAT);			break;
		}

		return fmt;
	}

	//
	static inline uint32_t getPixelFmtChannels(const ColorFormat& _fmt)
	{
		switch (_fmt)
		{
			case ColorFormat::NONE:		return 0;
			case ColorFormat::R8:
			case ColorFormat::R16F:
			case ColorFormat::R32F:		return 1;
			case ColorFormat::RG8:
			case ColorFormat::RG16F:		
			case ColorFormat::RG32F:	return 2;
			case ColorFormat::RGB8:
			case ColorFormat::RGB16F:	
			case ColorFormat::RGB32F:	return 3;
			case ColorFormat::RGBA8:
			case ColorFormat::RGBA16F:		
			case ColorFormat::RGBA32F:	return 4;
		}
	}


	//
	static inline size_t getPixelFmtTypeSize(const ColorFormat& _fmt)
	{
		switch (_fmt)
		{
			case ColorFormat::NONE:		return 0;
			case ColorFormat::R8:			
			case ColorFormat::RG8:		
			case ColorFormat::RGB8:		
			case ColorFormat::RGBA8:	return sizeof(GLubyte);
			case ColorFormat::R16F:
			case ColorFormat::RG16F:
			case ColorFormat::RGB16F:
			case ColorFormat::RGBA16F:
			case ColorFormat::R32F:
			case ColorFormat::RG32F:
			case ColorFormat::RGB32F:
			case ColorFormat::RGBA32F:	return sizeof(GLfloat);
		}
	}

	//
	class Texture
	{
	public:
		~Texture() = default;

		virtual void bind(uint32_t _tex_slot=0) = 0;

		virtual inline uint32_t getWidth() const { return m_width; }
		virtual inline uint32_t getHeight() const { return m_height; }
		virtual inline uint32_t getID() const { return m_textureID; }

	protected:
		uint32_t m_textureID = 0;
		uint32_t m_width = 0;
		uint32_t m_height = 0;
	};


}

