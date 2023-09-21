
#include "../../pch.hpp"

#include "MemoryRsrc.hpp"


namespace Syn {

	proc_mem_info_t ProcessInfo::m_procMemInfo;
	const char* ProcessInfo::m_fmtSize;
	uint32_t ProcessInfo::m_pageSize_kB;	

	MemoryResource* s_memorySTL 		= new MemoryResource(MemoryLog::insert, MemoryLog::remove, AllocType::STL);
	MemoryResource* s_memoryAllocShared = new MemoryResource(MemoryLog::insert, MemoryLog::remove, AllocType::SHARED);
	//MemoryResource* s_memoryExplicit 	= new MemoryResource(MemoryLog::insert, MemoryLog::remove, AllocType::EXPLICIT);

	STLMemoryResourceHandler s_STLMemRsrcHandler;
	
}