
#pragma once

#include <stdint.h> // uint32_t
#include <string>   // std::string
#include "Synapse/Core.hpp"

namespace Syn
{
    extern std::string e_bitString;
    
    template<typename T>
    const char* uint2bin(T _uint)
    {
        uint32_t bits = 0;
        if (std::is_same<T, uint8_t>::value)        bits =  8;
        else if (std::is_same<T, uint16_t>::value)  bits = 16;
        else if (std::is_same<T, uint32_t>::value)  bits = 32;
        else if (std::is_same<T, uint64_t>::value)  bits = 64;
        else
        {
            SYN_CORE_WARNING("illegal type for function (allowed types are uint8_t, \
                              uint16_t, uint32_t, uint64_t).");
        };
        
        char c[bits+1];
        c[bits] = '\0';

        char *p = c;
        for (uint32_t b = 1 << (bits-1); b > 0; b >>= 1)
            *p++ = (_uint & b) ? '1' : '0';

        e_bitString = c;
        return e_bitString.c_str();
    }
    
}
