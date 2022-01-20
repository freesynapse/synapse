
#pragma once

#include <random>
#include <functional>

#include <glm/glm.hpp>

#include "Synapse/Core.hpp"
#include "External/stb_image/stb_image_write.h"


namespace Syn
{
    class VxNoise
    {
    // Member variables
    private:
        // Perlin noise
        static uint32_t s_uint32_p[256];
        static uint32_t s_p_p[512];

        // Value noise
        static uint32_t s_r_sz;
        static uint32_t s_r_mask;
        static float s_r[256];
        static uint32_t s_p[512];


        // General
        static uint8_t s_octave_count;
        static float s_frequency_perlin;
        static float s_frequency_value;
        static float s_d_frequency;
        static float s_d_amplitude;

        static glm::mat2 s_mrot2;
        static glm::mat3 s_mrot3;


    // Private functions
    private:
        /* A faster version of std::floor() */
        inline static int fast_floor(float x)
        { return x > 0 ? (int)x : (int)x - 1; }
        /* Cubic smoothing (corresponding to smoothstep()). */
        inline static float cubic_step(float t)
        { return t * t * (3.0f - 2.0f * t); }
        /* 5:th order smoothing, used in original Perlin noise. */
        inline static float quintic_step(float t)
        { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }
        /* Simple linear interpolation. */
        inline static float lerp(float a, float b, float t)
        { return a + t * (b - a); }
        /* Gradient function for 2D Perlin noise. */
        static inline float grad2(int hash, float x, float y)
        {
            switch(hash & 0x3)
            {
                case 0x0: return  x + y;
                case 0x1: return -x + y;
                case 0x2: return  x - y;
                case 0x3: return -x - y;
                default: return 0;
            }
        }
        /* Gradient function for 3D Perlin noise. */
        static inline float grad3(int hash, float x, float y, float z)
        {
            switch(hash & 0xF)
            {
                case 0x0: return  x + y;
                case 0x1: return -x + y;
                case 0x2: return  x - y;
                case 0x3: return -x - y;
                case 0x4: return  x + z;
                case 0x5: return -x + z;
                case 0x6: return  x - z;
                case 0x7: return -x - z;
                case 0x8: return  y + z;
                case 0x9: return -y + z;
                case 0xA: return  y - z;
                case 0xB: return -y - z;
                case 0xC: return  y + x;
                case 0xD: return -y + z;
                case 0xE: return  y - x;
                case 0xF: return -y - z;
                default: return 0;
            }
        }

    // Accessors
    public:
        /* default = 5 */
        static void set_octave_count(uint8_t o)     { s_octave_count     = o;  }
        /* default = 5.0f */
        static void set_frequency_value(float f)    { s_frequency_value  = f;  }
        /* default = 5.0f */
        static void set_frequency_perlin(float f)   { s_frequency_perlin = f;  }
        /* default = 2.0f */
        static void set_d_frequency(float df)       { s_d_frequency      = df; }
        /* default = 0.5f */
        static void set_d_amplitude(float da)       { s_d_amplitude      = da; }


    // Member functions (in vx_noise.cpp)
    public:
        // Value noise functions
        /* 2D value noise, evaluated at (x, y). */
        static float value2(float x, float y);
        /* 2D fractional value noise, evaluated at (x, y) (5 octaves default). */
        static float value2_fbm(float x, float y);
        /* Rotated 2D fractional value noise, evaluated at p (5 octaves default). */
        static float value2_fbm_r(const glm::vec2& p);
        /* 3D value noise, evaluated at (x, y). */
        static float value3(float x, float y, float z);
        /* 3D fractional value noise, evaluated at (x, y) (5 octaves default). */
        static float value3_fbm(float x, float y, float z);
        /* Rotated 3D fractional value noise, evaluated at p (5 octaves default). */
        static float value3_fbm_r(const glm::vec3& p);

        // Perlin noise functions
        /* 2D Perlin noise, evaluated at (x, y). */
        static float perlin2(float x, float y);
        /* 2D fractional Perlin noise, evaluated at (x, y) (5 octaves default). */
        static float perlin2_fbm(float x, float y);
        /* 3D Perlin noise, evaluated at (x, y, z). */
        static float perlin3(float x, float y, float z);
        /* 3D fractional Perlin noise, evaluated at (x, y, z) (5 octaves default). */
        static float perlin3_fbm(float x, float y, float z);


    // Argument type overloads
    public:
        // Value noise
        /* 2D value noise, evaluated at p. */
        static float value2(const glm::vec2& p)
        { return value2(p.x, p.y); }
        /* 2D fractional value noise, evaluated at p (5 octaves default). */
        static float value2_fbm(const glm::vec2& p)
        { return value2_fbm(p.x, p.y); }
        /* Rotated 2D fractional value noise, evaluated at (x, y) (5 octaves default). */
        static float value2_fbm_r(float x, float y)
        { return value2_fbm_r(glm::vec2(x, y)); }
        /* 3D value noise, evaluated at p. */
        static float value3(const glm::vec3& p)
        { return value3(p.x, p.y, p.z); }
        /* 3D fractional value noise, evaluated at p (5 octaves default). */
        static float value3_fbm(const glm::vec3& p)
        { return value3_fbm(p.x, p.y, p.z); }
        /* Rotated 3D fractional value noise, evaluated at (x, y) (5 octaves default). */
        static float value3_fbm_r(float x, float y, float z)
        { return value3_fbm_r(glm::vec3(x, y, z)); }

        // Perlin noise
        /* 2D Perlin noise, evaluated at p. */
        static float perlin2(const glm::vec2& p)
        { return perlin2(p.x, p.y); }
        /* 2D fractional Perlin noise, evaluated at p (5 octaves default). */
        static float perlin2_fbm(const glm::vec2& p)
        { return perlin2_fbm(p.x, p.y); }
        /* 3D Perlin noise, evaluated at p. */
        static float perlin3(const glm::vec3& p)
        { return perlin3(p.x, p.y, p.z); }
        /* 3D fractional Perlin noise, evaluated at p (5 octaves default). */
        static float perlin3_fbm(const glm::vec3& p)
        { return perlin3_fbm(p.x, p.y, p.z); }


    // Initializer
    public:
        /* Initializer : must be called before use. */
        static void init(uint32_t seed=211117);


    // Utility function(s)
    public:
        /* Save noise as png image. */
        static void save_noise_PNG(const std::string& _file_name, 
                                   uint32_t _width, 
                                   uint32_t _height, 
                                   float* _data);

    };



}