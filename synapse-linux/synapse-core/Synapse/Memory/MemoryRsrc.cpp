
#include "pch.hpp"

#include "Synapse/Memory/MemoryRsrc.hpp"


namespace Syn {

	MemoryResource* s_memorySTL 		= new MemoryResource(memory_log::insert, memory_log::remove, AllocType::STL);
	MemoryResource* s_memoryAllocShared = new MemoryResource(memory_log::insert, memory_log::remove, AllocType::SHARED);
	//MemoryResource* s_memoryExplicit 	= new MemoryResource(memory_log::insert, memory_log::remove, AllocType::EXPLICIT);

	STLMemoryResourceHandler s_STLMemRsrcHandler;
	
}