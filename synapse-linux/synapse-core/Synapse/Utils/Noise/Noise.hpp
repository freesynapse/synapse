#pragma once


#include <glm/glm.hpp>
#include <random>
#include <cmath>

#include "Synapse/Core.hpp"
#include "Synapse/Types.hpp"
#include "Synapse/External/FastNoise/FastNoise.h"
#include "Synapse/Utils/Random/Random.hpp"


// type definitions of member function pointer

/* FastNoise single value 2D noise functions. */
typedef float(FastNoise::* FastNoise2DFncPtr)(float, float);
/* FastNoise single value 3D noise functions. */
typedef float(FastNoise::* FastNoise3DFncPtr)(float, float, float);

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

		/* Generate 2D noise using preset noise function (s_f_noise_2D_ptr). */
		static float fbm_2D(const glm::vec2& _p, bool _rotate=true);
		static float single_2D(const glm::vec2& _p);

		/* Save provided noise as .png. */
		static void save_noise_PNG(const std::string& _file_name, uint32_t _width, uint32_t _height, float* _data);


		// accessors
		static void set_noise_parameters(const NoiseParameters& _param) { s_noise_param = _param; set_noise_frequency(_param.baseFreq); }
		static NoiseParameters& get_noise_parameters() { return s_noise_param; }
		static void set_noise_type_2D(NoiseType _type=NoiseType::Value)
		{
			switch (_type)
			{
				case Syn::NoiseType::Value:
					s_f_noise_2D_ptr = &FastNoise::GetValue2;
					break;
				case Syn::NoiseType::Simplex:
					s_f_noise_2D_ptr = &FastNoise::GetSimplex2;
					break;
				case Syn::NoiseType::Perlin:
					s_f_noise_2D_ptr = &FastNoise::GetPerlin2;
					break;
				case Syn::NoiseType::Cellular:
					s_f_noise_2D_ptr = &FastNoise::GetCellular2;
					break;
				case Syn::NoiseType::Cubic:
					s_f_noise_2D_ptr = &FastNoise::GetCubic2;
					break;
				default:
					SYN_CORE_FATAL_ERROR("unknown Syn::NoiseType: ", (int)_type);
					break;
			}
		}
		static void set_noise_type_3D(NoiseType _type=NoiseType::Value)
		{
			switch (_type)
			{
				case Syn::NoiseType::Value:
					s_f_noise_3D_ptr = &FastNoise::GetValue3;
					break;
				case Syn::NoiseType::Simplex:
					s_f_noise_3D_ptr = &FastNoise::GetSimplex3;
					break;
				case Syn::NoiseType::Perlin:
					s_f_noise_3D_ptr = &FastNoise::GetPerlin3;
					break;
				case Syn::NoiseType::Cellular:
					s_f_noise_3D_ptr = &FastNoise::GetCellular3;
					break;
				case Syn::NoiseType::Cubic:
					s_f_noise_3D_ptr = &FastNoise::GetCubic3;
					break;
				default:
					SYN_CORE_FATAL_ERROR("unknown Syn::NoiseType: ", (int)_type);
					break;
			}
		}
		static void set_noise_frequency(float _frequency) { s_f_noise.SetFrequency(_frequency); }


		// fractional noise with partial derivatives
		static glm::vec3 fbm_perlin2_d(const glm::vec2& _v);
		static glm::vec4 fbm_perlin3_d(const glm::vec3& _v);
		static glm::vec3 fbm_value2_d(const glm::vec2& _v);
		static glm::vec4 fbm_value3_d(const glm::vec3& _v);


	private:
		static std::unique_ptr<NoisePerlin2> s_perlin2;
		static std::unique_ptr<NoisePerlin3> s_perlin3;
		static std::unique_ptr<NoiseValue2> s_value2;
		static std::unique_ptr<NoiseValue3> s_value3;
		
		// static rotation matrix for 2D noise
		static glm::mat2 s_rotation_mat2;
		static glm::mat3 s_rotation_mat3;

		// noise paramters
		static NoiseParameters s_noise_param;
	};



	// random generator for incidental use
	// N.B.!! --> replaced by Synapse/Utils/Random/Random.hpp
	// static
	/*
	class Random
	{
	public:
		static void set_seed(uint32_t _seed) { srand(_seed); }
		static int rand_int(int _low=0, int _high=std::numeric_limits<int>::max()) { return (rand() % (_high - _low)) + _low; }
		static float rand_float(float _low = 0.0f, float _high = std::numeric_limits<float>::max()) { return (static_cast<float>(rand()) / (static_cast<float>(INT_MAX) / (_high - _low))) + _low; }

		static char* get_random_string(size_t len)
		{
			static char buffer[256];
			memset(buffer, 0, 256);
			SYN_CORE_ASSERT(len < 256, "cannot handle >255 bytes.");

			static const char alphanum[] =
				"0123456789"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz";
			static size_t sz = sizeof(alphanum) - 1;
			
			for (uint32_t i = 0; i < len; i++)
				buffer[i] = alphanum[rand() % sz];

			return buffer;
		}

	};
	*/
}
