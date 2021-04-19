#ifndef __RADIX_SORT_H
#define __RADIX_SORT_H


#include "Profiler.h"


static void radixSort256(uint32_t* _arr, int _n)
{
	ProfilerTimer profilerTimer(__PRETTY_FUNCTION__);

	if (_n <= 1)
		return;
	
	uint32_t* output = new uint32_t[_n];
	int* count = new int[256];
	uint32_t* originalPtr = _arr;
	int i;

	for (int shift = 0, s = 0; shift < 4; shift++, s += 8)
	{
		for (i = 0; i < 256; i++)
			count[i] = 0;
		
		for (i = 0; i < _n; i++)
			count[(_arr[i] >> s) & 0xff]++;

		for (i = 1; i < 256; i++)
			count[i] += count[i - 1];
		
		for (i = _n - 1; i >= 0; i--)
		{
			int idx = (_arr[i] >> s) & 0xff;
			output[--count[idx]] = _arr[i];
		}

		uint32_t* tmp = _arr;
		_arr = output;
		output = tmp;
	}

	if (originalPtr == output)
	{
		uint32_t* tmp = _arr;
		_arr = output;
		output = tmp;

		for (i = 0; i < _n; i++)
			_arr[i] = output[i];

	}

	delete[] output;
	delete[] count;

	printf("execution took %.3f ms.\n", (float)profilerTimer.stop()/1000.0f);

}


#endif // __RADIX_SORT_H
