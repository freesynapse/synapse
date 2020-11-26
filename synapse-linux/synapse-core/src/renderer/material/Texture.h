#pragma once


#include <stdint.h>

#include "src/Core.h"
#include "src/external/stb_image/stb_image_write.h"


namespace Syn {


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

