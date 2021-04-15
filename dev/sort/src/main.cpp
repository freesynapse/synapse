
#include <stdint.h>		// types
#include <stdio.h>
#include <iostream>

#include <algorithm>

#include "Random.h"
#include "RadixSort.h"


//---------------------------------------------------------------------------------------
void printArray(uint32_t* _arr, int _n, const char* _pref=nullptr)
{
	if (_pref != nullptr)
		printf("%s\n", _pref);

	for (int i = 0; i < _n; i++)
		printf("%u\n", _arr[i]);
	printf("\n");
}

//---------------------------------------------------------------------------------------
void initArray(int _seed, uint32_t* _arr, int _n)
{
	XORShift32::setSeed(_seed);
	for (int i = 0; i < _n; i++)
		_arr[i] = XORShift32::get();
}


//---------------------------------------------------------------------------------------
void stdSort(uint32_t* _arr, int _n)
{
	ProfilerTimer profilerTimer(__PRETTY_FUNCTION__);
	std::sort(_arr, _arr+_n);
	printf("execution took %.3f ms.\n", (float)profilerTimer.stop()/1000.0f);

}


//---------------------------------------------------------------------------------------
bool validSort(uint32_t* _arr, int _n)
{
	bool ascending = true;
	for (int i = 1; i < _n; i++)
	{
		if (_arr[i] < _arr[i - 1])
			ascending = false;
	}

	return ascending;

}


//---------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	constexpr int N = 1e7;

	uint32_t *arr = new uint32_t[N];

	printf("sorting %d random uint32_t using std::sort().\n", N);
	for (int i = 0; i < 10; i++)
	{
		initArray(1, arr, N);
		stdSort(arr, N);
		if (validSort(arr, N) != true)
			printf("sort invalid\n");
	}
	printf("\n");

	printf("sorting %d random uint32_t using radixSort256().\n", N);
	for (int i = 0; i < 10; i++)
	{
		initArray(1, arr, N);
		radixSort256(arr, N);
		if (validSort(arr, N) != true)
			printf("sort invalid\n");
	}
	
	return 0;
}


