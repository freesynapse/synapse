#ifndef __RANDOM_H
#define __RANDOM_H


#include <stdint.h>


class XORShift32
{
public:
	static void setSeed(uint32_t _seed=1) {  m_seed = (_seed == 0) ? _seed++ : _seed;  }
	
	static uint32_t get() {  return xorshift32();  }

private:
	// 
	static uint32_t xorshift32()
	{
		uint32_t x = m_seed;
		x ^= x << 13;
		x ^= x >> 17;
		x ^= x << 5;
		m_seed = x;
		return x;
	}

	// private member variables
	static uint32_t m_seed;


};

uint32_t XORShift32::m_seed = 123455;



#endif // __RANDOM_H