
#pragma once

#include <Synapse/Types.hpp>
#include <glm/glm.hpp>


#define SQRT_3_DIV_2        0.866025404f
#define SQRT_3_DIV_2_2_INV  0.577350269f


namespace Syn
{

    inline int fast_floor(float x) { return x > 0 ? (int)x : (int)x - 1; }
    inline float dot_(const glm::vec3& g, float x, float y, float z) { return g.x*x + g.y*y + g.z*z; }
    inline float lerp(float a, float b, float t) { return (1.0f-t)*a + t*b; }
    inline float smoothstep(float t) { return t * t * (3.0f - 2.0f * t); }
    inline float smoothstep_d(float t) { return 6.0f * t * (1.0f - t); }
    inline float quintic(float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }

    class cnoise
    {
    public:
        glm::vec3 grad3[12] = 
        {
            glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0), glm::vec3(1, -1, 0), glm::vec3(-1, -1, 0),
            glm::vec3(1, 0, 1), glm::vec3(-1, 0, 1), glm::vec3(1, 0, -1), glm::vec3(-1, 0, -1),
            glm::vec3(0, 1, 1), glm::vec3(0, -1, 1), glm::vec3(0, 1, -1), glm::vec3(0, -1, -1)
        };

        int p[256] = 
        {
            151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,
            21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,
            237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,77,146,158,231,
            83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54, 65,25,63,161,1,
            216,80,73,209,76,132,187,208, 89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,
            173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,
            47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,
            167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
            251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,49,192,214,
            31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,
            29,24,72,243,141,128,195,78,66,215,61,156,180
        };

        int* perm = nullptr;;

        const glm::mat3 m3rot  = glm::mat3( 0.00f,  0.80f,  0.60f, \
                		                   -0.80f,  0.36f, -0.48f, \
                		                   -0.60f, -0.48f,  0.64f);
        const glm::mat3 m3irot = glm::mat3( 0.00f, -0.80f, -0.60f, \
                                            0.80f,  0.36f, -0.48f, \
                                            0.60f, -0.48f,  0.64f );

        int noise_octaves = 5;
        float noise_frequency = 0.01f;
        float noise_amplitude = 1.0f;
        float noise_delta_freq = 2.0f;
        float noise_delta_amp = 0.5f;
        glm::vec3 noise_offset = { 0.0f, 0.0f, 0.0f };
        bool noise_fbm_rotation = true;
        void set_octaves(int o) { noise_octaves = o; }
        void set_amplitude(float a) { noise_amplitude = a; }
        void set_frequency(float f) { noise_frequency = f; }
        void set_delta_amp(float da) { noise_delta_amp = da; }
        void set_delta_freq(float df) { noise_delta_freq = df; }
        void set_offset(const glm::vec3& o) { noise_offset = o; }
        void set_fbm_rotation(bool r) { noise_fbm_rotation = r; }


    public:
        cnoise()
        {
            perm = new int[512];
            for (int i = 0; i < 512; i++)
                perm[i] = p[i & 255];
        }

        ~cnoise()
        {
            if (perm)
                delete[] perm;
        }

        /*
         * 3D fractal Brownian motion noise.
         */
        float fbm_3(const glm::vec3& _p)
        {
            //_p = _p + offset
            int    o = noise_octaves;
            float  a = noise_amplitude;
            float da = noise_delta_amp;
            float  f = noise_frequency;
            float df = noise_delta_freq;
            float  t = 0;

            glm::mat3 m = noise_fbm_rotation ? m3rot : glm::mat3(1.0f);

            glm::vec3 p = _p + noise_offset;
            float n = 0.0f;

            for (int i = 0; i < o; i++)
            {
                p = m * p;  // rotate position
                n += a * noise_perlin_3(p, f); // get noise
                
                t += a;     // accumulate total amplitude
                a *= da;    // update amplitude and frequency for next octave
                f *= df;
            }
            return n;
        }

        /*
         * 3D perlin fractal Brownian motion noise with computation of 
         * noise partial derivatives. Returns a glm::vec4 where the noise
         * values is stored in .x and partial derivatives in .yzw.
         */
        const glm::vec4 fbm_3_d(const glm::vec3& _p)
        {
            glm::vec3 p = _p + noise_offset;
            float  a = noise_amplitude;
            float da = noise_delta_amp;
            float  f = noise_frequency;
            float df = noise_delta_freq;
            float  r = a;
            glm::vec3 tdn = glm::vec3(0.0f); // acc partial derivatives
            glm::vec3 dn  = glm::vec3(0.0f); // partial derivatives per octave
            float t = 0.0f; // acc noise
            //float t = noise_perlin_3_d(_p, tdn, f) * a;

            for (int i = 0; i < noise_octaves; i++)
            {
                t += a * noise_perlin_3_d(_p, dn, f);
                tdn += dn * a;
                f *= df;
                a *= da;
                r += a;
            }
            //float div = 1.0f / r;
            //return glm::vec4(t * div, tdn * div);   // .x = noise, .yzw = partial derivs
            return glm::vec4(t, glm::normalize(tdn));   // .x = noise, .yzw = partial derivs
        }

        float noise_perlin_3(const glm::vec3& _p, float _frequency)
        {
            glm::vec3 p = _p * _frequency;

            int x = fast_floor(p.x);
            int y = fast_floor(p.y);
            int z = fast_floor(p.z);

            float xf = p.x - x;
            float yf = p.y - y;
            float zf = p.z - z;

            x = x & 255;
            y = y & 255;
            z = z & 255;

            int gi000 = perm[x+perm[y+perm[z]]] % 12;
            int gi001 = perm[x+perm[y+perm[z+1]]] % 12;
            int gi010 = perm[x+perm[y+1+perm[z]]] % 12;
            int gi011 = perm[x+perm[y+1+perm[z+1]]] % 12;
            int gi100 = perm[x+1+perm[y+perm[z]]] % 12;
            int gi101 = perm[x+1+perm[y+perm[z+1]]] % 12;
            int gi110 = perm[x+1+perm[y+1+perm[z]]] % 12;
            int gi111 = perm[x+1+perm[y+1+perm[z+1]]] % 12;

            float n000 = dot_(grad3[gi000], xf, yf, zf);
            float n100 = dot_(grad3[gi100], xf-1, yf, zf);
            float n010 = dot_(grad3[gi010], xf, yf-1, zf);
            float n110 = dot_(grad3[gi110], xf-1, yf-1, zf);
            float n001 = dot_(grad3[gi001], xf, yf, zf-1);
            float n101 = dot_(grad3[gi101], xf-1, yf, zf-1);
            float n011 = dot_(grad3[gi011], xf, yf-1, zf-1);
            float n111 = dot_(grad3[gi111], xf-1, yf-1, zf-1);

            float u = smoothstep(xf);
            float v = smoothstep(yf);
            float w = smoothstep(zf);

            float nx00 = lerp(n000, n100, u);
            float nx01 = lerp(n001, n101, u);
            float nx10 = lerp(n010, n110, u);
            float nx11 = lerp(n011, n111, u);

            float nxy0 = lerp(nx00, nx10, v);
            float nxy1 = lerp(nx01, nx11, v);

            float nxyz = lerp(nxy0, nxy1, w);

            //return (nxyz + SQRT_3_DIV_2) * SQRT_3_DIV_2_2_INV;
            return nxyz;
            //return (nxyz + 1.0f) * 0.5f;
        }

        float noise_perlin_2_d(const glm::vec2& _p, glm::vec2& _dn_out, float _frequency)
        {
            return 0;
        }

        float noise_perlin_3_d(const glm::vec3& _p, glm::vec3& _dn_out, float _frequency)
        {
            glm::vec3 p = _p * _frequency;

            int x = (int)fast_floor(p.x);
            int y = (int)fast_floor(p.y);
            int z = (int)fast_floor(p.z);

            float xf = p.x - x;
            float yf = p.y - y;
            float zf = p.z - z;

            x = x & 255;
            y = y & 255;
            z = z & 255;

            float u = smoothstep(xf);
            float v = smoothstep(yf);
            float w = smoothstep(zf);

            float du = smoothstep_d(xf);
            float dv = smoothstep_d(yf);
            float dw = smoothstep_d(zf);

            // cell corner gradient indices
            int gi000 = perm[x+perm[y+perm[z]]] % 12;
            int gi001 = perm[x+perm[y+perm[z+1]]] % 12;
            int gi010 = perm[x+perm[y+1+perm[z]]] % 12;
            int gi011 = perm[x+perm[y+1+perm[z+1]]] % 12;
            int gi100 = perm[x+1+perm[y+perm[z]]] % 12;
            int gi101 = perm[x+1+perm[y+perm[z+1]]] % 12;
            int gi110 = perm[x+1+perm[y+1+perm[z]]] % 12;
            int gi111 = perm[x+1+perm[y+1+perm[z+1]]] % 12;

            // dot products of gradients and corner scalars
            float a = dot_(grad3[gi000], xf, yf, zf);
            float b = dot_(grad3[gi100], xf-1, yf, zf);
            float c = dot_(grad3[gi010], xf, yf-1, zf);
            float d = dot_(grad3[gi110], xf-1, yf-1, zf);
            float e = dot_(grad3[gi001], xf, yf, zf-1);
            float f = dot_(grad3[gi101], xf-1, yf, zf-1);
            float g = dot_(grad3[gi011], xf, yf-1, zf-1);
            float h = dot_(grad3[gi111], xf-1, yf-1, zf-1);

            // for algebraic 'simplification'
            float k0 = (b - a); 
            float k1 = (c - a); 
            float k2 = (e - a); 
            float k3 = (a + d - b - c); 
            float k4 = (a + f - b - e); 
            float k5 = (a + g - c - e); 
            float k6 = (b + c + e + h - a - d - f - g);

            float n = a + u * k0 + v * k1 + w * k2 + u * v * k3 + u * w * k4 + v * w * k5 + u * v * w * k6;
            _dn_out.x = du *(k0 + v * k3 + w * k4 + v * w * k6); // dn/dx
            _dn_out.y = dv *(k1 + u * k3 + w * k5 + u * w * k6); // dn/dy
            _dn_out.z = dw *(k2 + u * k4 + v * k5 + u * v * k6); // dn/dz
            _dn_out *= _frequency;

            return n;
        }

    };



}