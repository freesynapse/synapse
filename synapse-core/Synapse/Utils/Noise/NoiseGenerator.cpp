
#include "pch.hpp"

#include "Synapse/Utils/Noise/NoiseGenerator.hpp"


namespace Syn {


	//-----------------------------------------------------------------------------------
	// Perlin 1D
	//-----------------------------------------------------------------------------------

	NoisePerlin1::NoisePerlin1(uint32_t _seed)
	{
		this->initPerlinNoise(_seed);
	}
	//-----------------------------------------------------------------------------------
	float NoisePerlin1::eval(const float& _p, float _f)
	{
		return 0;
	}
	//-----------------------------------------------------------------------------------
	float NoisePerlin1::eval_d(const float& _p, float& _dn, float _f)
	{
		return 0;
	}

	
	//-----------------------------------------------------------------------------------
	// Perlin 2D
	//-----------------------------------------------------------------------------------
	
	NoisePerlin2::NoisePerlin2(uint32_t _seed)
	{
		this->initPerlinNoise(_seed);
	}
	//-----------------------------------------------------------------------------------
	float NoisePerlin2::eval(const glm::vec2& _p, float _f)
	{
		glm::vec2 p = _p * _f;

		int xf = (int)std::floor(p.x);
		int yf = (int)std::floor(p.y);

		int xi0 = xf & m_tableMask;
		int yi0 = yf & m_tableMask;

		int xi1 = (xi0 + 1) & m_tableMask;
		int yi1 = (yi0 + 1) & m_tableMask;

		float tx0 = p.x - xf;
		float ty0 = p.y - yf;

		float tx = Noise::perlinStep(tx0);
		float ty = Noise::perlinStep(ty0);

		// vectors going from the grid points to p
		float x0 = tx, x1 = tx - 1;
		float y0 = ty, y1 = ty - 1;

		// gradients at grid corners
		glm::vec2 g00 = glm::vec2(m_gradients[hash2(xi0, yi0)]);
		glm::vec2 g10 = glm::vec2(m_gradients[hash2(xi1, yi0)]);
		glm::vec2 g01 = glm::vec2(m_gradients[hash2(xi0, yi1)]);
		glm::vec2 g11 = glm::vec2(m_gradients[hash2(xi1, yi1)]);

		// gradients at grid corners
		glm::vec2 p00 = glm::vec2(x0, y0);
		glm::vec2 p10 = glm::vec2(x1, y0);
		glm::vec2 p01 = glm::vec2(x0, y1);
		glm::vec2 p11 = glm::vec2(x1, y1);

		// dot products (projection)
		float a0 = glm::dot(g00, p00);
		float b0 = glm::dot(g10, p10);
		float c0 = glm::dot(g01, p01);
		float d0 = glm::dot(g11, p11);

		// constants
		float a = a0;
		float b = (b0 - a0);
		float c = (c0 - a0);
		float d = a0 + d0 - b0 - c0;

		// the noise result, lerp:ed (but expanded)
		//return a0 + (b0 - a0) * tx + (c0 - a0) * ty + (a0 - b0 - c0 + d0) * tx * ty;
		return a + b * tx + c * ty + d * tx * ty;
		//return a + b * tx + (c + d * tx) * ty;
	}
	//-----------------------------------------------------------------------------------
	float NoisePerlin2::eval_d(const glm::vec2& _p, glm::vec2& _dn, float _f)
	{
		glm::vec2 p = _p * _f;

		int xf = (int)std::floor(p.x);
		int yf = (int)std::floor(p.y);

		int xi0 = xf & m_tableMask;
		int yi0 = yf & m_tableMask;

		int xi1 = (xi0 + 1) & m_tableMask;
		int yi1 = (yi0 + 1) & m_tableMask;

		float tx0 = p.x - xf;
		float ty0 = p.y - yf;

		float tx = Noise::perlinStep(tx0);
		float ty = Noise::perlinStep(ty0);

		float dtx = Noise::perlinStep_d(tx0);
		float dty = Noise::perlinStep_d(ty0);

		// vectors going from the grid points to p
		float x0 = tx, x1 = tx - 1;
		float y0 = ty, y1 = ty - 1;

		// gradients at grid corners
		glm::vec2 g00 = glm::vec2(m_gradients[hash2(xi0, yi0)]);
		glm::vec2 g10 = glm::vec2(m_gradients[hash2(xi1, yi0)]);
		glm::vec2 g01 = glm::vec2(m_gradients[hash2(xi0, yi1)]);
		glm::vec2 g11 = glm::vec2(m_gradients[hash2(xi1, yi1)]);

		// gradients at grid corners
		glm::vec2 p00 = glm::vec2(x0, y0);
		glm::vec2 p10 = glm::vec2(x1, y0);
		glm::vec2 p01 = glm::vec2(x0, y1);
		glm::vec2 p11 = glm::vec2(x1, y1);

		// dot products (projection)
		float a0 = glm::dot(g00, p00);
		float b0 = glm::dot(g10, p10);
		float c0 = glm::dot(g01, p01);
		float d0 = glm::dot(g11, p11);

		// constants
		float a = a0;
		float b = (b0 - a0);
		float c = (c0 - a0);
		float d = a0 + d0 - b0 - c0;

		glm::vec2 da = g00;
		glm::vec2 db = g10 - g00;
		glm::vec2 dc = g01 - g00;
		glm::vec2 dd = g00 + g11 - g01 - g10;

		// compute partial derivatives
		//
		// derivative of the interpolation
		glm::vec2 dn = da + db * tx + (dc + dd * tx) * ty;
		// partial addons
		dn.x += (b + d * ty) * dtx;
		dn.y += (c + d * tx) * dty;
		dn *= _f;
		// store as out reference
		_dn = dn;

		// the noise result, lerp:ed (but expanded)
		return a + b * tx + (c + d * tx) * ty;
	}


	//-----------------------------------------------------------------------------------
	// Perlin 3D
	//-----------------------------------------------------------------------------------
	
	NoisePerlin3::NoisePerlin3(uint32_t _seed)
	{ 
		this->initPerlinNoise(_seed); 
	}
	//-----------------------------------------------------------------------------------
	float NoisePerlin3::eval(const glm::vec3& _p, float _f)
	{
		glm::vec3 p = _p * _f;

		int xf = (int)std::floor(p.x);
		int yf = (int)std::floor(p.y);
		int zf = (int)std::floor(p.z);

		int xi0 = xf & m_tableMask;
		int yi0 = yf & m_tableMask;
		int zi0 = zf & m_tableMask;

		int xi1 = (xi0 + 1) & m_tableMask;
		int yi1 = (yi0 + 1) & m_tableMask;
		int zi1 = (zi0 + 1) & m_tableMask;

		float tx0 = p.x - xf;
		float ty0 = p.y - yf;
		float tz0 = p.z - zf;

		float tx = Noise::perlinStep(tx0);
		float ty = Noise::perlinStep(ty0);
		float tz = Noise::perlinStep(tz0);

		// vectors going from the grid points to p
		float x0 = tx, x1 = tx - 1;
		float y0 = ty, y1 = ty - 1;
		float z0 = tz, z1 = tz - 1;

		/* INSERT */
		// gradients at grid corners
		glm::vec3 g000 = m_gradients[hash3(xi0, yi0, zi0)];
		glm::vec3 g100 = m_gradients[hash3(xi1, yi0, zi0)];
		glm::vec3 g010 = m_gradients[hash3(xi0, yi1, zi0)];
		glm::vec3 g110 = m_gradients[hash3(xi1, yi1, zi0)];
		glm::vec3 g001 = m_gradients[hash3(xi0, yi0, zi1)];
		glm::vec3 g101 = m_gradients[hash3(xi1, yi0, zi1)];
		glm::vec3 g011 = m_gradients[hash3(xi0, yi1, zi1)];
		glm::vec3 g111 = m_gradients[hash3(xi1, yi1, zi1)];

		// gradients at grid corners
		glm::vec3 p000 = glm::vec3(x0, y0, z0);
		glm::vec3 p100 = glm::vec3(x1, y0, z0);
		glm::vec3 p010 = glm::vec3(x0, y1, z0);
		glm::vec3 p110 = glm::vec3(x1, y1, z0);
		glm::vec3 p001 = glm::vec3(x0, y0, z1);
		glm::vec3 p101 = glm::vec3(x1, y0, z1);
		glm::vec3 p011 = glm::vec3(x0, y1, z1);
		glm::vec3 p111 = glm::vec3(x1, y1, z1);

		// dot products (projection)
		float a0 = glm::dot(g000, p000);
		float b0 = glm::dot(g100, p100);
		float c0 = glm::dot(g010, p010);
		float d0 = glm::dot(g110, p110);
		float e0 = glm::dot(g001, p001);
		float f0 = glm::dot(g101, p101);
		float g0 = glm::dot(g011, p011);
		float h0 = glm::dot(g111, p111);

		// constants
		float a = a0;
		float b = (b0 - a0);
		float c = (c0 - a0);
		float d = (e0 - a0);
		float e = (a0 + d0 - b0 - c0);
		float f = (a0 + f0 - b0 - e0);
		float g = (a0 + g0 - c0 - e0);
		float h = (b0 + c0 + e0 + h0 - a0 - d0 - f0 - g0);
		/*INSERT */

		// constants
		//float k0 = a;
		//float k1 = (b - a);
		//float k2 = (c - a);
		//float k3 = (e - a);
		//float k4 = (a + d - b - c);
		//float k5 = (a + f - b - e);
		//float k6 = (a + g - c - e);
		//float k7 = (b + c + e + h - a - d - f - g);

		// the noise result, lerp:ed (but expanded)
		//float t = k0 + k1 * tx + k2 * ty + k3 * tz + k4 * tx * ty + k5 * tx * tz + k6 * ty * tz + k7 * tx * ty * tz;
		//float t = k0 + k1 * u + k2 * v + k3 * w + k4 * u * v + k5 * u * w + k6 * v * w + k7 * u * v * w;
		float t =  a + b * tx + (c + e * tx) * ty + (d + f * tx + (g + h * tx) * ty) * tz;
		//printf("%.1f ", t);
		return t;

	}
	//-----------------------------------------------------------------------------------
	float NoisePerlin3::eval_d(const glm::vec3& _p, glm::vec3& _dn, float _f)
	{
		glm::vec3 p = _p * _f;

		// grid boundaries
		int xf = static_cast<int>(std::floor(p.x));
		int yf = static_cast<int>(std::floor(p.y));
		int zf = static_cast<int>(std::floor(p.z));

		int xi0 = xf & m_tableMask;
		int yi0 = yf & m_tableMask;
		int zi0 = zf & m_tableMask;

		int xi1 = (xi0 + 1) & m_tableMask;
		int yi1 = (yi0 + 1) & m_tableMask;
		int zi1 = (zi0 + 1) & m_tableMask;

		// interpolation step
		float tx0 = p.x - xf;
		float ty0 = p.y - yf;
		float tz0 = p.z - zf;

		// smooth the interpolation step (quintic)
		float tx = Noise::perlinStep(tx0);
		float ty = Noise::perlinStep(ty0);
		float tz = Noise::perlinStep(tz0);

		// paritial derivatives
		float dtx = Noise::perlinStep_d(tx0);
		float dty = Noise::perlinStep_d(ty0);
		float dtz = Noise::perlinStep_d(tz0);

		// vectors from grid points to _v
		float x0 = tx, x1 = tx - 1;
		float y0 = ty, y1 = ty - 1;
		float z0 = tz, z1 = tz - 1;

		// gradients at grid corners
		glm::vec3 g000 = m_gradients[hash3(xi0, yi0, zi0)];
		glm::vec3 g100 = m_gradients[hash3(xi1, yi0, zi0)];
		glm::vec3 g010 = m_gradients[hash3(xi0, yi1, zi0)];
		glm::vec3 g110 = m_gradients[hash3(xi1, yi1, zi0)];
		glm::vec3 g001 = m_gradients[hash3(xi0, yi0, zi1)];
		glm::vec3 g101 = m_gradients[hash3(xi1, yi0, zi1)];
		glm::vec3 g011 = m_gradients[hash3(xi0, yi1, zi1)];
		glm::vec3 g111 = m_gradients[hash3(xi1, yi1, zi1)];

		// positions at grid corners
		glm::vec3 p000 = glm::vec3(x0, y0, z0);
		glm::vec3 p100 = glm::vec3(x1, y0, z0);
		glm::vec3 p010 = glm::vec3(x0, y1, z0);
		glm::vec3 p110 = glm::vec3(x1, y1, z0);
		glm::vec3 p001 = glm::vec3(x0, y0, z1);
		glm::vec3 p101 = glm::vec3(x1, y0, z1);
		glm::vec3 p011 = glm::vec3(x0, y1, z1);
		glm::vec3 p111 = glm::vec3(x1, y1, z1);

		// dot products (projection)
		float a0 = glm::dot(g000, p000);
		float b0 = glm::dot(g100, p100);
		float c0 = glm::dot(g010, p010);
		float d0 = glm::dot(g110, p110);
		float e0 = glm::dot(g001, p001);
		float f0 = glm::dot(g101, p101);
		float g0 = glm::dot(g011, p011);
		float h0 = glm::dot(g111, p111);

		// constants
		float a = a0;
		float b = (b0 - a0);
		float c = (c0 - a0);
		float d = (e0 - a0);
		float e = (a0 + d0 - b0 - c0);
		float f = (a0 + f0 - b0 - e0);
		float g = (a0 + g0 - c0 - e0);
		float h = (b0 + c0 + e0 + h0 - a0 - d0 - f0 - g0);

		glm::vec3 da = g000;
		glm::vec3 db = g100 - g000;
		glm::vec3 dc = g010 - g000;
		glm::vec3 dd = g001 - g000;
		glm::vec3 de = g110 - g010 - g100 + g000;
		glm::vec3 df = g101 - g001 - g100 + g000;
		glm::vec3 dg = g011 - g001 - g010 + g000;
		glm::vec3 dh = g111 - g011 - g101 + g001 - g110 + g010 + g100 - g000;

		// calculation of the partial derivatives (...)
		glm::vec3 dn = da + db * tx + (dc + de * tx) * ty + (dd + df * tx + (dg + dh * tx) * ty) * tz;
		dn.x += (b + e * ty + (f + h * ty) * tz) * dtx;
		dn.y += (c + e * tx + (g + h * tx) * tz) * dty;
		dn.z += (d + f * tx + (g + h * tx) * ty) * dtz;
		dn *= _f;
		_dn = dn;

		// return noise evaluated at _p
		return a + b * tx + (c + e * tx) * ty + (d + f * tx + (g + h * tx) * ty) * tz;
	}


	//-----------------------------------------------------------------------------------
	// Value 1D
	//-----------------------------------------------------------------------------------
	
	NoiseValue1::NoiseValue1(uint32_t _seed)
	{
		initValueNoise(_seed);
	}
	//-----------------------------------------------------------------------------------
	float NoiseValue1::eval(const float& _p, float _f)
	{
		// multiply by frequency
		float p = _p * _f;
		// floor
		int xi = (int)p - (p < 0 && p != (int)p);
		float tx0 = p - xi;
		// mod using bitwise AND
		int x0 = xi & m_tableMask;
		int x1 = (x0 + 1) & m_tableMask;

		float h0 = m_values[hash1(x0)];
		float h1 = m_values[hash1(x1)];

		// lerp smooth stepped value
		float tx = Noise::smoothStep(tx0);

		return h0 + (h1 - h0) * tx;
	}
	//-----------------------------------------------------------------------------------
	float NoiseValue1::eval_d(const float& _p, float& _dp, float _f)
	{
		// multiply by frequency
		float p = _p * _f;
		// floor
		int xi = (int)p - (p < 0 && p != (int)p);
		float tx0 = p - xi;
		// mod using bitwise AND
		int x0 = xi & m_tableMask;
		int x1 = (x0 + 1) & m_tableMask;

		float h0 = m_values[hash1(x0)];
		float h1 = m_values[hash1(x1)];

		float a = h0;
		float b = h1 - h0;

		// lerp smooth stepped value
		float tx = Noise::smoothStep(tx0);
		float dtx = Noise::smoothStep_d(tx0);

		// derivative (in x)
		_dp = b * dtx;
		_dp *= _f;

		// linearly interpolated noise value
		return a + b * tx;
	}


	//-----------------------------------------------------------------------------------
	// Value 2D
	//-----------------------------------------------------------------------------------

	NoiseValue2::NoiseValue2(uint32_t _seed)
	{
		initValueNoise(_seed);
	}
	//-----------------------------------------------------------------------------------
	float NoiseValue2::eval(const glm::vec2& _p, float _f)
	{
		// multiply by frequency
		glm::vec2 p = _p * _f;

		// floor
		int xi = (int)p.x - (p.x < 0 && p.x != (int)p.x);
		int yi = (int)p.y - (p.y < 0 && p.y != (int)p.y);

		// interpolator
		float tx0 = p.x - xi;
		float ty0 = p.y - yi;

		// mod using bitwise AND
		int x0 = xi & m_tableMask;
		int y0 = yi & m_tableMask;
		int x1 = (x0 + 1) & m_tableMask;
		int y1 = (y0 + 1) & m_tableMask;

		// lerp smooth stepped value
		float tx = Noise::smoothStep(tx0);
		float ty = Noise::smoothStep(ty0);

		// hashed random numbers
		float h00 = m_values[hash2(x0, y0)];
		float h10 = m_values[hash2(x1, y0)];
		float h01 = m_values[hash2(x0, y1)];
		float h11 = m_values[hash2(x1, y1)];

		// constants
		float a = h00;
		float b = h10 - h00;
		float c = h01 - h00;
		float d = h11 - h01 - h10 + h00;

		return a + b * tx + (c + d * tx) * ty;
	}
	//-----------------------------------------------------------------------------------
	float NoiseValue2::eval_d(const glm::vec2& _p, glm::vec2& _dp, float _f)
	{
		// multiply by frequency
		glm::vec2 p = _p * _f;

		// floor
		int xi = (int)p.x - (p.x < 0 && p.x != (int)p.x);
		int yi = (int)p.y - (p.y < 0 && p.y != (int)p.y);

		// interpolator
		float tx0 = p.x - xi;
		float ty0 = p.y - yi;

		// mod using bitwise AND
		int x0 = xi & m_tableMask;
		int y0 = yi & m_tableMask;
		int x1 = (x0 + 1) & m_tableMask;
		int y1 = (y0 + 1) & m_tableMask;

		// lerp smooth stepped value
		float tx = Noise::smoothStep(tx0);
		float ty = Noise::smoothStep(ty0);
		float dtx = Noise::smoothStep_d(tx0);
		float dty = Noise::smoothStep_d(ty0);

		// hashed random numbers
		float h00 = m_values[hash2(x0, y0)];
		float h10 = m_values[hash2(x1, y0)];
		float h01 = m_values[hash2(x0, y1)];
		float h11 = m_values[hash2(x1, y1)];

		// constants
		float a = h00;
		float b = h10 - h00;
		float c = h01 - h00;
		float d = h11 - h01 - h10 + h00;

		// compute partial derivatives
		_dp.x = (b + d * ty) * dtx;
		_dp.y = (c + d * tx) * dty;
		_dp *= _f;

		// return noise value
		return a + b * tx + (c + d * tx) * ty;
	}


	//-----------------------------------------------------------------------------------
	// Value 3D
	//-----------------------------------------------------------------------------------
	
	NoiseValue3::NoiseValue3(uint32_t _seed)
	{
		initValueNoise(_seed);
	}
	//-----------------------------------------------------------------------------------
	float NoiseValue3::eval(const glm::vec3& _p, float _f)
	{
		// multiply by frequency
		glm::vec3 p = _p * _f;

		// floor
		int xi = (int)p.x - (p.x < 0 && p.x != (int)p.x);
		int yi = (int)p.y - (p.y < 0 && p.y != (int)p.y);
		int zi = (int)p.z - (p.z < 0 && p.z != (int)p.z);

		// interpolator
		float tx0 = p.x - xi;
		float ty0 = p.y - yi;
		float tz0 = p.z - zi;

		// mod using bitwise AND
		int x0 = xi & m_tableMask;
		int y0 = yi & m_tableMask;
		int z0 = zi & m_tableMask;
		int x1 = (x0 + 1) & m_tableMask;
		int y1 = (y0 + 1) & m_tableMask;
		int z1 = (z0 + 1) & m_tableMask;

		// lerp smooth stepped value
		float tx = Noise::smoothStep(tx0);
		float ty = Noise::smoothStep(ty0);
		float tz = Noise::smoothStep(tz0);

		// hashed random numbers
		float h000 = m_values[hash3(x0, y0, z0)];
		float h100 = m_values[hash3(x1, y0, z0)];
		float h010 = m_values[hash3(x0, y1, z0)];
		float h110 = m_values[hash3(x1, y1, z0)];
		float h001 = m_values[hash3(x0, y0, z1)];
		float h101 = m_values[hash3(x1, y0, z1)];
		float h011 = m_values[hash3(x0, y1, z1)];
		float h111 = m_values[hash3(x1, y1, z1)];

		// constants
		float a = h000;
		float b = h100 - h000;
		float c = h010 - h000;
		float d = h001 - h000;
		float e = h110 - h010 - h100 + h000;
		float f = h101 - h001 - h100 + h000;
		float g = h011 - h001 - h010 + h000;
		float h = h111 - h011 - h101 + h001 - h110 + h010 + h100 - h000;

		return a + b * tx + (c + e * tx) * ty + (d + f * tx + (g + h * tx) * ty) * tz;
	}
	//-----------------------------------------------------------------------------------
	float NoiseValue3::eval_d(const glm::vec3& _p, glm::vec3& _dp, float _f)
	{
		// multiply by frequency
		glm::vec3 p = _p * _f;

		// floor
		int xi = (int)p.x - (p.x < 0 && p.x != (int)p.x);
		int yi = (int)p.y - (p.y < 0 && p.y != (int)p.y);
		int zi = (int)p.z - (p.z < 0 && p.z != (int)p.z);

		// interpolator
		float tx0 = p.x - xi;
		float ty0 = p.y - yi;
		float tz0 = p.z - zi;

		// mod using bitwise AND
		int x0 = xi & m_tableMask;
		int y0 = yi & m_tableMask;
		int z0 = zi & m_tableMask;
		int x1 = (x0 + 1) & m_tableMask;
		int y1 = (y0 + 1) & m_tableMask;
		int z1 = (z0 + 1) & m_tableMask;

		// lerp smooth stepped value
		float tx = Noise::smoothStep(tx0);
		float ty = Noise::smoothStep(ty0);
		float tz = Noise::smoothStep(tz0);

		float dtx = Noise::smoothStep_d(tx0);
		float dty = Noise::smoothStep_d(ty0);
		float dtz = Noise::smoothStep_d(tz0);

		// hashed random numbers
		float h000 = m_values[hash3(x0, y0, z0)];
		float h100 = m_values[hash3(x1, y0, z0)];
		float h010 = m_values[hash3(x0, y1, z0)];
		float h110 = m_values[hash3(x1, y1, z0)];
		float h001 = m_values[hash3(x0, y0, z1)];
		float h101 = m_values[hash3(x1, y0, z1)];
		float h011 = m_values[hash3(x0, y1, z1)];
		float h111 = m_values[hash3(x1, y1, z1)];

		// constants
		float a = h000;
		float b = h100 - h000;
		float c = h010 - h000;
		float d = h001 - h000;
		float e = h110 - h010 - h100 + h000;
		float f = h101 - h001 - h100 + h000;
		float g = h011 - h001 - h010 + h000;
		float h = h111 - h011 - h101 + h001 - h110 + h010 + h100 - h000;

		// compute partial derivatives
		_dp.x = (b + e * ty + (f + h * ty) * tz) * dtx;
		_dp.y = (c + e * tx + (g + h * tx) * tz) * dty;
		_dp.z = (d + f * tx + (g + h * tx) * ty) * dtz;
		_dp *= _f;

		return a + b * tx + (c + e * tx) * ty + (d + f * tx + (g + h * tx) * ty) * tz;
	}


}