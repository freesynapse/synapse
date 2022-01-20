
#pragma once

#include <random>
#include <functional>
#include <glm/glm.hpp>
#include <stdio.h>


namespace Syn
{
    class VxValue
    {
    private:
        inline int fast_floor(float x) { return x > 0 ? (int)x : (int)x - 1; }
        inline float cubic_step(float t) { return t * t * (3.0f - 2.0f * t); }
        inline float lerp(float a, float b, float t) { return a + t * (b - a); }

        static const int r_sz = 256;
        static const int r_mask = r_sz - 1;
        float r[r_sz];
        uint32_t p[r_sz * 2];

        glm::mat2 mrot2 = glm::mat2(0.8, 0.6, -0.6, 0.8);

        uint8_t m_octaves = 5;
        float m_frequency  = 1.0f;
        float m_dFrequency = 2.0f;
        float m_dAmplitude = 0.5f;

    public:
        /* default = 5 */
        void set_octave_count(uint8_t o) { m_octaves = o; }
        /* default = 1.0f */
        void set_frequency(float f) { m_frequency = f; }
        /* default = 2.0f */
        void set_d_frequency(float df) { m_dFrequency = df; }
        /* default = 0.5f */
        void set_d_amplitude(float da) { m_dAmplitude = da; }

    public:
        VxValue(uint32_t seed=211017)
        {
            std::mt19937 gen(seed);
            std::uniform_real_distribution<float> distFloat;
            auto rfloat = std::bind(distFloat, gen);
            for (uint32_t i = 0; i < r_sz; i++)
            {
                r[i] = rfloat();
                p[i] = i;
            }

            std::uniform_int_distribution<uint> distUInt;
            auto r_uint = std::bind(distUInt, gen);
            for (uint32_t i = 0; i < r_sz; i++)
            {
                uint32_t j = r_uint() & r_mask;
                std::swap(p[i], p[j]);
                p[i + r_sz] = p[i];
            }
        }
        //-------------------------------------------------------------------------------
        float fbm2(const glm::vec2& p) { return fbm2(p.x, p.y); }
        //-------------------------------------------------------------------------------
        inline float fbm2(float x, float y)
        {
            float  n = 0.0f;    // accumulated noise
            float  f = m_frequency;
            float df = m_dFrequency;
            float  a = 1.0f;
            float da = m_dAmplitude;
            float  t = 0.0f;    // total amp, for normalizing to [0..1]

            for (uint8_t i = 0; i < m_octaves; i++)
            {
                n += a * eval2(x * f, y * f);
                t += a;
                a *= da;
                f *= df;
            }
            return n / t;
        }
        //-------------------------------------------------------------------------------
        float fbm2_r(float x, float y) { return fbm2_r(glm::vec2(x, y)); }
        //-------------------------------------------------------------------------------
        inline float fbm2_r(const glm::vec2& p)
        {
            glm::vec2 p_ = p;
            float  n = 0.0f;    // accumulated noise
            float  f = m_frequency;
            float df = m_dFrequency;
            float  a = 1.0f;
            float  t = 0.0f;    // total amp, for normalizing to [0..1]
            float da = m_dAmplitude;

            for (uint8_t i = 0; i < m_octaves; i++)
            {
                n += a * eval2(p_ * f);
                p_ = mrot2 * p_;
                t += a;
                a *= da;
                f *= df;
            }
            return n / t;
        }
        //-------------------------------------------------------------------------------
        float eval2(const glm::vec2& p) { return eval2(p.x, p.y); }
        //-------------------------------------------------------------------------------
        inline float eval2(float x, float y)
        {
            int xi = fast_floor(x);
            int yi = fast_floor(y);
            
            float xf = x - xi;
            float yf = y - yi;

            int rx0 = xi & r_mask;
            int ry0 = yi & r_mask;
            int rx1 = (rx0 + 1) & r_mask;
            int ry1 = (ry0 + 1) & r_mask;

            const float& c00 = r[p[p[rx0] + ry0]];
            const float& c10 = r[p[p[rx1] + ry0]];
            const float& c01 = r[p[p[rx0] + ry1]];
            const float& c11 = r[p[p[rx1] + ry1]];

            float sx = cubic_step(xf);
            float sy = cubic_step(yf);

            float nx0 = lerp(c00, c10, sx);
            float nx1 = lerp(c01, c11, sx);

            return lerp(nx0, nx1, sy);
        }
        //-------------------------------------------------------------------------------

    };


    class VxPerlin
    {
    private:
        inline int fast_floor(float x) { return x > 0 ? (int)x : (int)x - 1; }
        inline float cubic_step(float t) { return t * t * (3.0f - 2.0f * t); }
        inline float quintic_step(float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }
        inline float lerp(float a, float b, float t) { return a + t * (b - a); }
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

        int m_p[512];   // the actual permutation table
        int permutation[256] = 
        { 151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,
          21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,
          237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,
          83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,
          216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,
          173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,
          47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,
          167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,
          251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,214,
          31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,
          29,24,72,243,141,128,195,78,66,215,61,156,180
        };        

        uint8_t m_octaves = 5;
        float m_frequency  = 1.0f;
        float m_dFrequency = 2.0f;
        float m_dAmplitude = 0.5f;

    public:
        /* default = 5 */
        void set_octave_count(uint8_t o) { m_octaves = o; }
        /* default = 1.0f */
        void set_frequency(float f) { m_frequency = f; }
        /* default = 2.0f */
        void set_d_frequency(float df) { m_dFrequency = df; }
        /* default = 0.5f */
        void set_d_amplitude(float da) { m_dAmplitude = da; }

    public:
        VxPerlin()
        {
            for (int i = 0; i < 512; i++)
                m_p[i] = permutation[i % 256];
        }
        //-------------------------------------------------------------------------------
        float fbm2(const glm::vec2& p) { return fbm2(p.x, p.y); }
        //-------------------------------------------------------------------------------
        inline float fbm2(float x, float y)
        {
            float  n = 0.0f;    // accumulated noise
            float  t = 0.0f;    // total amp, for normalizing [0..1]
            float  f = m_frequency;
            float df = m_dFrequency;
            float  a = 1.0f;
            float da = m_dAmplitude;

            for (uint8_t i = 0; i < m_octaves; i++)
            {
                n += a * eval2(x * f, y * f);
                t += a;
                a *= da;
                f *= df;
            }
            return n / t;            
        }
        //-------------------------------------------------------------------------------
        float eval2(const glm::vec2& p) { return eval2(p.x, p.y); }
        //-------------------------------------------------------------------------------
        float eval2_non_norm(const glm::vec2& p) { return (eval2(p.x, p.y) * 2.0f) - 1.0f; }
        //-------------------------------------------------------------------------------
        float eval2_non_norm(float x, float y) { return (eval2(x, y) * 2.0f) - 1.0f; }
        //-------------------------------------------------------------------------------
        inline float eval2(float x, float y)
        {
            int xi = (int)x & 255;
            int yi = (int)y & 255;

            float xf = x - (int)x;
            float yf = y - (int)y;

            float u = quintic_step(xf);
            float v = quintic_step(yf);

            int g00, g01, g10, g11;
            g00 = m_p[m_p[  xi]+  yi];
            g10 = m_p[m_p[1+xi]+  yi];
            g01 = m_p[m_p[  xi]+1+yi];
            g11 = m_p[m_p[1+xi]+1+yi];

            float x1, x2;
            x1 = lerp(grad2(g00, xf, yf  ), grad2(g10, xf-1, yf  ), u);
            x2 = lerp(grad2(g01, xf, yf-1), grad2(g11, xf-1, yf-1), u);
            return (lerp(x1, x2, v) + 1.0f) * 0.5f;
        }
        //-------------------------------------------------------------------------------
        float fbm3(const glm::vec3& p) { return fbm3(p.x, p.y, p.z); }
        //-------------------------------------------------------------------------------
        inline float fbm3(float x, float y, float z)
        {
            float  n = 0.0f;    // accumulated noise
            float  t = 0.0f;    // total amp, for normalizing [0..1]
            float  f = m_frequency;
            float df = m_dFrequency;
            float  a = 1.0f;
            float da = m_dAmplitude;

            for (uint8_t i = 0; i < m_octaves; i++)
            {
                n += a * eval3(x*f, y*f, z*f);
                t += a;
                a *= da;
                f *= df;
            }
            return n / t;
        }
        //-------------------------------------------------------------------------------
        float eval3(const glm::vec3& p) { return eval3(p.x, p.y, p.z); }
        //-------------------------------------------------------------------------------
        float eval3_non_norm(const glm::vec3& p) { return (eval3(p.x, p.y, p.z) * 2.0f) - 1.0f; }
        //-------------------------------------------------------------------------------
        inline float eval3(float x, float y, float z)
        {
            int xi = (int)x & 255;
            int yi = (int)y & 255;
            int zi = (int)z & 255;

            float xf = x - (int)x;
            float yf = y - (int)y;
            float zf = z - (int)z;

            float u = quintic_step(xf);
            float v = quintic_step(yf);
            float w = quintic_step(zf);

            int g000, g010, g001, g011, g100, g110, g101, g111;
    		g000 = m_p[m_p[m_p[  xi]+  yi]+  zi];
    		g010 = m_p[m_p[m_p[  xi]+1+yi]+  zi];
    		g001 = m_p[m_p[m_p[  xi]+  yi]+1+zi];
    		g011 = m_p[m_p[m_p[  xi]+1+yi]+1+zi];
    		g100 = m_p[m_p[m_p[1+xi]+  yi]+  zi];
    		g110 = m_p[m_p[m_p[1+xi]+1+yi]+  zi];
    		g101 = m_p[m_p[m_p[1+xi]+  yi]+1+zi];
    		g111 = m_p[m_p[m_p[1+xi]+1+yi]+1+zi];

            float x1, x2, y1, y2;
            x1 = lerp(grad3(g000, xf  , yf  , zf  ), grad3(g100, xf-1, yf  , zf  ), u);
            x2 = lerp(grad3(g010, xf  , yf-1, zf  ), grad3(g110, xf-1, yf-1, zf  ), u);
            y1 = lerp(x1, x2, v);
            x1 = lerp(grad3(g001, xf  , yf  , zf-1), grad3(g101, xf-1, yf  , zf-1), u);
            x2 = lerp(grad3(g011, xf  , yf-1, zf-1), grad3(g111, xf-1, yf-1, zf-1), u);
            y2 = lerp(x1, x2, v);

            return (lerp(y1, y2, w) + 1.0f) * 0.5f;
        }
    
    };


}