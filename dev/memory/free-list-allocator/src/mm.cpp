
#include <stdio.h>
#include <unistd.h>		// sbrk()
#include <iostream>
#include <iomanip>

#include "mm.h"
#include "Profiler.h"


using namespace Syn;

// virtual heap
block_header_t* MemoryAllocator::m_heapPtr 		   = nullptr;
uint64_t 		MemoryAllocator::m_memoryChunkSize = MEMORY_CHUNK_SIZE;
uint64_t 		MemoryAllocator::m_totalHeapSize   = 0;
// free list
block_header_t* MemoryAllocator::m_freeListPtr  = nullptr;
block_header_t* MemoryAllocator::m_freeListLast = nullptr;
// trackers
uint64_t MemoryAllocator::m_numAllocations  	   = 0;
uint64_t MemoryAllocator::m_numDeallocations	   = 0;
uint64_t MemoryAllocator::m_numBlockSplits  	   = 0;
uint64_t MemoryAllocator::m_numBlockMerges  	   = 0;
uint64_t MemoryAllocator::m_numHeapClears		   = 0;

std::string spaces;

//---------------------------------------------------------------------------------------
void MemoryAllocator::initialize(size_t _heap_size)
{
	PROFILE_FUNCTION();
	
	// request initial memory chunk to serve as heap
	m_memoryChunkSize = _heap_size;
	extendHeap();
}

//---------------------------------------------------------------------------------------
void MemoryAllocator::shutdown()
{
	PROFILE_FUNCTION();
	
	// release all previously requested memory
	sbrk(-m_totalHeapSize);
	m_totalHeapSize = 0;

	// reset pointers
	m_freeListPtr->next = m_freeListLast->next = nullptr;
	m_freeListPtr->prev = m_freeListLast->prev = nullptr; 
	m_freeListPtr 	= nullptr;
	m_freeListLast 	= nullptr;
	m_heapPtr 		= nullptr;

	// reset counters
	resetCounters();
}

//---------------------------------------------------------------------------------------
block_header_t* MemoryAllocator::extendHeap(size_t _heap_size)
{
	PROFILE_FUNCTION();
	
	size_t blockSize = ALIGN(_heap_size);
	size_t dataSize = DATA_SIZE(blockSize);
	
	// pointer to current heap break
	block_header_t* newMem = (block_header_t*)sbrk(0);
	
	// request memory (by extending the heap by blockSize bytes)
	if (sbrk(blockSize) == (void*)-1)
		return nullptr;

	#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
	printf("%s: allocating %zu B\n", __PRETTY_FUNCTION__, blockSize);
	printf("\taligned data size = %zu B\n", dataSize);
	#endif

	// set block fields
	SET_SIZE(newMem, dataSize);
	SET_FOOTER(newMem->data, dataSize);

	// update heap size and pointer
	if (m_heapPtr == nullptr)
		m_heapPtr = newMem;
	m_totalHeapSize += blockSize;

	// extend_heap() is responsible for adding the newly
	// allocated memory to the deallocate-list.
	addFreeBlock(newMem);
	
	#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
	debugTraverseFreeList();
	debugTraverseHeap();
	#endif

	return newMem;
}

//---------------------------------------------------------------------------------------
void* MemoryAllocator::allocate(size_t _size)
{
	PROFILE_FUNCTION();
	
	size_t asize = ALIGN(_size);
	
	#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
	printf("%s: request %zu B from heap (block size %zu).\n", __PRETTY_FUNCTION__, _size, BLOCK_SIZE(asize));
	#endif

	block_header_t* block = findFirst(asize);

	SET_SIZE_ALLOC(block, asize);
	SET_FOOTER(block, asize);
	SET_ALLOC_BIT(block);

	m_numAllocations++;

	return (void*)block->data;
}

//---------------------------------------------------------------------------------------
void MemoryAllocator::deallocate(void* _data_ptr)
{
	PROFILE_FUNCTION();
	
	// get block ptr
	block_header_t* block = GET_HEADER((word_t*)_data_ptr);

	// add back to deallocate-list
	addFreeBlock(block);

	m_numDeallocations++;
}

//---------------------------------------------------------------------------------------
void MemoryAllocator::clearHeap()
{
	PROFILE_FUNCTION();
	
	#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
	printf("%s: ", __PRETTY_FUNCTION__);
	#endif

	// pointer to beginning of the heap
	block_header_t* bptr = m_heapPtr;
	size_t traversedBytes = 0;

	// step through heap
	while (traversedBytes < m_totalHeapSize)
	{
		// step up pointer size bytes + sizeof header to get to the next block header
		size_t blockSize = BLOCK_SIZE(GET_SIZE(bptr));
		traversedBytes += blockSize;
		bptr = (block_header_t*)((char*)bptr + blockSize);
	}

	// reset free list
	m_freeListPtr = m_freeListLast = nullptr;

	block_header_t* heap = m_heapPtr;
	size_t heapDataSize = DATA_SIZE(traversedBytes);
	SET_SIZE(heap, heapDataSize);
	SET_FOOTER(heap, heapDataSize);
	addFreeBlock(heap);

	m_numHeapClears++;

	#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
	printf("\tclearHeap(): ok.\n");
	#endif
}

//---------------------------------------------------------------------------------------
void MemoryAllocator::addFreeBlock(block_header_t* _block_ptr)
{
	PROFILE_FUNCTION();
	
	#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
	printf("%s: adding block of size %zu (%p).\n", __PRETTY_FUNCTION__, BLOCK_SIZE(_block_ptr), _block_ptr);
	#endif

	block_header_t* ptr = m_freeListPtr;

	// mark as deallocate
	UNSET_ALLOC_BIT(_block_ptr);

	// list is empty, create a new list
	if (m_freeListPtr == nullptr)
	{
		#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
		printf("\tcreating free list (%p)\n", _block_ptr);
		#endif

		m_freeListPtr = m_freeListLast = _block_ptr;
		_block_ptr->prev = _block_ptr->next = nullptr;
	}
	// insert first in list if address is before the first node
	else if (_block_ptr < m_freeListPtr)
	{
		#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
		printf("\tinserting %p first in list\n", _block_ptr);
		#endif

		_block_ptr->next = m_freeListPtr;
		_block_ptr->next->prev = _block_ptr;
		m_freeListPtr = _block_ptr;
	}
	// not first and _block_ptr > m_freeListPtr
	else
	{
		ptr = m_freeListPtr;
		// locate block after which the new block is to be inserted
		while (ptr->next != nullptr && ptr < _block_ptr)
			ptr = ptr->next;
		// ptr is now pointing to the block after location of insertion,
		// however, if the free list contains only one entry, then ptr == m_freeListPtr, 
		// since ptr->next will be nullptr even if ptr < _block_ptr. _block_ptr should therefore
		// be inserted after ptr, instead of before
		if (ptr < _block_ptr)
		{
			#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
			printf("\tinserting block %p last\n", _block_ptr);
			#endif
			// set pointers
			ptr->next = _block_ptr;
			_block_ptr->prev = ptr;
			_block_ptr->next = nullptr;
			m_freeListLast = _block_ptr;
			return;
		}

		// store for later insertion of _block_ptr
		block_header_t* prev;
		if (ptr->prev != nullptr) 
			prev = ptr->prev;

		// insert _block_ptr before ptr and after ptr->prev
		#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
		printf("\tinsering %p, after %p, before %p\n", _block_ptr, prev, ptr);
		#endif

		// insert _block_ptr before ptr and after ptr->prev
		_block_ptr->next = ptr;

		// if not appending to end of list
		if (ptr == nullptr)
			m_freeListLast = _block_ptr;
		else
			ptr->prev = _block_ptr;

		// set previous pointer
		if (prev == nullptr)
			m_freeListPtr = _block_ptr;
		else
			prev->next = _block_ptr;
		
		_block_ptr->prev = prev;		
	}
}

//---------------------------------------------------------------------------------------
void MemoryAllocator::removeFreeBlock(block_header_t* _block_ptr)
{
	PROFILE_FUNCTION();
	
	#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
	printf("%s: removing block %p.\n", __PRETTY_FUNCTION__, (char*)_block_ptr);
	debugBlock(_block_ptr, true);
	#endif

	// list is empty -- invalid deletion
	if ((m_freeListPtr == nullptr) || (_block_ptr == nullptr))
		return;

	SET_ALLOC_BIT(_block_ptr);

	// first node
	if (_block_ptr == m_freeListPtr)
		m_freeListPtr = _block_ptr->next;

	// in the middle
	//
	// set next block prev ptr to the deleted blocks prev ptr.
	if (_block_ptr->next != nullptr)
		_block_ptr->next->prev = _block_ptr->prev;
	// set prev blocks next ptr to deleted blocks next ptr.
	if (_block_ptr->prev != nullptr)
		_block_ptr->prev->next = _block_ptr->next;

}

//---------------------------------------------------------------------------------------
block_header_t* MemoryAllocator::findFirst(size_t _aligned_data_size)
{
	PROFILE_FUNCTION();
	
	size_t blockSize = BLOCK_SIZE(_aligned_data_size);
	
	#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
	printf("%s: searching for block of size %zu.\n", __PRETTY_FUNCTION__, blockSize);
	#endif

	block_header_t* blockPtr = m_freeListPtr;
	while (blockPtr != nullptr)
	{
		size_t blockDataSize = GET_SIZE(blockPtr);
		// is the block big enough?
		if (blockDataSize >= _aligned_data_size)
		{
			#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
			printf("\tfound block of size %zu.\n", BLOCK_SIZE(blockPtr));
			#endif
			// can we split?
			if (BLOCK_SIZE(blockPtr) - (blockSize) > MIN_BLOCK_SIZE)
				return splitBlock(blockPtr, blockSize);
			// no split

			#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
			printf("\tblock of size %zu (not split).\n", blockDataSize);
			#endif
			
			removeFreeBlock(blockPtr);
			
			return blockPtr;
		}
		// not big enough, go to next
		blockPtr = blockPtr->next;
	}

	// if we reached this point, we are out of heap memory, and we need more.
	// extendHeap() is responsible for adding the new memory to the deallocate-list
	#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
	printf("out of heap memory: extending heap with %d bytes.\n", MEMORY_CHUNK_SIZE);
	#endif
	size_t requestedBlockSize = BLOCK_SIZE(_aligned_data_size);
	size_t extendSize = (requestedBlockSize > MEMORY_CHUNK_SIZE) ? requestedBlockSize + MEMORY_CHUNK_SIZE : MEMORY_CHUNK_SIZE;
	block_header_t* mem = extendHeap(extendSize);
	if (mem == nullptr)
	{
		#ifdef MEMORY_ALLOCATOR_STDOUT
		printf(">>> ERROR: out of memory.\n");
		#endif

		return nullptr;
	}
	
	return findFirst(_aligned_data_size);

}

//---------------------------------------------------------------------------------------
block_header_t* MemoryAllocator::splitBlock(block_header_t* _block_ptr, size_t _requested_block_size)
{
	PROFILE_FUNCTION();
	
	#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
	printf("splitting block of size %zu.\n", BLOCK_SIZE(_block_ptr));
	#endif

	// if we reached this point, the point of no return has passed,
	// i.e. no more assertions or checks.

	size_t originalBlockSize = BLOCK_SIZE(_block_ptr);
	size_t freeBlockSize = originalBlockSize - _requested_block_size;

	#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
	printf("\trequested data size = %zu\n", DATA_SIZE(_requested_block_size));
	printf("\tfree block size after split = %zu (%zu - %zu)\n", freeBlockSize, originalBlockSize, _requested_block_size);
	#endif
	//
	removeFreeBlock(_block_ptr);

	// set pointer of new block end-to-end with the removed block
	block_header_t* newFreeBlock = (block_header_t*)((char*)_block_ptr + _requested_block_size);

	// adjust size and allocation flag of new deallocate block,
	// the original block size and allocation is set in malloc().
	size_t newDataSize = DATA_SIZE(freeBlockSize);
	SET_SIZE(newFreeBlock, newDataSize);
	SET_FOOTER(newFreeBlock->data, newDataSize);
	UNSET_ALLOC_BIT(newFreeBlock);
	//
	//debugBlock(newFreeBlock);

	// finally add the new block to the deallocate list
	addFreeBlock(newFreeBlock);

	m_numBlockSplits++;

	// return the original, now split, block
	return _block_ptr;

}

//---------------------------------------------------------------------------------------
size_t MemoryAllocator::coalesceBlocks(block_header_t* _block_ptr)
{
	PROFILE_FUNCTION();
	
	#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
	printf("%s: trying to coalesce %p (block size %zu) with neighbouring blocks\n", __PRETTY_FUNCTION__, _block_ptr, BLOCK_SIZE(_block_ptr));
	#endif

	// Check the pointers in the free list:
	// 	1. 	if this->next - this->size == this --> this->next is directly after this 
	//		in the heap and deallocated (otherwise it wouldn't be in the free list)
	//	2.	if this->prev + this->prev->size == this --> this->prev is directly before
	//		this and deallocated.
	//
	// N.B. This concept rests on the fact that the free list is sorted from low to high
	// 		memory.
	//

	if (GET_ALLOC_BIT(_block_ptr) == 1)
	{
		#ifdef MEMORY_ALLOCATOR_STDOUT
		printf(">>> WARNING: block %p allocated: abort.\n", _block_ptr);
		#endif
		
		return 0;
	}

	block_header_t* nextBlock = _block_ptr->next;
	block_header_t* prevBlock = _block_ptr->prev;
	bool nextFree = false;
	bool prevFree = false;
	
	// keeps track of the beginning of the coalesced block start
	block_header_t* coalescedPtr = _block_ptr;

	// tracks coalesced total block size
	size_t coalescedBlockSize = BLOCK_SIZE(_block_ptr);

	// check for coalescing with previous block
	if (prevBlock != nullptr)
	{
		// check if previous block is directly preceding this block (i.e. is deallocated)
		if ((char*)prevBlock + BLOCK_SIZE(prevBlock) == (char*)_block_ptr)
		{
			#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
			printf("\tcoalescing with previous block:\n\t\tprevious: %p\n\t\tthis: %p\n\t\tdiff: %p + %zu = %p\n",
					prevBlock, _block_ptr, (char*)prevBlock, BLOCK_SIZE(prevBlock), (char*)prevBlock + BLOCK_SIZE(prevBlock));
			#endif

			coalescedBlockSize += BLOCK_SIZE(prevBlock);
			// set previous block as start of coalesced block
			coalescedPtr = prevBlock;
			//coalescedPtr->prev = prevBlock->prev;
			prevFree = true;
		}
	}

	// check for coalescing with next block
	if (nextBlock != nullptr)
	{
		// check if next block directly follows this block (i.e. is deallocated)
		if ((char*)nextBlock - BLOCK_SIZE(_block_ptr) == (char*)_block_ptr)
		{
			#ifdef DEBUG_MEMORY_ALLOCATOR_LOG
			printf("\tcoalescing with next block:\n\t\tthis: %p\n\t\tnext: %p\n\t\tdiff: %p - %zu = %p\n",
					_block_ptr, nextBlock, (char*)nextBlock, BLOCK_SIZE(_block_ptr), (char*)nextBlock - BLOCK_SIZE(_block_ptr));
			#endif

			coalescedBlockSize += BLOCK_SIZE(nextBlock);
			// set next pointer
			coalescedPtr->next = nextBlock->next;

			nextFree = true;
		}
	}
	
	// have any blocks been merged?
	if (prevFree || nextFree)
	{
		// update size
		size_t coalescedDataSize = DATA_SIZE(coalescedBlockSize);
		SET_SIZE(coalescedPtr, coalescedDataSize);
		SET_FOOTER(coalescedPtr, coalescedDataSize);
		UNSET_ALLOC_BIT(coalescedPtr);

		// update free list
		if (nextFree) removeFreeBlock(nextBlock);
		if (prevFree) removeFreeBlock(_block_ptr);

		m_numBlockMerges++;

		return coalescedBlockSize;
	}
	// no blocks merged
	else
		return 0;

}

//---------------------------------------------------------------------------------------
void MemoryAllocator::defragHeapCoalesce()
{
	PROFILE_FUNCTION();
	
	// step through the free list, coalesce all blocks
	block_header_t* ptr = m_freeListPtr;
	while (ptr != nullptr)
	{
		size_t res = coalesceBlocks(ptr);
		if (res > 0)
			break;
		ptr = ptr->next;
	}
}

//---------------------------------------------------------------------------------------
void MemoryAllocator::defragHeapReallocate()
{
	PROFILE_FUNCTION();
	
	// Find the first allocated block that is following a free block and reallocate it
	// to the start of the heap.
}

//---------------------------------------------------------------------------------------
void MemoryAllocator::debugTraverseHeap(bool _debug_footer)
{
	printf("%s:\n", __PRETTY_FUNCTION__);

	block_header_t* blockPtr = m_heapPtr;
	size_t traversedBytes = 0;

	// table header
	std::cout << '\t' << std::right << std::setw(20) << "address" 
									<< std::setw(12) << "offset" 
									<< std::setw(12) << "alloc"
									<< std::setw(12) << "block_size"
									<< std::setw(12) << "data_size";
	if (_debug_footer)
		   std::cout << std::right << std::setw(12) << "ftr";
	std::cout << std::endl;

	// table rows
	while (traversedBytes < m_totalHeapSize)
	{
		// output debug info
		debugBlock(blockPtr, _debug_footer);

		// step up pointer size bytes + sizeof header to get to the next block header
		size_t blockSize = BLOCK_SIZE(GET_SIZE(blockPtr));
		traversedBytes += blockSize;
		blockPtr = (block_header_t*)((char*)blockPtr + blockSize);
	}

	printf("heap size: %zu\n", traversedBytes);

}

//---------------------------------------------------------------------------------------
void MemoryAllocator::debugTraverseFreeList(bool _debug_footer)
{
	printf("%s:\n", __PRETTY_FUNCTION__);
	printf("\tdeallocate list first ptr: %p\n", m_freeListPtr);
	printf("\tdeallocate list last ptr:  %p\n", m_freeListLast);

	// table header
	std::cout << '\t' << std::right << std::setw(20) << "address" 
									<< std::setw(12) << "offset" 
									<< std::setw(12) << "alloc"
									<< std::setw(12) << "block_size"
									<< std::setw(12) << "data_size";
	if (_debug_footer)
		   std::cout << std::right << std::setw(12) << "ftr";
	std::cout << std::endl;
	// table rows
	block_header_t* blockPtr = m_freeListPtr;
	size_t freeListSize = 0;
	while (blockPtr != nullptr)
	{
		debugBlock(blockPtr, _debug_footer);
		freeListSize += BLOCK_SIZE(blockPtr);
		blockPtr = blockPtr->next;
	}
	printf("\tfree list size: %zu\n", freeListSize);
}

//---------------------------------------------------------------------------------------
void MemoryAllocator::debugBlock(block_header_t* _block_ptr, bool _debug_footer)
{
	char* memStart = (char*)m_heapPtr;

	printf("\t%20p%12ld%12lu%12zu%12zu", 
		   (char*)_block_ptr, 
		   ((char*)_block_ptr - memStart), 
		   GET_ALLOC_BIT(_block_ptr), 
		   BLOCK_SIZE(_block_ptr), 
		   GET_SIZE(_block_ptr)
	);

	if (_debug_footer)
		printf("%12zu", GET_FOOTER(_block_ptr)->size);

	printf("\n");
}


