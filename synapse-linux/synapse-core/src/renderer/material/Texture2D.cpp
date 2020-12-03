
#include "pch.hpp"

#include "src/renderer/material/Texture2D.h"

#include "src/Core.h"
#include "src/renderer/Renderer.h"
#include "src/debug/Profiler.h"
#include "src/external/stb_image/stb_image.h"


namespace Syn {


	Texture2D::Texture2D(const std::string& _asset_path) :
		m_assetPath(_asset_path)
	{
		SYN_PROFILE_FUNCTION();

		// read data from image file
		int w, h, c;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* data = stbi_load(_asset_path.c_str(), &w, &h, &c, 0);
		if (!data)
		{
			SYN_CORE_WARNING("Couldn't load file '", _asset_path, "'.");
		}
		else
		{ 
			m_width = (uint32_t)w;
			m_height = (uint32_t)h;

			#ifdef DEBUG_TEXTURES
				SYN_CORE_TRACE("loaded '", _asset_path, "' (", m_width, "x", m_height, "x", c, ").");
			#endif

			// hack number of channels in
			if (c == 3)
			{
				m_fmt = TextureFormat::RGB8;
				m_pxFmt = TexturePixelFormat(GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);
			}
			else // assume 4 channels
			{
				m_fmt = TextureFormat::RGBA8;
				m_pxFmt = TexturePixelFormat(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
			}
			//GLenum storageChannels = (c == 3 ? GL_RGB8 : GL_RGBA8);
			//GLenum uploadChannels = (c == 3 ? GL_RGB : GL_RGBA);

			SYN_RENDER_S1(data, {
				// create texture
				glCreateTextures(GL_TEXTURE_2D, 1, &self->m_textureID);
				glTextureStorage2D(self->m_textureID, 1, self->m_pxFmt.internalFormat, self->m_width, self->m_height);
				//glBindTexture(GL_TEXTURE_2D, m_textureID);

				// upload to VRAM
				glTextureSubImage2D(self->m_textureID, 0, 0, 0, self->m_width, self->m_height, self->m_pxFmt.storageFormat, self->m_pxFmt.storageType, data);
				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

				// set texture parameters
				glGenerateMipmap(GL_TEXTURE_2D);

				glTextureParameteri(self->m_textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				glTextureParameteri(self->m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				glTextureParameteri(self->m_textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(self->m_textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);

				// release heap-alloc data
				stbi_image_free(data);
			});
		}

		//glBindTexture(GL_TEXTURE_2D, 0);

	}


	//-----------------------------------------------------------------------------------
	Texture2D::Texture2D(uint32_t _width, uint32_t _height, TextureFormat _texure_format)
	{
		SYN_PROFILE_FUNCTION();

		m_width = _width;
		m_height = _height;
		m_fmt = _texure_format;

		// set storage, upload and type from format
		switch (m_fmt)
		{
			case TextureFormat::R8:		m_pxFmt = TexturePixelFormat(GL_R8,    GL_RED,  GL_UNSIGNED_BYTE);	break;
			case TextureFormat::RG8:	m_pxFmt = TexturePixelFormat(GL_RG8,   GL_RG,   GL_UNSIGNED_BYTE);	break;
			case TextureFormat::RGB8:	m_pxFmt = TexturePixelFormat(GL_RGB8,  GL_RGB,  GL_UNSIGNED_BYTE);	break;
			case TextureFormat::RGBA8:	m_pxFmt = TexturePixelFormat(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);	break;

			case TextureFormat::R32F:		m_pxFmt = TexturePixelFormat(GL_R16F,    GL_RED,  GL_FLOAT);	break;
			case TextureFormat::RG32F:		m_pxFmt = TexturePixelFormat(GL_RG16F,   GL_RG,   GL_FLOAT);	break;
			case TextureFormat::RGB32F:		m_pxFmt = TexturePixelFormat(GL_RGB16F,  GL_RGB,  GL_FLOAT);	break;
			case TextureFormat::RGBA32F:	m_pxFmt = TexturePixelFormat(GL_RGBA16F, GL_RGBA, GL_FLOAT);	break;
		}

		SYN_RENDER_S0({
			glCreateTextures(GL_TEXTURE_2D, 1, &self->m_textureID);
			glTextureStorage2D(self->m_textureID, 1, self->m_pxFmt.internalFormat, self->m_width, self->m_height);

			glTextureParameteri(self->m_textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(self->m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTextureParameteri(self->m_textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(self->m_textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		});

	}


	//-----------------------------------------------------------------------------------
	Texture2D::~Texture2D()
	{
		SYN_RENDER_S0({
			glDeleteTextures(1, &self->m_textureID);
		});
	}


	//-----------------------------------------------------------------------------------
	void Texture2D::bind(uint32_t _tex_slot)
	{
		SYN_RENDER_S1(_tex_slot, {
			glBindTextureUnit(_tex_slot, self->m_textureID);
		});
	}


	//-----------------------------------------------------------------------------------
	void Texture2D::setData(void* _data, size_t _size_in_bytes)
	{
		SYN_PROFILE_FUNCTION();

		// assert full texture
		uint32_t sz = m_width * m_height * getTextureFmtChannels(m_fmt) * getTextureFmtTypeSize(m_fmt);
		if (_size_in_bytes != sz)
		{
			SYN_CORE_WARNING("size mismatch: _data: ", _size_in_bytes, " != ", sz, ". Only full texture upload permitted");
			return;
		}

		SYN_RENDER_S2(_data, _size_in_bytes, {
			// upload to VRAM
			glTextureSubImage2D(self->m_textureID, 0, 0, 0, self->m_width, self->m_height, self->m_pxFmt.storageFormat, self->m_pxFmt.storageType, _data);
			
			#ifdef DEBUG_TEXTURES
				SYN_CORE_TRACE("Texture data uploaded (", _size_in_bytes, " bytes).");
			#endif
		});

	}


}

