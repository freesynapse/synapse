
#include "../../../pch.hpp"

#include "../../../External/stb_image/stb_image_write.h"
#include "../../../External/FastNoise/FastNoise.h"

#include "Noise.hpp"
#include "NoiseGenerator.hpp"


// buffer overflow in indexing 3 * size * size in save_noise_PNG()
#pragma warning(disable : 6386)


namespace Syn
{

	// static declarations of NoiseGenerators for derivatives of noise
	std::unique_ptr<NoisePerlin2> Noise::s_perlin2 = std::make_unique<NoisePerlin2>();
	std::unique_ptr<NoisePerlin3> Noise::s_perlin3 = std::make_unique<NoisePerlin3>();
	std::unique_ptr<NoiseValue2> Noise::s_value2 = std::make_unique<NoiseValue2>();
	std::unique_ptr<NoiseValue3> Noise::s_value3 = std::make_unique<NoiseValue3>();

	// FastNoise instance
	FastNoise Noise::s_f_noise;
	
	// static noise parameters with default values
	NoiseParameters Noise::s_noise_param = NoiseParameters(3, 0.01f, 2.0f, 1.0f, 0.5f, glm::vec3(0));

	// rotation matrix (for rotated, layered noise)
	glm::mat2 Noise::s_rotation_mat2 = glm::mat2( 0.80f,  0.60f, \
												 -0.60f,  0.80f);
	glm::mat3 Noise::s_rotation_mat3 = glm::mat3( 0.00f,  0.80f,  0.60f, \
												 -0.80f,  0.36f, -0.48f, \
												 -0.60f, -0.48f,  0.64f);

	// function pointers
	FastNoise2DFncPtr Noise::s_f_noise_2D_ptr = &FastNoise::GetValue2;
	FastNoise3DFncPtr Noise::s_f_noise_3D_ptr = &FastNoise::GetValue3;
	FastNoise4DFncPtr Noise::s_f_noise_4D_ptr = &FastNoise::GetSimplex4;


	//---------------------------------------------------------------------------------------
	// fBm noise without derivatives (using FastNoise function pointers).
	//---------------------------------------------------------------------------------------
	float Noise::fbm_perlin2(const glm::vec2& _p, bool _rotate)
	{
		// noise parameters
		glm::vec2 p = _p + glm::vec2(s_noise_param.offset.x, s_noise_param.offset.y);
		//s_f_noise.SetFrequency(s_noise_param.baseFreq);
		float  f = s_noise_param.baseFreq;
		float df = s_noise_param.deltaFreq;
		float  a = s_noise_param.baseAmp;
		float da = s_noise_param.deltaAmp;
		int    o = s_noise_param.octaveCount;
		float  r = a;// accumulated amplitude, for normalization
		// rotational fractal noise? no? then multiply each octave with identity.
		glm::mat2 rotMat = _rotate ? s_rotation_mat2 : glm::mat2(1.0f);
		
		// first octave
		float n = a * s_perlin2->eval(p, f);;
		//float n = /* baseAmp * */FASTNOISE_MEMBER_FNC_PTR(s_f_noise_2D_ptr)(p.x, p.y);

		// rest of octaves
		for (int i = 1; i < o; i++)
		{
			f *= df;
			a *= da;
			p = rotMat * p;
			n += a * s_perlin2->eval(p, f);
			r += a;
		}
		// return normalized noise
		return (n);// / r);
		//return (n / r + 1.0f) * 0.5f;

	}
	//---------------------------------------------------------------------------------------
	float Noise::single_2D(const glm::vec2& _p)
	{
		glm::vec2 p = _p + glm::vec2(s_noise_param.offset.x, s_noise_param.offset.z);
		return (FASTNOISE_MEMBER_FNC_PTR(s_f_noise_2D_ptr)(p.x, p.y) + 1.0f) * 0.5f;
	}
	//---------------------------------------------------------------------------------------
	float Noise::fbm_perlin3(const glm::vec3& _p, bool _rotate)
	{
		// noise parameters
		glm::vec3 p = _p + s_noise_param.offset;
		//s_f_noise.SetFrequency(s_noise_param.baseFreq);
		float  f = s_noise_param.baseFreq;
		float df = s_noise_param.deltaFreq;
		float  a = s_noise_param.baseAmp;
		float da = s_noise_param.deltaAmp;
		int    o = s_noise_param.octaveCount;
		float  r = a;// accumulated amplitude, for normalization
		// rotational fractal noise? no? then multiply each octave with identity.
		glm::mat3 rotMat = _rotate ? s_rotation_mat3 : glm::mat3(1.0f);

		// first octave
		//float n = /* baseAmp * */FASTNOISE_MEMBER_FNC_PTR(s_f_noise_3D_ptr)(p.x, p.y, p.z);
		float n = a * s_perlin3->eval(p, f);

		// rest of octaves
		for (int i = 1; i < o; i++)
		{
			f *= df;						// update frequency every octave
			r += a;							// total amplitude for scaling
			a *= da;						// update (decrease) amplitude
			p = rotMat * p;					// rotate for this octave
			n += a * s_perlin3->eval(p, f);	// add next layer of noise
			//n += baseAmp * FASTNOISE_MEMBER_FNC_PTR(s_f_noise_3D_ptr)(p.x, p.y, p.z);
		}
		// return normalized noise
		return (n);// / r);
		//return (n / r + 1.0f) * 0.5f;
	}
	//---------------------------------------------------------------------------------------
	float Noise::single_4D(const glm::vec4& _p)
	{
		glm::vec4 p = _p + glm::vec4(s_noise_param.offset.x,
									 s_noise_param.offset.y,
									 s_noise_param.offset.z,
									 0.0f);
		return (FASTNOISE_MEMBER_FNC_PTR(s_f_noise_4D_ptr)(p.x, p.y, p.z, p.w) + 1.0f) * 0.5f;
	}
	//---------------------------------------------------------------------------------------
	float Noise::single_4D(float _x, float _y, float _z, float _w)
	{
		float x = _x + s_noise_param.offset.x;
		float y = _y + s_noise_param.offset.y;
		float z = _z + s_noise_param.offset.z;
		float w = _w + 0.0f;
		
		return (FASTNOISE_MEMBER_FNC_PTR(s_f_noise_4D_ptr)(x, y, z, w) + 1.0f) * 0.5f;
	}
	//---------------------------------------------------------------------------------------
	// Single noise evaluations with partial derivatives
	//---------------------------------------------------------------------------------------
	glm::vec3 Noise::eval_perlin2_d(const glm::vec2& _v)
	{
		glm::vec2 dv = glm::vec2(0.0f);	// partial derivatives
		float t = s_perlin2->eval_d(_v, dv, s_noise_param.baseFreq);
		// return vec3(t, dx, dy)
		return glm::vec3(t, dv);
	}
	//---------------------------------------------------------------------------------------
	glm::vec4 Noise::eval_perlin3_d(const glm::vec3& _v)
	{
		glm::vec3 dv = glm::vec3(0.0f);
		float t = s_perlin3->eval_d(_v, dv, s_noise_param.baseFreq);
		// return vec4(t, dx, dy, dz)
		return glm::vec4(t, dv);
	}
	//---------------------------------------------------------------------------------------
	// fBm noise with partial derivatives.
	//---------------------------------------------------------------------------------------
	glm::vec3 Noise::fbm_perlin2_d(const glm::vec2& _v)
	{
		float a  = s_noise_param.baseAmp;	// base amplitude
		float da = s_noise_param.deltaAmp;	// amplitude change every octave
		float f  = s_noise_param.baseFreq;	// base frequency
		float df = s_noise_param.deltaFreq;	// frequency change every octave
		float r  = a;						// range of amplitudes for scaling accumulated noise
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
		//float div = 1.0f / r;
		//return glm::vec3(t * div, tn * div);
		return glm::vec3(t, glm::normalize(tn));
	}
	//---------------------------------------------------------------------------------------
	glm::vec4 Noise::fbm_perlin3_d(const glm::vec3& _v)
	{
		float a  = s_noise_param.baseAmp;	// base amplitude
		float da = s_noise_param.deltaAmp;	// amplitude change every octave
		float f  = s_noise_param.baseFreq;	// base frequency
		float df = s_noise_param.deltaFreq;	// frequency change every octave
		float r  = a;						// range of amplitudes for scaling accumulated noise
		glm::vec3 tn = glm::vec3(0.0f);		// accumulated partial derivatives
		glm::vec3 dn = glm::vec3(0.0f);		// partial derivatives for this octave
		// get the first octave of noise
		float t = s_perlin3->eval_d(_v, tn, f);
		
		for (int i = 1; i < s_noise_param.octaveCount; i++)
		{
			f *= df;
			a *= da;
			r += a;
			t += s_perlin3->eval_d(_v, dn, f) * a;
			tn += dn * a;
		}
		//float div = 1.0f / r;
		//return glm::vec4(t * div, tn * div);r
		return glm::vec4(t, glm::normalize(tn));
	}
	//---------------------------------------------------------------------------------------
	glm::vec3 Noise::fbm_value2_d(const glm::vec2& _v)
	{
		float a  = s_noise_param.baseAmp;	// base amplitude
		float da = s_noise_param.deltaAmp;	// amplitude change every octave
		float f  = s_noise_param.baseFreq;	// base frequency
		float df = s_noise_param.deltaFreq;	// frequency change every octave
		float r  = a;						// range of amplitudes for scaling accumulated noise
		glm::vec2 tn(0.0f);	// accumulated partial derivatives
		glm::vec2 dn(0.0f); // partial derivatives for this octave
		printf("%d %.1f\n", s_noise_param.octaveCount, s_noise_param.baseFreq);
		float t = s_value2->eval_d(_v, tn, f) * a;	// get the first octave of noise

		for (int i = 1; i < s_noise_param.octaveCount; i++)
		{
			f *= df;
			a *= da;
			r += a;
			t += s_value2->eval_d(_v, dn, f) * a;
			tn += dn * a;
		}
		//float div = 1.0f / r;
		//return glm::vec3((t - 0.5f) * div, tn * div);
		return glm::vec3(t, glm::normalize(tn));
	}
	//---------------------------------------------------------------------------------------
	glm::vec4 Noise::fbm_value3_d(const glm::vec3& _v)
	{
		float a  = s_noise_param.baseAmp;	// base amplitude
		float da = s_noise_param.deltaAmp;	// amplitude change every octave
		float f  = s_noise_param.baseFreq;	// base frequency
		float df = s_noise_param.deltaFreq;	// frequency change every octave
		float r  = a;						// range of amplitudes for scaling accumulated noise
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
		//float div = 1.0f / r;
		//return glm::vec4((t - 0.5f) * div, tn * div);
		return glm::vec4(t, glm::normalize(tn));
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

				m_pixels[3 * index+0] = c;
				m_pixels[3 * index+1] = c;
				m_pixels[3 * index+2] = c;
				index += 3;
			}
		}

		stbi_flip_vertically_on_write(1);
		stbi_write_png(_file_name.c_str(), _width, _height, 3, m_pixels, 0);

		delete[] m_pixels;

		SYN_CORE_TRACE("saved image to '", _file_name, "'.");
	}


}
