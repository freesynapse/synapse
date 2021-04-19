
#include "pch.hpp"

#include "Synapse/Utils/Noise/Noise.hpp"
#include "Synapse/Utils/Noise/NoiseGenerator.hpp"
#include "External/stb_image/stb_image_write.h"
#include "External/FastNoise/FastNoise.h"


// buffer overflow in indexing 3 * size * size in save_noise_PNG()
#pragma warning(disable : 6386)


namespace Syn {


	// static declarations of NoiseGenerators for derivatives of noise
	std::unique_ptr<NoisePerlin2> Noise::s_perlin2 = std::make_unique<NoisePerlin2>();
	std::unique_ptr<NoisePerlin3> Noise::s_perlin3 = std::make_unique<NoisePerlin3>();
	std::unique_ptr<NoiseValue2> Noise::s_value2 = std::make_unique<NoiseValue2>();
	std::unique_ptr<NoiseValue3> Noise::s_value3 = std::make_unique<NoiseValue3>();

	// FastNoise instance
	FastNoise Noise::s_f_noise;
	
	// static noise parameters with base values
	NoiseParameters Noise::s_noise_param;

	// rotation matrix (for rotated, layered noise)
	glm::mat2 Noise::s_rotation_mat2 = glm::mat2(0.8f, 0.6f, -0.8f, 0.6f);

	// function pointers
	FastNoise2DFncPtr Noise::s_f_noise_2D_ptr = &FastNoise::GetValue2;
	FastNoise3DFncPtr Noise::s_f_noise_3D_ptr = &FastNoise::GetValue3;


	//---------------------------------------------------------------------------------------
	/* Generate 2D noise using preset noise function (s_f_noise_2D_ptr). */
	float Noise::fbm_2D(const glm::vec2& _p, bool _rotate)
	{
		// noise parameters
		glm::vec2 p = _p + glm::vec2(s_noise_param.offset.x, s_noise_param.offset.z);
		//s_f_noise.SetFrequency(s_noise_param.baseFreq);
		float dFreq = s_noise_param.deltaFreq;
		float baseAmp = 1.0f;
		float dAmp = s_noise_param.deltaAmp;
		int octaves = s_noise_param.octaveCount;
		float totalAmps = 1.0f;
		// rotational fractal noise? no? then multiply each octave with identity.
		glm::mat2 rotMat = _rotate ? s_rotation_mat2 : glm::mat2(1.0f);

		// first octave
		float n = /* baseAmp * */FASTNOISE_MEMBER_FNC_PTR(s_f_noise_2D_ptr)(p.x, p.y);

		// rest of octaves
		for (int i = 1; i < octaves; i++)
		{
			// update (decrease) amplitude
			baseAmp *= dAmp;
			// add to total for scaling
			totalAmps += baseAmp;
			// update frequency and (optionally) rotate
			p = p * rotMat * dFreq;
			// add next layer of noise
			n += baseAmp * FASTNOISE_MEMBER_FNC_PTR(s_f_noise_2D_ptr)(p.x, p.y);
		}

		// return normalized noise
		return (n / totalAmps + 1.0f) * 0.5f;

	}
	//---------------------------------------------------------------------------------------
	float Noise::single_2D(const glm::vec2& _p)
	{
		glm::vec2 p = _p + glm::vec2(s_noise_param.offset.x, s_noise_param.offset.z);
		return (FASTNOISE_MEMBER_FNC_PTR(s_f_noise_2D_ptr)(p.x, p.y) + 1.0f) * 0.5f;
	}


	//---------------------------------------------------------------------------------------
	// fBm noise with partial drivatives.
	//-----------------------------------------------------------------------------------
	glm::vec3 Noise::fbm_perlin2_d(const glm::vec2& _v)
	{
		float a  = 1.0f;	// base amplitude
		float da = 0.5f;	// amplitude change every octave
		float f  = 1.0f;	// base frequency
		float df = 2.0f;	// frequency change every octave
		float r  = 1.0f;	// range of amplitudes for scaling accumulated noise
		glm::vec2 tn(0.0f);	// accumulated partial derivatives
		glm::vec2 dn(0.0f); // partial derivatives for this octave
		// get the first octave of noise
		float t = s_perlin2->eval_d(_v, tn, f) * a;

		for (int i = 1; i < s_noise_param.octaveCount; i++)
		{
			f *= df;
			a *= da;
			r += a;
			t += s_perlin2->eval_d(_v, dn, f) * a;
			tn += dn * a;
		}
		float div = 1.0f / r;
		return glm::vec3(t * div, tn * div);
	}
	//---------------------------------------------------------------------------------------
	glm::vec4 Noise::fbm_perlin3_d(const glm::vec3& _v)
	{
		float a  = 1.0f;				// base amplitude
		float da = 0.5f;				// amplitude change every octave
		float f  = 1.0f;				// base frequency
		float df = 2.0f;				// frequency change every octave
		float r  = 1.0f;				// range of amplitudes for scaling accumulated noise
		glm::vec3 tn = glm::vec3(0.0f);	// accumulated partial derivatives
		glm::vec3 dn = glm::vec3(0.0f);	// partial derivatives for this octave
		// get the first octave of noise
		float t = s_perlin3->eval_d(_v, tn, f) * a;
		
		for (int i = 1; i < s_noise_param.octaveCount; i++)
		{
			f *= df;
			a *= da;
			r += a;
			t += s_perlin3->eval_d(_v, dn, f) * a;
			tn += dn * a;
		}
		float div = 1.0f / r;
		return glm::vec4(t * div, tn * div);
	}
	//---------------------------------------------------------------------------------------
	glm::vec3 Noise::fbm_value2_d(const glm::vec2& _v)
	{
		float  a = 1.0f;	// base amplitude
		float da = 0.5f;	// amplitude change every octave
		float  f = 1.0f;	// base frequency
		float df = 2.0f;	// frequency change every octave
		float  r = 1.0f;	// range of amplitudes for scaling accumulated noise
		glm::vec2 tn(0.0f);	// accumulated partial derivatives
		glm::vec2 dn(0.0f); // partial derivatives for this octave
		// get the first octave of noise
		float t = s_value2->eval_d(_v, tn, f) * a;

		for (int i = 1; i < s_noise_param.octaveCount; i++)
		{
			f *= df;
			a *= da;
			r += a;
			t += s_value2->eval_d(_v, dn, f) * a;
			tn += dn * a;
		}
		float div = 1.0f / r;
		return glm::vec3((t - 0.5f) * div, tn * div);
	}
	//---------------------------------------------------------------------------------------
	glm::vec4 Noise::fbm_value3_d(const glm::vec3& _v)
	{
		float  a = 1.0f;	// base amplitude
		float da = 0.5f;	// amplitude change every octave
		float  f = 1.0f;	// base frequency
		float df = 2.0f;	// frequency change every octave
		float  r = 1.0f;	// range of amplitudes for scaling accumulated noise
		glm::vec3 tn(0.0f);	// accumulated partial derivatives
		glm::vec3 dn(0.0f); // partial derivatives for this octave
		// get the first octave of noise
		float t = s_value3->eval_d(_v, tn, f) * a;

		for (int i = 1; i < s_noise_param.octaveCount; i++)
		{
			f *= df;
			a *= da;
			r += a;
			t += s_value3->eval_d(_v, dn, f) * a;
			tn += dn * a;
		}
		float div = 1.0f / r;
		return glm::vec4((t - 0.5f) * div, tn * div);
	}


	//---------------------------------------------------------------------------------------
	void Noise::save_noise_PNG(const std::string& _file_name, uint32_t _width, uint32_t _height, float* _data)
	{
		// RGB data assumed, i.e. 3 channels
		size_t sz = _height * _width * 3;

		unsigned char* m_pixels = new unsigned char[sz];
		memset(m_pixels, 0, sz);

		// normalization -- Perlin noise is in the range [ -sqrt(N/2) ... sqrt(N/2) ] where N is the dimensionality.
		//float f_sqrt = sqrtf(static_cast<float>(_dimensions) * 0.5f);
		//float f_sqrt_2_inv = 1.0f / (f_sqrt * 2.0f);
		for (uint32_t y = 0; y < _height; y++)
		{
			for (uint32_t x = 0; x < _width; x++)
			{
				uint32_t index = y * _width + x;
				unsigned char c = static_cast<unsigned char>(_data[index] * 255.0f);

				m_pixels[3 * index++] = c;
				m_pixels[3 * index++] = c;
				m_pixels[3 * index++] = c;
			}
		}

		stbi_flip_vertically_on_write(1);
		stbi_write_png(_file_name.c_str(), _width, _height, 3, m_pixels, 0);

		delete[] m_pixels;

		SYN_CORE_TRACE("saved image to '", _file_name, "'.");
	}


}