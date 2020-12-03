#pragma once


#include <stdint.h>

#include "src/Core.h"
#include "src/external/stb_image/stb_image_write.h"


namespace Syn {

	enum class TextureFormat
	{
		NONE	= 0,
		R8		= 1,
		RG8		= 2,
		RGB8 	= 3,
		RGBA8	= 4,
		R32F	= 5,
		RG32F	= 6,
		RGB32F	= 7,
		RGBA32F	= 8
	};


	//
	typedef struct texture_pixel_format_
	{
		GLint internalFormat;
		GLenum storageFormat;
		GLenum storageType;

		texture_pixel_format_() {}
		texture_pixel_format_(GLint _i, GLenum _s, GLenum _t) :
			internalFormat(_i), storageFormat(_s), storageType(_t) 
		{}

	} TexturePixelFormat;


	//
	static inline uint32_t getTextureFmtChannels(const TextureFormat& _fmt)
	{
		switch (_fmt)
		{
			case TextureFormat::NONE:		return 0;
			case TextureFormat::R8:			
			case TextureFormat::R32F:		return 1;
			case TextureFormat::RG8:		
			case TextureFormat::RG32F:		return 2;
			case TextureFormat::RGB8:		
			case TextureFormat::RGB32F:		return 3;
			case TextureFormat::RGBA8:		
			case TextureFormat::RGBA32F:	return 4;
		}
	}


	//
	static inline size_t getTextureFmtTypeSize(const TextureFormat& _fmt)
	{
		switch (_fmt)
		{
			case TextureFormat::NONE:		return 0;
			case TextureFormat::R8:			
			case TextureFormat::RG8:		
			case TextureFormat::RGB8:		
			case TextureFormat::RGBA8:		return sizeof(GLubyte);
			case TextureFormat::R32F:
			case TextureFormat::RG32F:
			case TextureFormat::RGB32F:
			case TextureFormat::RGBA32F:	return sizeof(GLfloat);
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

