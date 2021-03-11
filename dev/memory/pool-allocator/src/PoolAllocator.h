#ifndef __POOLALLOCATOR_H
#define __POOLALLOCATOR_H


#include <stdint.h>		// uint_t:s
#include <stddef.h>		// size_t


// a fixed-size pool allocator; 
// --> for later implementation of a segregated free list allocator 
//

// Compared to the variable-size free list allocator implemented, 
// the fixed-size allocator will NOT need the following:
//	--	Headers! Since all blocks are fixed in size and the next
//		block if offset by the same size relative to the current one.
//	--	Block and data size macros (implicit).
//	--	Functions for split and merge.
//	--	Footers (since the block size is implicit)
//
// What it WILL need:
//	--	Linearly placed memory; on heap extension, since moves
//		will depend on moving blocks of data leading up to the moved
//		block upon defragmentation reallocation.
//		How reliable is malloc_usable_size()?
//
//

// debugging definitions
#define DEBUG_POOL_ALLOCATOR

// data types
typedef uint64_t word_t;
constexpr size_t WORD_SIZE = sizeof(word_t);
typedef block_ block_t;
// block struct, including 'header' of 8 bits
struct block_
{
	uint8_t alloc;	// lowest bit = allocation flag (0=free, 1=allocated)
	word_t data[1];	// payload pointer
};
// sizes
#define MEMORY_CHUNK_SIZE 1024 * 1024 * 64
#define ALIGN(_size) (((_size) + (WORD_SIZE-1)) & ~(WORD_SIZE - 1))
constexpr uint32_t BLOCK_SIZE = ALIGN(sizeof(block_t));
// pointer macros
#define NEXT_BLOCK(_block_ptr) ((char*)_block_ptr + BLOCK_SIZE)
#define PREV_BLOCK(_block_ptr) ((char*)_block_ptr - BLOCK_SIZE)


class PoolAllocator
{
public:
	void initalize(size_t _heap_size=MEMORY_CHUNK_SIZE);
	void shutdown();

	void* allocate(size_t _size);
	void deallocate(void* _data_ptr);
	
private:
	void addFreeBlock(block_t* _block_ptr);
	void removeFree(block_t* _block_ptr);
	
	word_t* findFirst();

private:
	// pointers
	block_t* g_freeListPtr;
	block_t* g_freeListLast;
	block_t* g_heapPtr;
	uint64_t g_heapSize;
};



// allocator functions
//

// Defragmentation is different from the variable size free list allocator
// in that only moves have to be performed, no merges. A move could be 
// performed as follows:
// 	-	identify a block with 
// 
// 
// 
// 
// N.B! If defragmentation as described above is to be used, only smart
// 		pointers can be used! Otherwise, addresses will become incongruent.










#endif // __POOLALLOCATOR_H
