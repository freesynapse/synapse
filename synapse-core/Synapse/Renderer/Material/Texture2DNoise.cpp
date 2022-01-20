
#include "pch.hpp"

#include "Synapse/Renderer/Material/Texture2DNoise.hpp"
#include "Synapse/Renderer/Renderer.hpp"
#include "Synapse/Utils/Timer/Timer.hpp"
#include "External/stb_image/stb_image.h"
//#include "External/stb_image/stb_image_write.h"


namespace Syn
{

	Texture2DNoise::Texture2DNoise(uint32_t _side)
	{
		m_width = m_height = _side;
		// allocate memory to store noise and pixel data
		m_pixelData = new unsigned char[3 * _side * _side];
		m_noiseData = new float[_side * _side];
		memset(m_pixelData, 0, 3 * _side * _side);
		memset(m_noiseData, 0, sizeof(float) * _side * _side);

		SYN_RENDER_S0({
			// create texture
			glCreateTextures(GL_TEXTURE_2D, 1, &self->m_textureID);
			// allocate storage
			glTextureStorage2D(self->m_textureID, 1, GL_RGB8, self->m_width, self->m_height);
			// upload to VRAM
			//glTextureSubImage2D(self->m_textureID, 0, 0, 0, self->m_width, self->m_height, GL_RGB, GL_UNSIGNED_BYTE, data);
			// texture parameters
			glGenerateMipmap(GL_TEXTURE_2D);
			glTextureParameteri(self->m_textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(self->m_textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTextureParameteri(self->m_textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTextureParameteri(self->m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		});

	}


	//-----------------------------------------------------------------------------------
	Texture2DNoise::~Texture2DNoise()
	{
		if (m_noiseData != nullptr)
			delete[] m_noiseData;

		if (m_pixelData != nullptr)
			delete[] m_pixelData;

		SYN_RENDER_S0({
			glDeleteTextures(1, &self->m_textureID);
		});
	}


	//-----------------------------------------------------------------------------------
	void Texture2DNoise::setDataDebug1D(float* _data)
	{
		// incoming data in range [0..1] and with m_width elements.
		m_noiseData = _data;
	
		uint32_t index = 0;
		for (uint32_t x = 0; x < m_width; x++)
		{
			uint32_t h = (uint32_t)(m_noiseData[x] * m_height);
			//uint32_t y = h;
			for (uint32_t y = 0; y < h; y++)
			{
				index = 3 * (y * m_width + x);
				m_pixelData[index+0] = 255;
				m_pixelData[index+1] = 255;
				m_pixelData[index+2] = 255;
			}
		}

		SYN_RENDER_S0({
			// upload new data to VRAM
			glTextureSubImage2D(self->m_textureID, 0, 0, 0, self->m_width, self->m_height, GL_RGB, GL_UNSIGNED_BYTE, self->m_pixelData);
		});
	}


	//-----------------------------------------------------------------------------------
	void Texture2DNoise::setDataDebug2D(float* _data, const glm::vec2& _noise_limits)
	{
		// delete existing data if this is a reset
		if (m_noiseData != nullptr)
			delete[] m_noiseData;

		// incoming data in range [_noise_limits.x.._noise_limits.y] and with m_size elements.
		// please normalize to range [0..1] before rendering..

		m_noiseData = _data;

		uint32_t index = 0;
		// offset normalization - push to [0..max]
		float offset = -_noise_limits.x;
		// scale normalization - push to [0..1]
		float div = 1.0f / (_noise_limits.y - _noise_limits.x);
		
		for (uint32_t y = 0; y < m_height; y++)
		{
			for (uint32_t x = 0; x < m_width; x++)
			{
				// 2d index into 1d array, as per usual
				index = y * m_width + x;
		
				// map to range [0..1]
				float normalizedNoise = (m_noiseData[index] + offset) * div;
				
				// convert to [0..255] for rendering
				unsigned char c = static_cast<unsigned char>(normalizedNoise * 255.0f);
				
				// three (3) channels per pixel
				m_pixelData[3 * index + 0] = c;
				m_pixelData[3 * index + 1] = c;
				m_pixelData[3 * index + 2] = c;
			}
		}
		
		SYN_CORE_TRACE("creating texture (", m_width, "x", m_height, " px).");

		SYN_RENDER_S0({
			// upload to VRAM
			glTextureSubImage2D(self->m_textureID, 0, 0, 0, self->m_width, self->m_height, GL_RGB, GL_UNSIGNED_BYTE, self->m_pixelData);
		});
	}

	
	//-----------------------------------------------------------------------------------
	void Texture2DNoise::setFromNoiseFunc(bool _rotate_noise)
	{
		// if this is a reset, delete and reallocate memory
		if (m_noiseData != nullptr)
			delete[] m_noiseData;

		m_noiseData = new float[m_width * m_height];

		uint32_t index = 0;
		glm::vec2 v(0);
		float n = 0.0f;

		for (uint32_t y = 0; y < m_height; y++)
		{
			for (uint32_t x = 0; x < m_width; x++)
			{
				index = y * m_width + x;
				v = glm::vec2(x, y);
				// TODO : replace by selected fractional function? -- don't know what this means, function pointer?! can't remember...
				n = Syn::Noise::fbm_perlin2(v, _rotate_noise);
				m_noiseData[index] = n;
				unsigned char c = static_cast<unsigned char>(n * 255.0f);

				m_pixelData[3 * index + 0] = c;
				m_pixelData[3 * index + 1] = c;
				m_pixelData[3 * index + 2] = c;

			}
		}

		SYN_RENDER_S0({
			// upload to VRAM
			glTextureSubImage2D(self->m_textureID, 0, 0, 0, self->m_width, self->m_height, GL_RGB, GL_UNSIGNED_BYTE, self->m_pixelData);
		});

	}
	

	//-----------------------------------------------------------------------------------
	void Texture2DNoise::setFromNoiseData(float* _noise_data)
	{
		// if reassignment, delete[]
		if (m_noiseData != nullptr)
			delete[] m_noiseData;

		// copy and store data, don't store pointer
		size_t sz = m_width * m_height;
		m_noiseData = new float[sz];
		memcpy(m_noiseData, _noise_data, sizeof(float) * sz);

		uint32_t index;

		for (uint32_t y = 0; y < m_height; y++)
		{
			for (uint32_t x = 0; x < m_width; x++)
			{
				index = y * m_width + x;
				float n = _noise_data[index];
				unsigned char c = static_cast<unsigned char>(n * 255.0f);
				m_pixelData[3 * index + 0] = c;
				m_pixelData[3 * index + 1] = c;
				m_pixelData[3 * index + 2] = c;
			}
		}

		SYN_RENDER_S0({
			// upload to VRAM
			glTextureSubImage2D(self->m_textureID, 0, 0, 0, self->m_width, self->m_height, GL_RGB, GL_UNSIGNED_BYTE, self->m_pixelData);
		});

	}


	//-----------------------------------------------------------------------------------
	void Texture2DNoise::setFromPixelData(unsigned char* _px_data)
	{
		if (m_pixelData != nullptr)
			delete[] m_pixelData;

		m_pixelData = new unsigned char[3 * m_width * m_height];
		memcpy(m_pixelData, _px_data, 3 * m_width * m_height);

		SYN_RENDER_S0({
			// upload to VRAM
			glTextureSubImage2D(self->m_textureID, 0, 0, 0, self->m_width, self->m_height, GL_RGB, GL_UNSIGNED_BYTE, self->m_pixelData);
		});
	}


	//-----------------------------------------------------------------------------------
	void Texture2DNoise::scale(float _f)
	{
		// TODO : figure out why this works without a new call to glTextureSubImage2D()!

		if (m_pixelData == nullptr)
			return;

		// cant scale using a negative value
		float scale = MAX(_f, 0.0f);

		uint32_t index;

		for (uint32_t y = 0; y < m_height; y++)
		{
			for (uint32_t x = 0; x < m_width; x++)
			{
				index = y * m_width + x;
				float old_c = static_cast<float>(m_pixelData[3 * index]);
				unsigned char c = static_cast<unsigned char>(old_c * scale);
				m_pixelData[3 * index + 0] = c;
				m_pixelData[3 * index + 1] = c;
				m_pixelData[3 * index + 2] = c;
			}
		}
	}


	//-----------------------------------------------------------------------------------
	void Texture2DNoise::convolute(const Ref<Texture2DNoise>& _texture_ptr, float _weight)
	{
		if (_texture_ptr == nullptr)
			return;

		//if (m_pixelData == nullptr)

		// check for matching dimensions, incoming texture have to be at least the same size
		if (m_width > _texture_ptr->getWidth() || m_height > _texture_ptr->getHeight())
		{
			SYN_CORE_WARNING("incoming texture too small.");
			return;
		}

		float thatWeight = MIN(MAX(_weight, 0.0f), 1.0f);
		float thisWeight = 1.0f - thatWeight;

		float* incomingData = _texture_ptr->getNoiseData();
		uint32_t index;
		float a, b, n;
		unsigned char c;

		for (uint32_t y = 0; y < m_height; y++)
		{
			for (uint32_t x = 0; x < m_width; x++)
			{
				index = y * m_width + x;
				a = m_noiseData[index];
				b = incomingData[index];
				n = a * thisWeight + b * thatWeight;
				m_noiseData[index] = n;
				
				c = static_cast<unsigned char>(n * 255.0f);
				m_pixelData[3 * index + 0] = c;
				m_pixelData[3 * index + 1] = c;
				m_pixelData[3 * index + 2] = c;
			}
		}

		SYN_RENDER_S0({
			// upload to VRAM
			glTextureSubImage2D(self->m_textureID, 0, 0, 0, self->m_width, self->m_height, GL_RGB, GL_UNSIGNED_BYTE, self->m_pixelData);
		});

	}


	//-----------------------------------------------------------------------------------
	void Texture2DNoise::saveAsPNG(const std::string& _filename)
	{
		stbi_flip_vertically_on_write(1);
		stbi_write_png(_filename.c_str(), m_width, m_height, 3, (void*)m_pixelData, 0);
	}


	//-----------------------------------------------------------------------------------
	void Texture2DNoise::bind(uint32_t _tex_slot)
	{
		SYN_RENDER_S1(_tex_slot, {
			glBindTextureUnit(_tex_slot, self->m_textureID);
		});
	}



}