
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
			GLenum storageChannels = (c == 3 ? GL_RGB8 : GL_RGBA8);
			GLenum uploadChannels = (c == 3 ? GL_RGB : GL_RGBA);

			SYN_RENDER_S3(storageChannels, uploadChannels, data, {
				// create texture
				glCreateTextures(GL_TEXTURE_2D, 1, &self->m_textureID);
				glTextureStorage2D(self->m_textureID, 1, storageChannels, self->m_width, self->m_height);
				//glBindTexture(GL_TEXTURE_2D, m_textureID);

				// upload to VRAM
				glTextureSubImage2D(self->m_textureID, 0, 0, 0, self->m_width, self->m_height, uploadChannels, GL_UNSIGNED_BYTE, data);
				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

				// set texture parameters
				glGenerateMipmap(GL_TEXTURE_2D);
				glTextureParameteri(self->m_textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(self->m_textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTextureParameteri(self->m_textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				glTextureParameteri(self->m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				stbi_image_free(data);
			});
		}

		//glBindTexture(GL_TEXTURE_2D, 0);

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


}