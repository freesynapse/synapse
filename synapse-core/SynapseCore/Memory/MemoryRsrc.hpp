
#pragma once


#include <memory_resource> // std::pmr::
#include <unistd.h> // sysconf()
#include <fstream>

#include "MemoryLog.hpp"
#include "../Debug/Log.hpp"


namespace Syn {


    // Tracking of total heap usage. Stores the heap break pointer at initialization
    // and subsequently calculates the offset on request; this ought to correspond to
    // the total dynamic allocations.
    //
    typedef struct proc_mem_info_ proc_mem_info_t;
    struct proc_mem_info_
    {
		// all sizes are in kilobytes
		uint32_t virtual_size;
		uint32_t rss_size;
		uint32_t shared_size;
    };

    class ProcessInfo
    {
    public:
		static void init()	{	m_pageSize_kB = sysconf(_SC_PAGE_SIZE) / 1024;	}

		// accessors (size_t)
		static uint32_t getSizeVirtual_kB() 	{	_updateProcMemInfo(); return m_procMemInfo.virtual_size; 	}
		static uint32_t getSizeRSS_kB() 		{	_updateProcMemInfo(); return m_procMemInfo.rss_size; 		}
		static uint32_t getSizeShared_kB() 		{	_updateProcMemInfo(); return m_procMemInfo.shared_size; 	}

		// accessors (const char*)
		static const char* getSizeVirtual_cchar() 	{	_updateProcMemInfo(); _sz(m_procMemInfo.virtual_size);	return m_fmtSize;	}
		static const char* getSizeRSS_cchar() 		{	_updateProcMemInfo(); _sz(m_procMemInfo.rss_size); 	 	return m_fmtSize;	}
		static const char* getSizeShared_cchar() 	{	_updateProcMemInfo(); _sz(m_procMemInfo.shared_size);  	return m_fmtSize;	}

    private:
		static void _updateProcMemInfo() 
		{
			uint32_t size, rss, shared;
			{
			std::ifstream ifs("/proc/self/statm", std::ios_base::in);
			ifs >> size >> rss >> shared;
			}
			// all sizes are in pages, each page is m_pageSize_kB kilobytes
			// long, hence:
			m_procMemInfo.virtual_size = size * m_pageSize_kB;
			m_procMemInfo.rss_size     = rss * m_pageSize_kB;
			m_procMemInfo.shared_size  = shared * m_pageSize_kB;
		}

		static void _sz(uint32_t _size_kb)
		{
			std::ostringstream ss;
			if (_size_kb < 1024)		ss << _size_kb << " kB";
			else						ss << std::fixed << std::setprecision(2) << (float)_size_kb / 1024.0f << " MB";
			m_fmtSize = ss.str().c_str();
		}

    private:
		static proc_mem_info_t m_procMemInfo;
		static const char* m_fmtSize;
		static uint32_t m_pageSize_kB;
    };



    // Explicit tracking of heap allocations, for STL containers, raw allocations and 
    // shared pointer allocations. 
    //

    // Polymorfic resource override, using the global new/delete
    // memory resource (i.e. the global heap): new_delete_resource().
    class MemoryResource : public std::pmr::memory_resource
    {
    public:
		MemoryResource() { m_memory = std::pmr::new_delete_resource(); }
		explicit MemoryResource(insert_func _insert_func,
								remove_func _remove_func,
								AllocType _malloc_type = AllocType::NONE,
								std::pmr::memory_resource* _memory=std::pmr::new_delete_resource())
		{
			m_allocType = _malloc_type;
			m_insertFunc = _insert_func;
			m_removeFunc = _remove_func;
			m_memory = _memory;
		}
		// override the allocation and deallocation functions
		void* do_allocate(std::size_t _bytes, 
				std::size_t _alignment=alignof(std::max_align_t)) override
		{
			void* ptr = m_memory->allocate(_bytes, _alignment);
			m_insertFunc(ptr, _bytes, malloc_size_func((void*)ptr), m_allocType, m_lastCaller);
			assert(m_allocType != AllocType::NONE);
			return ptr;
		}

		void do_deallocate(void* _ptr, 
				std::size_t _bytes, 
				std::size_t _alignment=alignof(std::max_align_t)) override
		{
			assert(m_allocType != AllocType::NONE);
			m_removeFunc(_ptr, _bytes, malloc_size_func(_ptr), m_allocType);
			m_memory->deallocate(_ptr, _bytes, _alignment);
		}

		bool do_is_equal(const std::pmr::memory_resource& _other) const noexcept override { return this == &_other; }

		// set caller signature
		void set_caller_signature(const std::string& _caller_sig) { m_lastCaller = _caller_sig; }


    private:
		// type of allocation, according to AllocType enum class.
		AllocType m_allocType;
		// function pointers to the insert and remove functions of the memory map
		insert_func m_insertFunc = nullptr;
		remove_func m_removeFunc = nullptr;
		// caller signature
		std::string m_lastCaller = "";
		// pointer to the global heap
		std::pmr::memory_resource* m_memory = nullptr;
    };


    // STL memory resource handler:
    // Everytime a Syn::vector<> is instantiated, a new MemoryResource is needed to
    // track reallocations for that vector. Since the memory is allocated before it's
    // copied when the vector is dynamically extended, the previuos address' signature
    // is unavailable. To circumvent this, a new MemoryResource is created everytime
    // a new vector is requested, a new MemoryResource is created and stored in this
    // handler. This will increase the memory footprint of the vector by 16 bytes;
    // 8 bytes for the MemoryResource pointer in the vector in the current class and 8
    // 8 bytes for the MemoryResource instance itself.
    // Upon application shutdown, the destructor handles deallocation of the heap-
    // allocated MemoryResource instances.
    class STLMemoryResourceHandler
    {
    public:
		STLMemoryResourceHandler() {};
		~STLMemoryResourceHandler()
		{
			for (auto i : m_rsrcs)
			delete i;
		}
		// Return a new MemoryResource and out store address for 
		// deallocation upon destruction of this.
		MemoryResource* getNewMemoryResource(insert_func _in_fnc=MemoryLog::insert, 
							remove_func _rm_fnc=MemoryLog::remove, 
							AllocType _alloc_type=AllocType::STL)
		{
			MemoryResource* ptr = new MemoryResource(_in_fnc, _rm_fnc, _alloc_type);
			m_rsrcs.push_back(ptr);
			return ptr;
		}
		// Return the memory footprint of this class and all created pointers 
		// and the MemoryResource:s they point to.
		constexpr std::size_t getMemSize() const
		{ return sizeof(STLMemoryResourceHandler) + m_rsrcs.size() * (sizeof(MemoryResource*) + sizeof(MemoryResource)); }

    private:
		std::vector<MemoryResource*> m_rsrcs;

    };

    // Global handler instance.
    extern STLMemoryResourceHandler s_STLMemRsrcHandler;




}
