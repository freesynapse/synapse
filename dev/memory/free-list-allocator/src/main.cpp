
#include <stdio.h>

#include "Profiler.h"
//#include "MemoryResource.h"
#include "mm.h"

using namespace Syn;

//---------------------------------------------------------------------------------------
struct smallStruct
{
	float i;
	float j;
};

struct largeStruct
{
	uint8_t i[1024*1024];
};

constexpr int n_i = 1000;
constexpr int n_j = 50;
constexpr int n_ij = n_i * n_j;

template<typename T>
void newDeleteBenchmark0()
{
	ProfilerTimer profilerTimer(__PRETTY_FUNCTION__);

	for (int i = 0; i < n_i * n_j; i++)
	{
		T* c = new T;
		delete c;
	}

	printf("execution took %.3f ms.\n", (float)profilerTimer.stop()/1000.0f);
}

template<typename T>
void newDeleteBenchmark1()
{
	ProfilerTimer profilerTimer(__PRETTY_FUNCTION__);

	for (int i = 0; i < n_i; i++)
	{
		T* array[n_j];
		for (int j = 0; j < n_j; j++)
			array[j] = new T;
		for (int j = 0; j < n_j; j++)
			delete array[j];
	}

	printf("execution took %.3f ms.\n", (float)profilerTimer.stop()/1000.0f);
}

template<typename T>
void allocateFreeBenchmark0()
{
	ProfilerTimer profilerTimer(__PRETTY_FUNCTION__);
	for (int i = 0; i < n_i * n_j; i++)
	{
		T* c = (T*)Syn::allocate(sizeof(T));
		Syn::deallocate(c);
	}

	printf("execution took %.3f ms.\n", (float)profilerTimer.stop()/1000.0f);
}

template<typename T>
void allocateFreeBenchmark1()
{
	ProfilerTimer profilerTimer(__PRETTY_FUNCTION__);
	for (int i = 0; i < n_i; i++)
	{
		T* array[n_j];
		for (int j = 0; j < n_j; j++)
		{
			array[j] = (T*)Syn::allocate(sizeof(T));
		}
		for (int j = 0; j < n_j; j++)
			Syn::deallocate(array[j]);
	}

	printf("execution took %.3f ms.\n", (float)profilerTimer.stop()/1000.0f);
}

void test05_customHeap()
{
	word_t* var = (word_t*)Syn::allocate(32);
	//debugTraverseHeap();
	MemoryAllocator::debugTraverseFreeList();
	printf("\n");

	Syn::deallocate(var);
	MemoryAllocator::debugTraverseFreeList();
	printf("\n");

	word_t* var2 = (word_t*)Syn::allocate(40);
	MemoryAllocator::debugTraverseFreeList();
	printf("\n");

	MemoryAllocator::debugTraverseHeap();

	// heap is now as follows:
	//  Block size		Alloc		Data size
	//  	  64 B		 free			 32 B
	// 		  72 B		alloc			 40 B
	// 		 120 B		 free			 88 B
	// 
	// Total size = 256 B (== MEMORY_CHUNK_SIZE)
	// 
	// 

	// make a request larger than the largest free block
	printf("\n\n\n");
	word_t* var3 = (word_t*)Syn::allocate(128);
	word_t* var4 = (word_t*)Syn::allocate(128);
	word_t* var5 = (word_t*)Syn::allocate(128);
	MemoryAllocator::debugTraverseFreeList();
	MemoryAllocator::debugTraverseHeap();

	Syn::deallocate(var2);
	Syn::deallocate(var3);
	Syn::deallocate(var4);
	Syn::deallocate(var5);

	var3 = (word_t*)Syn::allocate(128);
	var4 = (word_t*)Syn::allocate(129);
	MemoryAllocator::debugTraverseHeap();
}

void test06_clearHeap()
{
	printf("Heap before clear:\n");
	MemoryAllocator::debugTraverseHeap();

	MemoryAllocator::clearHeap();
	printf("\nHeap after clear:\n");
	MemoryAllocator::debugTraverseHeap();
}

void test07_blockCoalescing()
{
	block_header_t* b0 = GET_HEADER(MemoryAllocator::allocate(128));
	block_header_t* b1 = GET_HEADER(MemoryAllocator::allocate(128));
	block_header_t* b2 = GET_HEADER(MemoryAllocator::allocate(128));
	MemoryAllocator::debugTraverseHeap();

	printf("\ndeallocating blocks...\n");
	MemoryAllocator::deallocate(b0->data);
	MemoryAllocator::deallocate(b1->data);
	MemoryAllocator::deallocate(b2->data);
	MemoryAllocator::debugTraverseHeap();
	printf("free list before coalescing:\n");
	MemoryAllocator::debugTraverseFreeList();

	MemoryAllocator::coalesceBlocks(b1);
	printf("\nfree list after coalescing block 1\n");
	MemoryAllocator::debugTraverseFreeList();
	printf("\nheap after coalescing block 1\n");
	MemoryAllocator::debugTraverseHeap();

	// now the heap consists of two blocks, the first with block size 480 (448 data size)
	// and the second with the remaining heap size. 

	// allocate 512 bytes, requiring splitting of the 2nd block
	block_header_t* b3 = GET_HEADER(MemoryAllocator::allocate(512));
	// deallocate and show free list
	MemoryAllocator::deallocate(b3->data);
	MemoryAllocator::debugTraverseFreeList();
	// coalesce the new block, should result in a single block
	MemoryAllocator::coalesceBlocks(b3);
	MemoryAllocator::debugTraverseFreeList();
	MemoryAllocator::debugTraverseHeap();
	// only a single block of m_heapSize remains

	// test coalescing with the next block only
	printf("\ncoalescing first block with next:\n");
	b0 = GET_HEADER(MemoryAllocator::allocate(128));
	b1 = GET_HEADER(MemoryAllocator::allocate(128));
	MemoryAllocator::deallocate(b0->data);
	MemoryAllocator::deallocate(b1->data);
	MemoryAllocator::debugTraverseHeap();
	printf("-- coalescing block 0 (with block 1)\n");
	MemoryAllocator::coalesceBlocks(b0);
	MemoryAllocator::debugTraverseHeap();
	MemoryAllocator::clearHeap();

	// split first block into two again
	printf("\ncoalescing second block with previous:\n");
	b0 = GET_HEADER(MemoryAllocator::allocate(128));
	b1 = GET_HEADER(MemoryAllocator::allocate(128));
	b2 = GET_HEADER(MemoryAllocator::allocate(128));
	MemoryAllocator::deallocate(b0->data);
	MemoryAllocator::deallocate(b1->data);
	MemoryAllocator::debugTraverseHeap();
	printf("-- coalescing block 1 (with block 0)\n");
	MemoryAllocator::coalesceBlocks(b1);
	MemoryAllocator::debugTraverseHeap();

	// try to coalesce allocated block
	printf("-- coalescing allocated block 2\n");
	MemoryAllocator::coalesceBlocks(b2);
	printf("-- deallocating block 2\n");
	MemoryAllocator::deallocate(b2->data);
	MemoryAllocator::debugTraverseHeap();
	printf("-- coalescing allocated block 2 again\n");
	MemoryAllocator::coalesceBlocks(b2);
	MemoryAllocator::debugTraverseHeap();

}

template<typename T>
void test08_newDeleteBenchmark()
{
	printf("newDeleteBenchmark0:\n\t");
	newDeleteBenchmark0<T>();

	printf("newDeleteBenchmark1:\n\t");
	newDeleteBenchmark1<T>();
}

template<typename T>
void test09_allocDeallocBenchmark()
{
	MemoryAllocator::resetCounters();
	printf("allocateFreeBenchmark0:\n\t");
	allocateFreeBenchmark0<T>();
	printf("allocs: %lu, deallocs: %lu, splits: %lu, merges: %lu\n", 
			MemoryAllocator::numAllocations(), 
			MemoryAllocator::numDeallocations(), 
			MemoryAllocator::numBlockSplits(),
			MemoryAllocator::numBlockMerges());

	MemoryAllocator::resetCounters();
	printf("allocateFreeBenchmark1:\n\t");
	allocateFreeBenchmark1<T>();
	printf("allocs: %lu, deallocs: %lu, splits: %lu, merges: %lu\n", 
			MemoryAllocator::numAllocations(), 
			MemoryAllocator::numDeallocations(), 
			MemoryAllocator::numBlockSplits(),
			MemoryAllocator::numBlockMerges());

}

template<typename T>
void test10_heapDefragCoalescing()
{
	// first defragment the heap
	for (int i = 0; i < n_i; i++)
	{
		T* array[n_j];
		for (int j = 0; j < n_j; j++)
		{
			array[j] = (T*)Syn::allocate(sizeof(T));
		}
		for (int j = 0; j < n_j; j++)
			Syn::deallocate(array[j]);
	}

	printf("free list length = %zu\n", MemoryAllocator::freeListLength());
	// time the coalescing defrag
	ProfilerTimer profilerTimer(__PRETTY_FUNCTION__);
	// coalesce blocks (if any is found)
	printf("coalescing 10 frames...\n");
	for (int i = 0; i < 10; i++)
		MemoryAllocator::defragHeapCoalesce();
	printf("execution took %.3f ms.\n", (float)profilerTimer.stop()/1000.0f);
	printf("free list length = %zu\n", MemoryAllocator::freeListLength());

}

//---------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	Profiler::get().beginSession("ll-tests");

	//
	printf("----------- TEST 01: alignment -----------\n");
	for (int i = 1; i <= 33; i+=8)
		printf("ALIGN(%d) = %d\n", i, ALIGN(i));

	//
	printf("\n----------- TEST 02: align data structures -----------\n");
	printf("ALIGN(sizeof(block_header_t)) = %ld\n", ALIGN(sizeof(block_header_t)));
	printf("ALIGN(sizeof(block_header_t*)) = %ld\n", ALIGN(sizeof(block_header_t*)));
	printf("ALIGN(sizeof(uint16_t)) = %ld\n", ALIGN(sizeof(uint16_t)));

	//
	printf("\n----------- TEST 03: bit masking and allocation bit -----------\n");
	int a = 8;		printf("a      = %d\n", a);
	a |= 1;			printf("a |= 1 = %d\n", a);
	a &= 8;			printf("a &= 8 = %d\n", a);
	a = 9;
	a = (a >> 1) << 1;
	printf("a = (a >> 1) << 1 = %d\n", a);
	a = 9 & ~0x1;
	printf("a = (9 & %d) = %d\n", ~0x1, a);
	
	//
	printf("\n----------- TEST 04: mm test -----------\n");
	printf("HDR_SIZE = %zu\n", HDR_SIZE);
	printf("FTR_SIZE = %zu\n", FTR_SIZE);
	printf("BLOCK_OVERHEAD_SIZE = %zu\n", BLOCK_OVERHEAD_SIZE);
	printf("MIN_BLOCK_SIZE = %zu\n", MIN_BLOCK_SIZE);
	
	printf("\n----------- TEST 05: custom heap -----------\n");
	MemoryAllocator::initialize();
	test05_customHeap();
	
	printf("\n----------- TEST 06: clear heap -----------\n");
	test06_clearHeap();

	printf("\n----------- TEST 07: block coalescing -----------\n");
	test07_blockCoalescing();


	#define MEM_OBJECT largeStruct

	// benchmark new/delete
	printf("\n----------- TEST 09: new/delete benchmark -----------\n");
	printf("benchmarking new/delete: %.1fM calls.\n", (float)(n_ij) / 1e6);
	test08_newDeleteBenchmark<MEM_OBJECT>();


	printf("\n----------- TEST 09: allocate() / deallocate() benchmarks -----------\n");
	test09_allocDeallocBenchmark<MEM_OBJECT>();
	printf("sizeof(float) = %zu, sizeof(smallStruct) = %zu, sizeof(largeStruct) = %zu\n", sizeof(float), sizeof(smallStruct), sizeof(largeStruct));
	MemoryAllocator::clearHeap();

	printf("\n----------- TEST 10: heap defrag: coalescing free blocks -----------\n");
	test10_heapDefragCoalescing<MEM_OBJECT>();
	MemoryAllocator::debugTraverseFreeList();
	MemoryAllocator::clearHeap();

	printf("\n----------- TEST 11: heap defrag: reallocation -----------\n");


	return 0;
}