
#pragma once

#include <random>

#include <Synapse/Core.hpp>


namespace Syn
{
    class Random
    {
    public:
        static void init()
        {
            s_randomEngine.seed(std::random_device()());
        }

        static uint32_t rint()
        {
            return s_intDistribution(s_randomEngine);
        }

        static uint32_t rintr(uint32_t _lo=0, uint32_t _hi=std::numeric_limits<uint32_t>::max())
        {
            return s_intDistribution(s_randomEngine) % (_hi - _lo) + _lo;
        }

        static float rfloat()
        {
            return s_realDistribution(s_randomEngine) * s_invMaxFloat;
        }

        static float rfloatr(float _lo=0.0f, float _hi=std::numeric_limits<float>::max())
        {
            //float r = s_realDistribution(s_randomEngine);
            //float div = std::numeric_limits<float>::max() / (_hi - _lo);
            //r = r / div + _lo;
            return s_realDistribution(s_randomEngine) / (std::numeric_limits<float>::max() / (_hi-_lo)) + _lo;
        }

		static char* rstr(size_t len)
		{
			static char buffer[256];
			memset(buffer, 0, 256);
			SYN_CORE_ASSERT(len < 256, "cannot handle >255 bytes.");
			for (uint32_t i = 0; i < len; i++)
				buffer[i] = s_alphanum[rand() % s_alphanumSize];
			return buffer;
		}

    private:
        static std::mt19937 s_randomEngine;
        static std::uniform_int_distribution<std::mt19937::result_type> s_intDistribution;
        static std::uniform_real_distribution<> s_realDistribution;
        static float s_invMaxFloat;
        static const char* s_alphanum;
        static size_t s_alphanumSize;
    };

}


