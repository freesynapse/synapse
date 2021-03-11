#ifndef __MM_H
#define __MM_H

#include <stdint.h>		// uintptr_t, uint32_t
#include <stddef.h>		// size_t
#include <utility>		// std::declval

// predefined sizes .................................................................................................................................
constexpr uint32_t MEMORY_CHUNK_SIZE = 1024 * 1024 * 64;
typedef uintptr_t word_t;
constexpr uint8_t WORD_SIZE = sizeof(word_t);
// memory alignment .................................................................................................................................
#define ALIGN(_size) (((_size) + (WORD_SIZE-1)) & ~(WORD_SIZE - 1))
// block header struct ..............................................................................................................................
typedef struct block_hdr_ block_header_t;
struct block_hdr_
{
	block_header_t* next;	// Next free block.
	block_header_t* prev;	// Previous free block.
	size_t size_alloc;		// Lowest bit flag used to track allocation; since the size is aligned to WORD_SIZE, 
							// at least the 4 lower bits are unused.
	word_t data[1];			// Payload pointer; returned as void* to user upon allocation.
};
// block footer struct ..............................................................................................................................
typedef struct block_ftr_ block_footer_t;
struct block_ftr_
{ size_t size; }; // size of this block (used for coalescing)
// block-associated sizes ...........................................................................................................................
constexpr size_t HDR_SIZE = ALIGN(sizeof(block_header_t));
constexpr size_t FTR_SIZE = ALIGN(sizeof(block_footer_t));
constexpr size_t BLOCK_OVERHEAD_SIZE = HDR_SIZE + FTR_SIZE;
constexpr size_t HDR_DATA_PTR_SIZE = sizeof(std::declval<block_header_t>().data);
constexpr size_t MIN_BLOCK_SIZE = BLOCK_OVERHEAD_SIZE; // includes one word_t of data
// alloc size for an entire memory block including overhead and data size ...........................................................................
inline size_t BLOCK_SIZE(size_t _aligned_data_size)		{	return _aligned_data_size + BLOCK_OVERHEAD_SIZE - HDR_DATA_PTR_SIZE; 				}
inline size_t BLOCK_SIZE(block_header_t* _block_ptr)	{	return (_block_ptr->size_alloc & ~0x1) + BLOCK_OVERHEAD_SIZE - HDR_DATA_PTR_SIZE; 	}
// allocated size of data, given a block size (i.e. how many bytes are remaining after overhead) ....................................................
inline size_t DATA_SIZE(size_t _block_size) 			{ 	return _block_size + HDR_DATA_PTR_SIZE - BLOCK_OVERHEAD_SIZE; 						}
// header accessor ..................................................................................................................................
#define GET_HEADER(_data_ptr) 						((block_header_t*)((char*)(_data_ptr) + HDR_DATA_PTR_SIZE - HDR_SIZE))
// size and allocation bit accessors ................................................................................................................
#define SET_SIZE_ALLOC(_block_ptr, _aligned_size) 	((_block_ptr)->size_alloc = (_aligned_size) | 0x1)
#define SET_SIZE(_block_ptr, _aligned_size) 		((_block_ptr)->size_alloc = (_aligned_size))
#define GET_SIZE(_block_ptr)						((_block_ptr)->size_alloc & ~0x1)
#define GET_ALLOC_BIT(_block_ptr)					((_block_ptr)->size_alloc & 0x1)
#define SET_ALLOC_BIT(_block_ptr)					((_block_ptr)->size_alloc = (_block_ptr)->size_alloc | 0x1)
#define UNSET_ALLOC_BIT(_block_ptr)					((_block_ptr)->size_alloc = (_block_ptr)->size_alloc & ~0x1)
// footer accessors .................................................................................................................................
inline block_footer_t* GET_FOOTER(block_header_t* _block_ptr) 				{ 	return (block_footer_t*)((char*)_block_ptr + HDR_SIZE - HDR_DATA_PTR_SIZE + GET_SIZE(_block_ptr));	}
inline block_footer_t* GET_FOOTER(word_t* _data_ptr) 						{ 	return (block_footer_t*)((char*)_data_ptr + GET_SIZE(GET_HEADER(_data_ptr))); }
inline void SET_FOOTER(block_header_t* _block_ptr, size_t _aligned_size)	{	GET_FOOTER(_block_ptr->data)->size = _aligned_size;	}
inline void SET_FOOTER(word_t* _data_ptr, size_t _aligned_size)				{	GET_FOOTER(_data_ptr)->size = _aligned_size;		}

// Memory allocator class (static) ..................................................................................................................
//
namespace Syn {

	class MemoryAllocator
	{
		#define MEMORY_ALLOCATOR_STDOUT
		//#undef MEMORY_ALLOCATOR_STDOUT
		
		#define DEBUG_MEMORY_ALLOCATOR_LOG
		#undef DEBUG_MEMORY_ALLOCATOR_LOG
		
		#define DEBUG_MEMORY_ALLOCATOR_PROFILING
		#undef DEBUG_MEMORY_ALLOCATOR_PROFILING

	public:
		static void initialize(size_t _heap_size=MEMORY_CHUNK_SIZE);
		static void shutdown();
		// alloc and free -----------------------------------------------------
		static void* allocate(size_t _size);
		static void deallocate(void* _data_ptr);
		// deallocate the whole heap ------------------------------------------
		static void clearHeap();
		// debug functions ----------------------------------------------------
		static void debugTraverseHeap(bool _debug_footer=true);
		static void debugTraverseFreeList(bool _debug_footer=true);
		static void debugBlock(block_header_t* _block_ptr, bool _debug_footer=true);
		// counter accessors --------------------------------------------------
		static uint64_t numAllocations() 	{ return m_numAllocations; 	 }
		static uint64_t numDeallocations() 	{ return m_numDeallocations; }
		static uint64_t numBlockSplits() 	{ return m_numBlockSplits;   }
		static uint64_t numBlockMerges() 	{ return m_numBlockMerges; 	 }
		static uint64_t numHeapClears()		{ return m_numHeapClears;	 }
		static void resetCounters() 		{ m_numAllocations 	 = 
											  m_numDeallocations = 
											  m_numBlockSplits 	 = 
											  m_numBlockMerges 	 = 
											  m_numHeapClears 	 = 0; }
		static size_t freeListLength() 
		{
			block_header_t* ptr = m_freeListPtr;
			size_t n = 0;
			while (ptr != nullptr)
			{
				n++;
				ptr = ptr->next;
			}
			return n;
		}

	private:		
		// extend current heap ----------------------------------------------------------
		static block_header_t* extendHeap(size_t _heap_size=MEMORY_CHUNK_SIZE);
		// free list functions ----------------------------------------------------------
		static void addFreeBlock(block_header_t* _block_ptr);
		static void removeFreeBlock(block_header_t* _block_ptr);
		static block_header_t* findFirst(size_t _aligned_data_size);
		static block_header_t* splitBlock(block_header_t* _block_ptr, size_t _requested_size);
		// coalescing of blocks ---------------------------------------------------------
		// Block coalescing prevents heap defragmentation (together with moving allocated block to
		// the beginning of the stack). The footer of the previous block is used to get a pointer to the
		// previous block (since the size of the footer plus the size of the data is leaves the fixed
		// sized header as the previous structure, accessible through the inline functions 
		// GET_PREV_FOOTER(block_header_t*) and GET_PREV_BLOCK(block_header_t*). The next block is accessible
		// through the BLOCK_SIZE() of the current block (and GET_NEXT_BLOCK).
		//
		// There are three unique cases where coalescing is possible:
		//
		// 1. The previous block is deallocated, the next block is allocated.
		// 2. The previous block is allocated, the next block is deallocated.
		// 3. Both the previous and the next block is deallocated.
		//
		//
	// temporary as public
	public:
		static size_t coalesceBlocks(block_header_t* _block_ptr);
	public:
		// Coalesces blocks in the free list, called once per frame.
		// Starts from at m_freeListPtr and tries to coalesce neighbouring blocks. Continues stepping
		// though the free list until empty. Presumably, if heavily defragmented, the free list will
		// shrink for each pass.
		static void defragHeapCoalesce();
		// Reallocates allocated blocks to the beginning of the heap, with updating of smart pointers.
		static void defragHeapReallocate();

	private:
		// private member variables -----------------------------------------------------
		//
		// virtual heap -----------------------------------------------------------------
		static block_header_t* m_heapPtr;
		static uint64_t m_memoryChunkSize;
		static uint64_t m_totalHeapSize;
		// free list --------------------------------------------------------------------
		static block_header_t* m_freeListPtr;
		static block_header_t* m_freeListLast;
		// debug trackers ---------------------------------------------------------------
		static uint64_t m_numAllocations;
		static uint64_t m_numDeallocations;
		static uint64_t m_numBlockSplits;
		static uint64_t m_numBlockMerges;
		static uint64_t m_numHeapClears;
	};

	// conveniance wrappers -------------------------------------------------------------
	inline void* allocate(size_t _size)	{	return MemoryAllocator::allocate(_size); }
	inline void deallocate(void* _data)	{	MemoryAllocator::deallocate(_data); 	 }

}

#endif //__MM_H


		// helper functions for heap defragmentation
		//inline static block_footer_t* GET_PREV_FOOTER(block_header_t* _block_ptr)
		//{
		//	if (_block_ptr == m_heapPtr)
		//		return nullptr;
		//	return (block_footer_t*)((char*)_block_ptr - FTR_SIZE);
		//}
		//inline static block_header_t* GET_PREV_HEADER(block_header_t* _block_ptr)
		//{
		//	block_footer_t* prevFtr = GET_PREV_FOOTER(_block_ptr);
		//	if (prevFtr == nullptr)
		//		return nullptr;
		//	size_t prevFtrDataSize = FTR_SIZE + prevFtr->size;
		//	block_header_t* prevHdr = (block_header_t*)((char*)_block_ptr - prevFtrDataSize - HDR_SIZE + HDR_DATA_PTR_SIZE);
		//	return prevHdr;
		//}
		//inline static block_header_t* GET_NEXT_HEADER(block_header_t* _block_ptr)
		//{
		//	return (block_header_t*)((char*)_block_ptr + BLOCK_SIZE(_block_ptr));
		//}





