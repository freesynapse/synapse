#pragma once


#include "Synapse/Utils/Noise/Noise.hpp"


namespace Syn {



	template<typename T>
	class NoiseGenerator
	{
	public:
		virtual ~NoiseGenerator() = default;

		/*
		Evaluation of noise function at coordinate(s) _p.
		Optional frequency multiplier argument (_f).
		*/
		virtual float eval(const T& _p, float _f = 1.0f) = 0;
		/*
		Evaluation of noise function at coordinate(s) _p.
		In addition, partial derivatives are calculated and stored
		in _dp. As in the std eval function, a frequency multiplier
		is as optional argument (_f).
		*/
		virtual float eval_d(const T& _p, T& _dp, float _f = 1.0f) = 0;

	protected:
		void initPerlinNoise(uint32_t _seed)
		{
			std::mt19937 generator(_seed);
			std::uniform_real_distribution<float> dist;
			auto randFloat = std::bind(dist, generator);

			for (uint32_t i = 0; i < m_tableSize; i++)
			{
				float theta = acosf(2.0f * randFloat() - 1.0f);
				float phi = 2.0f * randFloat() * M_PI;

				float x = cosf(phi) * sinf(theta);
				float y = sinf(phi) * sinf(theta);
				float z = cosf(theta);

				m_gradients[i] = glm::vec3(x, y, z);
				m_permutationTable[i] = i;
			}

			// set permutation table entries
			std::uniform_int_distribution<uint32_t> distInt;
			auto randInt = std::bind(distInt, generator);
			for (uint32_t i = 0; i < m_tableSize; i++)
				std::swap(m_permutationTable[i], m_permutationTable[randInt() & m_tableMask]);
			for (uint32_t i = 0; i < m_tableSize; i++)
				m_permutationTable[m_tableSize + i] = m_permutationTable[i];
		}
		// TODO : change from srand to std::uniform_dist<float>
		void initValueNoise(uint32_t _seed)
		{
			srand(_seed);
			memset(m_gradients, 0, sizeof(float) * m_tableSize);
			for (uint32_t i = 0; i < m_tableSize; i++)
			{
				m_values[i] = Noise::frand();
				m_permutationTable[i] = i;
			}
			// set permutation table entries
			std::mt19937 generator(_seed);
			std::uniform_int_distribution<uint32_t> distInt;
			auto randInt = std::bind(distInt, generator);
			for (uint32_t i = 0; i < m_tableSize; i++)
				std::swap(m_permutationTable[i], m_permutationTable[randInt() & m_tableMask]);
			for (uint32_t i = 0; i < m_tableSize; i++)
				m_permutationTable[m_tableSize + i] = m_permutationTable[i];
		}

	protected:
		inline uint32_t hash1(const int& _x) const
		{ return m_permutationTable[_x]; }
		inline uint32_t hash2(const int& _x, const int& _y) const
		{ return m_permutationTable[m_permutationTable[_x] + _y]; }
		inline uint32_t hash3(const int& _x, const int& _y, const int& _z) const
		{ return m_permutationTable[m_permutationTable[m_permutationTable[_x] + _y] + _z]; }

	protected:
		uint32_t m_tableSize = 256;
		uint32_t m_tableMask = m_tableSize - 1;
		glm::vec3 m_gradients[256];
		float m_values[256];
		uint32_t m_permutationTable[512];
	};


	/* 1D Perlin noise */
	class NoisePerlin1 : public NoiseGenerator<float>
	{
	public:
		NoisePerlin1(uint32_t _seed = 191018);
		virtual float eval(const float& _p, float _f) override;
		virtual float eval_d(const float& _p, float& _dn, float _f) override;
	};

	/* 2D Perlin noise */
	class NoisePerlin2 : public NoiseGenerator<glm::vec2>
	{
	public:
		NoisePerlin2(uint32_t _seed=191018);
		virtual float eval(const glm::vec2& _p, float _f) override;
		virtual float eval_d(const glm::vec2& _p, glm::vec2& _dn, float _f) override;
	};


	/* 3D Perlin noise */
	class NoisePerlin3 : public NoiseGenerator<glm::vec3>
	{
	public:
		NoisePerlin3(uint32_t _seed=191010);
		virtual float eval(const glm::vec3& _p, float _f) override;
		virtual float eval_d(const glm::vec3& _p, glm::vec3& _dn, float _f) override;
	private:
		float gradientDotV(uint8_t _perm, float _x, float _y, float _z) const
		{
			switch (_perm & 15)
			{
			case  0:	return  _x + _y; // (1,1,0) 
			case  1:	return -_x + _y; // (-1,1,0) 
			case  2:	return  _x - _y; // (1,-1,0) 
			case  3:	return -_x - _y; // (-1,-1,0) 
			case  4:	return  _x + _z; // (1,0,1) 
			case  5:	return -_x + _z; // (-1,0,1) 
			case  6:	return  _x - _z; // (1,0,-1) 
			case  7:	return -_x - _z; // (-1,0,-1) 
			case  8:	return  _y + _z; // (0,1,1), 
			case  9:	return -_y + _z; // (0,-1,1), 
			case 10:	return  _y - _z; // (0,1,-1), 
			case 11:	return -_y - _z; // (0,-1,-1) 
			case 12:	return  _y + _x; // (1,1,0) 
			case 13:	return -_x + _y; // (-1,1,0) 
			case 14:	return -_y + _z; // (0,-1,1) 
			case 15:	return -_y - _z; // (0,-1,-1) 
			default:    return 0;
			}
		}
	};


	/* 1D value noise */
	class NoiseValue1 : public NoiseGenerator<float>
	{
	public:
		NoiseValue1(uint32_t _seed=1337);
		virtual float eval(const float& _p, float _f) override;
		virtual float eval_d(const float& _p, float& _dp, float _f) override;
	};


	/* 2D value noise */
	class NoiseValue2 : public NoiseGenerator<glm::vec2>
	{
	public:
		NoiseValue2(uint32_t _seed=191018);
		virtual float eval(const glm::vec2& _p, float _f) override;
		virtual float eval_d(const glm::vec2& _p, glm::vec2& _dp, float _f) override;
	};


	/* 3D value noise */
	class NoiseValue3 : public NoiseGenerator<glm::vec3>
	{
	public:
		NoiseValue3(uint32_t _seed = 191018);
		virtual float eval(const glm::vec3& _p, float _f) override;
		virtual float eval_d(const glm::vec3& _p, glm::vec3& _dp, float _f) override;
	};

}


