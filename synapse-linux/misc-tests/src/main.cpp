
#include <iostream>
#include "./Profiler.h"
#include <vector>


#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
	//#define __FUNCSIG__ (__FILE__ "(" STRINGIZE(__LINE__) "): " __func__)
	#define __S1(x) #x
	#define __S2(x) __S1(x)
	#define __FUNCSIG__ __FILE__ ":" __S2(__LINE__) ": " 
#endif
#define __func__ __FUNCSIG__

#define PASTE2(a, b) a ## b
#define PASTE(a, b) PASTE2(a, b)
#define UNIQUE(x) PASTE(x, PASTE(__LINE__, __COUNTER__))

int main(int argc, char* argv[])
{
	printf("%d\n", UNIQUE(2));


	return 0;
}




/*
const long long n = 5000000000;

inline float idiot_work(int _i)
{
	float f = (float)_i;
	return (f * f * f * f * f);
}


void stack_alloc_array()
{
	PROFILE_FUNCTION();
	printf("%s\n", __PRETTY_FUNCTION__);
	
	// allocate
	float array[n];
	// do work
	for (long long i = 0; i < n; i++)
		array[i] = idiot_work(i);
	
}

void stack_alloc_vector_pushback()
{
	PROFILE_FUNCTION();
	printf("%s\n", __PRETTY_FUNCTION__);

	// allocate
	{
		PROFILE_FUNCTION();
		std::vector<float> vec;
		// do work
		for (long long i = 0; i < n; i++)
			vec.push_back(idiot_work(i));
	}
}

void stack_alloc_vector_size()
{
	PROFILE_FUNCTION();
	printf("%s\n", __PRETTY_FUNCTION__);

	// allocate and initialize
	std::vector<float> vec;
	vec.resize(n);
	// do work
	for (long long i = 0; i < n; i++)
		vec[i] = idiot_work(i);
}

void heap_alloc_array()
{
	PROFILE_FUNCTION();
	printf("%s\n", __PRETTY_FUNCTION__);

	// allocate
	float* array = new float[n];
	// do work
	for (long long i = 0; i < n; i++)
		array[i] = idiot_work(i);
	// release
	delete[] array;

}

void heap_alloc_vector()
{
	PROFILE_FUNCTION();
	printf("%s\n", __PRETTY_FUNCTION__);

	// allocate
	std::vector<float> vec;
	{
		PROFILE_FUNCTION();
		// do work
		for (long long i = 0; i < n; i++)
			vec.push_back(idiot_work(i));
	}
	
	{
		PROFILE_FUNCTION();
		// release
		std::vector<float>().swap(vec);
	}

}


int main(int argc, char *argv[])
{	
	Profiler::get().beginSession("array_tests");

	{
		PROFILE_FUNCTION();
		printf("%s\n", __PRETTY_FUNCTION__);

		stack_alloc_array();
		stack_alloc_vector_pushback();
		stack_alloc_vector_size();

		heap_alloc_array();
		heap_alloc_vector();
	}

	Profiler::get().endSession();

	return 0;
}

*/



