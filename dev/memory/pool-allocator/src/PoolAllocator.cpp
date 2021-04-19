
#include <stdio.h>

#include "PoolAllocator.h"


block_t* g_freeListPtr 	= nullptr;
block_t* g_freeListLast = nullptr;
block_t* g_heapPtr 		= nullptr;
uint64_t g_heapSize 	= 0;

//---------------------------------------------------------------------------------------
void initialize()
{

}

//---------------------------------------------------------------------------------------
void shutdown()
{
	
}

//---------------------------------------------------------------------------------------
void* allocate(size_t _size)
{
	if (_size > WORD_SIZE)
	{
		#ifdef DEBUG_POOL_ALLOCATOR
		printf("%s: %zu exceeding fixed size of %zu.\n", __PRETTY_FUNCTION__, _size, WORD_SIZE);
		#endif

		return nullptr;
	}


}




