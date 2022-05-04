
#pragma once


#include <glm/glm.hpp>
#include <random>
#include <cmath>

#include "FastNoise/FastNoise.h"

#include "Synapse/Core.hpp"
#include "Synapse/Types.hpp"
#include "Synapse/Utils/Random/Random.hpp"



// type definitions of member function pointer

/* FastNoise single value 2D noise functions. */
typedef float(FastNoise::* FastNoise2DFncPtr)(float, float);
/* FastNoise single value 3D noise functions. */
typedef float(FastNoise::* FastNoise3DFncPtr)(float, float, float);
/* FastNoise single value 4D noise functions. */
typedef float(FastNoise::* FastNoise4DFncPtr)(float, float, float, float);

/*	Macro for ease of access of FastNoise member functions --
	note: uses the static FastNoise instance FastNoise Noise::s_f_noise. */
#define FASTNOISE_MEMBER_FNC_PTR(ptr) ((s_f_noise).*(ptr))


// disable memory allocation warning, I know what I'm doing...
#pragma warning(disable : 6011)


namespace Syn {


	// forward declarations
	class NoisePerlin2;
	class NoisePerlin3;
	class NoiseValue2;
	class NoiseValue3;


	// static class
	class Noise
	{
	public:

		// mixing functions
		inline static float lerp(float a, float b, float t) { return a * (1.0f - t) + b * t; }
		inline static float mix(float a, float b, float t) { return a + t * (b - a); }

		// stepping functions
		inline static float smoothStep(float t) { return t * t * (3 - 2 * t); }
		inline static float smoothStep_d(float t) { return 6 * t * (1 - t); }
		inline static float perlinStep(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
		inline static float perlinStep_d(float t) { return 30 * t * t * (t * (t - 2) + 1); }

		// misc
		inline static float fract(float f) { return f - floor(f); }

		// hashes / random-number gens
		inline static float frand() { return static_cast<float>(rand()) / (static_cast<float>(INT_MAX) + 1.0f); }


		// instance of FastNoise
		static FastNoise s_f_noise;
		// function pointers used by fBm functions -- which type of noise to use?
		static FastNoise2DFncPtr s_f_noise_2D_ptr;
		static FastNoise3DFncPtr s_f_noise_3D_ptr;
		static FastNoise4DFncPtr s_f_noise_4D_ptr;

		/* Generate 2D noise using preset noise function (s_f_noise_2D_ptr). */
		static float fbm_perlin2(const glm::vec2& _p, bool _rotate=true);
		static float single_2D(const glm::vec2& _p);

		/* Generate 3D noise using preset noise function (s_f_noise_3D_ptr) */
		static float fbm_perlin3(const glm::vec3& _p, bool _rotate=true);

		/* Generate 4D noise using preset noise function (s_f_noise_4D_ptr) */
		static float single_4D(const glm::vec4& _p);
		static float single_4D(float _x, float _y, float _z, float _w);

		/* Save provided noise as .png. */
		static void save_noise_PNG(const std::string& _file_name, uint32_t _width, uint32_t _height, float* _data);


		// accessors
		static void set_noise_parameters(const NoiseParameters& _param) { s_noise_param = _param; set_noise_frequency(_param.baseFreq); }
		/* Set octave count for fractal noise; default = 3. */ 
		static void set_param_octave_count(const int _count) { s_noise_param.octaveCount = _count; }
		/* Set base frequency of noise; default = 0.01f. */
		static void set_param_base_frequency(const float _freq) { s_noise_param.baseFreq = _freq; set_noise_frequency(_freq); }
		/* Set delta frequency per octave in fractal noise; default = 2.0f. */
		static void set_param_delta_freq(const float _freq) { s_noise_param.deltaFreq = _freq; }
		/* Set base amplitude of noise; default = 1.0f. */
		static void set_param_base_amp(const float _amp) { s_noise_param.baseAmp = _amp; }
		/* Set delta amplitude per octave in fractal noise; default = 0.5f. */
		static void set_param_delta_amp(const float _amp) { s_noise_param.deltaAmp = _amp; }
		/* Set noise offset; default glm::vec3(0.0f). */
		static void set_param_offset(const glm::vec3& _offset) { s_noise_param.offset = _offset; }
		static NoiseParameters& get_noise_parameters() { return s_noise_param; }
		static void set_noise_type_2D(NoiseType _type=NoiseType::Value)
		{
			switch (_type)
			{
				case Syn::NoiseType::Value:		s_f_noise_2D_ptr = &FastNoise::GetValue2;	 break;
				case Syn::NoiseType::Simplex:	s_f_noise_2D_ptr = &FastNoise::GetSimplex2;	 break;
				case Syn::NoiseType::Perlin:	s_f_noise_2D_ptr = &FastNoise::GetPerlin2;	 break;
				case Syn::NoiseType::Cellular:	s_f_noise_2D_ptr = &FastNoise::GetCellular2; break;
				case Syn::NoiseType::Cubic:		s_f_noise_2D_ptr = &FastNoise::GetCubic2;	 break;
				default:	SYN_CORE_FATAL_ERROR("unknown Syn::NoiseType: ", (int)_type);	 break;
			}
		}
		static void set_noise_type_3D(NoiseType _type=NoiseType::Value)
		{
			switch (_type)
			{
				case Syn::NoiseType::Value:		s_f_noise_3D_ptr = &FastNoise::GetValue3;    break;
				case Syn::NoiseType::Simplex:	s_f_noise_3D_ptr = &FastNoise::GetSimplex3;  break;
				case Syn::NoiseType::Perlin:	s_f_noise_3D_ptr = &FastNoise::GetPerlin3;	 break;
				case Syn::NoiseType::Cellular:	s_f_noise_3D_ptr = &FastNoise::GetCellular3; break;
				case Syn::NoiseType::Cubic:		s_f_noise_3D_ptr = &FastNoise::GetCubic3;	 break;
				default:	SYN_CORE_FATAL_ERROR("unknown Syn::NoiseType: ", (int)_type);	 break;
			}
		}
		static void set_noise_type_4D(NoiseType _type=NoiseType::Simplex)
		{
			switch (_type)
			{
				case Syn::NoiseType::Simplex:	s_f_noise_4D_ptr = &FastNoise::GetSimplex4;	 break;
				default:	SYN_CORE_FATAL_ERROR("unknown Syn::NoiseType: ", (int)_type);	 break;
			}
		}
		static void set_noise_frequency(float _frequency) { s_f_noise.SetFrequency(_frequency); }

		/* Simple and fractional noise with partial derivatives.
		 *
		 * IMPORTANT: the input vector _v at which the noise is evaluated, has to be a float vector, 
		 * since the noise interpolation is based on fractions and integers at noise 'cell' corners.
		 * Simply stepping through a field (of scalars), using integer (x, y, z) vectors will thus
		 * always result in noise evaluated to 0.0f. See examples below:
		 * 
		 * WILL NOT WORK, n.x will always be 0.0f!
		 * 	for (int y = 0; y < yn; y++)
		 * 		for (int x = 0; x < xn; x++)
		 * 			glm::vec3 n = eval_perlin2_d(glm::vec2(x, y))
		 * 
		 * DOES WORK!
		 * 	float step_x = 1.0f / xn;
		 * 	float step_y = 1.0f / yn;
		 * 	for (int y = 0; y < yn; y++)
		 * 		for (int x = 0; x < xn; x++)
		 * 			glm::vec3 n = eval_perlin2_d(glm::vec2(step_x * x, step_y * y))
		 * 
		 */
		static glm::vec3 eval_perlin2_d(const glm::vec2& _v);
		static glm::vec4 eval_perlin3_d(const glm::vec3& _v);
		static glm::vec3 fbm_perlin2_d(const glm::vec2& _v);
		static glm::vec4 fbm_perlin3_d(const glm::vec3& _v);
		static glm::vec3 fbm_value2_d(const glm::vec2& _v);
		static glm::vec4 fbm_value3_d(const glm::vec3& _v);


	private:
		static std::unique_ptr<NoisePerlin3> s_perlin3;
		static std::unique_ptr<NoisePerlin2> s_perlin2;
		static std::unique_ptr<NoiseValue2> s_value2;
		static std::unique_ptr<NoiseValue3> s_value3;
		
		// static rotation matrix for 2D noise
		static glm::mat2 s_rotation_mat2;
		static glm::mat3 s_rotation_mat3;

		// noise paramters
		static NoiseParameters s_noise_param;
	};

}

