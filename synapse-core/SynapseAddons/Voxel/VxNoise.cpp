
#include "../../pch.hpp"

#include "VxNoise.hpp"


namespace Syn
{
    // Static variable declarations -----------------------------------------------------

    // Perlin noise
    uint32_t    VxNoise::s_uint32_p[256] =
    {
        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,
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
    uint32_t    VxNoise::s_p_p[512];

    // Value noise
    uint32_t    VxNoise::s_r_sz             = 256;
    uint32_t    VxNoise::s_r_mask           = s_r_sz - 1;
    float       VxNoise::s_r[256];
    uint32_t    VxNoise::s_p[512];

    // General
    uint8_t     VxNoise::s_octave_count     = 5;
    float       VxNoise::s_frequency_perlin = 1.0f;
    float       VxNoise::s_frequency_value  = 5.0f;
    float       VxNoise::s_d_frequency      = 2.0f;
    float       VxNoise::s_d_amplitude      = 0.5f;
    glm::mat2   VxNoise::s_mrot2            = { 0.8, 0.6, -0.6, 0.8 };
    glm::mat3   VxNoise::s_mrot3            = { 0.00f,  0.80f,  0.60f, -0.80f,  0.36f, -0.48f, -0.60f, -0.48f,  0.64f };

    // Member functions -----------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    /* 2D value noise, evaluated at (x, y). */
    float VxNoise::value2(float x, float y)
    {
        int xi = fast_floor(x);
        int yi = fast_floor(y);
        
        float xf = x - xi;
        float yf = y - yi;

        int rx0 = xi & s_r_mask;
        int ry0 = yi & s_r_mask;
        int rx1 = (rx0 + 1) & s_r_mask;
        int ry1 = (ry0 + 1) & s_r_mask;

        const float& c00 = s_r[s_p[s_p[rx0] + ry0]];
        const float& c10 = s_r[s_p[s_p[rx1] + ry0]];
        const float& c01 = s_r[s_p[s_p[rx0] + ry1]];
        const float& c11 = s_r[s_p[s_p[rx1] + ry1]];

        float sx = cubic_step(xf);
        float sy = cubic_step(yf);

        float nx0 = lerp(c00, c10, sx);
        float nx1 = lerp(c01, c11, sx);

        return lerp(nx0, nx1, sy);
    }
    //-----------------------------------------------------------------------------------
    /* 2D fractional value noise, evaluated at (x, y) (5 octaves default). */
    float VxNoise::value2_fbm(float x, float y)
    {
        float  n = 0.0f;    // accumulated noise
        float  f = s_frequency_value;
        float df = s_d_frequency;
        float  a = 1.0f;
        float da = s_d_amplitude;
        float  t = 0.0f;    // total amp, for normalizing to [0..1]

        for (uint8_t i = 0; i < s_octave_count; i++)
        {
            n += a * value2(x * f, y * f);
            t += a;
            a *= da;
            f *= df;
        }
        return n / t;
    }
    //-----------------------------------------------------------------------------------
    /* Rotated 2D fractional value noise, evaluated at p (5 octaves default). */
    float VxNoise::value2_fbm_r(const glm::vec2& p)
    {
        glm::vec2 p_ = p;
        float  n = 0.0f;    // accumulated noise
        float  f = s_frequency_value;
        float df = s_d_frequency;
        float  a = 1.0f;
        float  t = 0.0f;    // total amp, for normalizing to [0..1]
        float da = s_d_amplitude;

        for (uint8_t i = 0; i < s_octave_count; i++)
        {
            n += a * value2(p_.x * f, p_.y * f);
            p_ = s_mrot2 * p_;
            t += a;
            a *= da;
            f *= df;
        }
        return n / t;
    }
    //-----------------------------------------------------------------------------------
    /* 3D value noise, evaluated at (x, y). */
    float VxNoise::value3(float x, float y, float z)
    {
        int xi = fast_floor(x);
        int yi = fast_floor(y);
        int zi = fast_floor(z);
        
        float xf = x - xi;
        float yf = y - yi;
        float zf = z - zi;

        int rx0 = xi & s_r_mask;
        int ry0 = yi & s_r_mask;
        int rz0 = zi & s_r_mask;
        int rx1 = (rx0 + 1) & s_r_mask;
        int ry1 = (ry0 + 1) & s_r_mask;
        int rz1 = (rz0 + 1) & s_r_mask;

        const float& c000 = s_r[s_p[s_p[s_p[rx0] + ry0] + rz0]];
        const float& c100 = s_r[s_p[s_p[s_p[rx1] + ry0] + rz0]];
        const float& c010 = s_r[s_p[s_p[s_p[rx0] + ry1] + rz0]];
        const float& c110 = s_r[s_p[s_p[s_p[rx1] + ry1] + rz0]];
        const float& c001 = s_r[s_p[s_p[s_p[rx0] + ry0] + rz1]];
        const float& c101 = s_r[s_p[s_p[s_p[rx1] + ry0] + rz1]];
        const float& c011 = s_r[s_p[s_p[s_p[rx0] + ry1] + rz1]];
        const float& c111 = s_r[s_p[s_p[s_p[rx1] + ry1] + rz1]];

        float u = cubic_step(xf);
        float v = cubic_step(yf);
        float w = cubic_step(zf);
        
        float x1, x2, y1, y2;
        x1 = lerp(c000, c100, u);
        x2 = lerp(c010, c110, u);
        y1 = lerp(x1, x2, v);
        x1 = lerp(c001, c101, u);
        x2 = lerp(c011, c111, u);
        y2 = lerp(x1, x2, v);

        return lerp(y1, y2, w);
    }
    //-----------------------------------------------------------------------------------
    /* 3D fractional value noise, evaluated at (x, y) (5 octaves default). */
    float VxNoise::value3_fbm(float x, float y, float z)
    {
        float  n = 0.0f;    // accumulated noise
        float  f = s_frequency_value;
        float df = s_d_frequency;
        float  a = 1.0f;
        float da = s_d_amplitude;
        float  t = 0.0f;    // total amp, for normalizing to [0..1]

        for (uint8_t i = 0; i < s_octave_count; i++)
        {
            n += a * value3(x * f, y * f, z * f);
            t += a;
            a *= da;
            f *= df;
        }
        return n / t;
    }
    //-----------------------------------------------------------------------------------
    /* Rotated 3D fractional value noise, evaluated at p (5 octaves default). */
    float VxNoise::value3_fbm_r(const glm::vec3& p)
    {
        glm::vec3 p_ = p;
        float  n = 0.0f;    // accumulated noise
        float  f = s_frequency_value;
        float df = s_d_frequency;
        float  a = 1.0f;
        float  t = 0.0f;    // total amp, for normalizing to [0..1]
        float da = s_d_amplitude;

        for (uint8_t i = 0; i < s_octave_count; i++)
        {
            n += a * value3(p_.x * f, p_.y * f, p_.z * f);
            p_ = s_mrot3 * p_;
            t += a;
            a *= da;
            f *= df;
        }
        return n / t;
    }
    //-----------------------------------------------------------------------------------
    /* 2D Perlin noise, evaluated at (x, y). */
    float VxNoise::perlin2(float x, float y)
    {
        int xi = (int)x & 255;
        int yi = (int)y & 255;

        float xf = x - (int)x;
        float yf = y - (int)y;

        float u = quintic_step(xf);
        float v = quintic_step(yf);

        int g00, g01, g10, g11;
        g00 = s_p_p[s_p_p[  xi]+  yi];
        g10 = s_p_p[s_p_p[1+xi]+  yi];
        g01 = s_p_p[s_p_p[  xi]+1+yi];
        g11 = s_p_p[s_p_p[1+xi]+1+yi];

        float x1, x2;
        x1 = lerp(grad2(g00, xf, yf  ), grad2(g10, xf-1, yf  ), u);
        x2 = lerp(grad2(g01, xf, yf-1), grad2(g11, xf-1, yf-1), u);
        
        return (lerp(x1, x2, v) + 1.0f) * 0.5f;
    }
    //-----------------------------------------------------------------------------------
    /* 2D fractional Perlin noise, evaluated at (x, y) (5 octaves default). */
    float VxNoise::perlin2_fbm(float x, float y)
    {
        float  n = 0.0f;    // accumulated noise
        float  t = 0.0f;    // total amp, for normalizing [0..1]
        float  f = s_frequency_perlin;
        float df = s_d_frequency;
        float  a = 1.0f;
        float da = s_d_amplitude;

        for (uint8_t i = 0; i < s_octave_count; i++)
        {
            n += a * perlin2(x * f, y * f);
            t += a;
            a *= da;
            f *= df;
        }
        return n / t;            
    }
    //-----------------------------------------------------------------------------------
    /* 3D Perlin noise, evaluated at (x, y, z). */
    float VxNoise::perlin3(float x, float y, float z)
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
        g000 = s_p_p[s_p_p[s_p_p[  xi]+  yi]+  zi];
        g010 = s_p_p[s_p_p[s_p_p[  xi]+1+yi]+  zi];
        g001 = s_p_p[s_p_p[s_p_p[  xi]+  yi]+1+zi];
        g011 = s_p_p[s_p_p[s_p_p[  xi]+1+yi]+1+zi];
        g100 = s_p_p[s_p_p[s_p_p[1+xi]+  yi]+  zi];
        g110 = s_p_p[s_p_p[s_p_p[1+xi]+1+yi]+  zi];
        g101 = s_p_p[s_p_p[s_p_p[1+xi]+  yi]+1+zi];
        g111 = s_p_p[s_p_p[s_p_p[1+xi]+1+yi]+1+zi];

        float x1, x2, y1, y2;
        x1 = lerp(grad3(g000, xf  , yf  , zf  ), grad3(g100, xf-1, yf  , zf  ), u);
        x2 = lerp(grad3(g010, xf  , yf-1, zf  ), grad3(g110, xf-1, yf-1, zf  ), u);
        y1 = lerp(x1, x2, v);
        x1 = lerp(grad3(g001, xf  , yf  , zf-1), grad3(g101, xf-1, yf  , zf-1), u);
        x2 = lerp(grad3(g011, xf  , yf-1, zf-1), grad3(g111, xf-1, yf-1, zf-1), u);
        y2 = lerp(x1, x2, v);

        return (lerp(y1, y2, w) + 1.0f) * 0.5f;
    }
    //-----------------------------------------------------------------------------------
    /* 3D fractional Perlin noise, evaluated at (x, y, z) (5 octaves default). */
    float VxNoise::perlin3_fbm(float x, float y, float z)
    {
        float  n = 0.0f;    // accumulated noise
        float  t = 0.0f;    // total amp, for normalizing [0..1]
        float  f = s_frequency_perlin;
        float df = s_d_frequency;
        float  a = 1.0f;
        float da = s_d_amplitude;

        for (uint8_t i = 0; i < s_octave_count; i++)
        {
            n += a * perlin3(x*f, y*f, z*f);
            t += a;
            a *= da;
            f *= df;
        }
        return n / t;
    }
    //-----------------------------------------------------------------------------------
    /* Initializer : must be called before use. */
    void VxNoise::init(uint32_t seed)
    {
        // Initialize value noise permutation tables
        //

        // Value noise random number generation (unifrom float dist)
        std::mt19937 gen(seed);
        std::uniform_real_distribution<float> dist_float;
        auto r_float = std::bind(dist_float, gen);
        for (uint32_t i = 0; i < s_r_sz; i++)
        {
            s_r[i] = r_float();
            s_p[i] = i;
        }

        // Permute random floats
        std::uniform_int_distribution<uint> dist_uint;
        auto r_uint = std::bind(dist_uint, gen);
        for (uint32_t i = 0; i < s_r_sz; i++)
        {
            uint32_t j = r_uint() & s_r_mask;
            std::swap(s_p[i], s_p[j]);
            s_p[i + s_r_sz] = s_p[i];
        }

        // Initialize Perlin noise permutation table
        for (int i = 0; i < 512; i++)
            s_p_p[i] = s_uint32_p[i % 256];

        SYN_CORE_TRACE("voxel noise engine initialized.");
    }
    //-----------------------------------------------------------------------------------
    /* Save noise as png image. */
    void VxNoise::save_noise_PNG(const std::string& _file_name, 
                                 uint32_t _width, 
                                 uint32_t _height, 
                                 float* _data)
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