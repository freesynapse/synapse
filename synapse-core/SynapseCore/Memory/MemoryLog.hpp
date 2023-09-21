
#pragma once


#include "../Core.hpp"


namespace Syn {

    // In essence, three 'different' types of memory allocations are handled;
    //
    // 1. 	STL container allocations are tracked through the use of a 
    // 		std::polymorphic_allocator<T> pointing to a custom pmr::memory_resource
    // 		which in turn, upon allocation and deallocation, calls the insert and 
    // 		remove functions of the memory_log class.
    // 
    // 2.	Shared pointer instantiation, in turn aliased as Ref (in Synapse/
    // 		Memory.hpp). Here, the custom pmr::memory_resource in Synapse/Memory/
    // 		MemoryRsrc.hpp (same as for STL containers) are utilized through a call
    // 		to std::allocate_shared<T> using a pmr::polymorphic_allocator.
    // 
    // 3.  Regular calls to new/delete are overridden through the use of new inline
    // 		functions, Syn::allocate, Syn::deallocate, and variations thereof.
    // 		Allocations are tracked through direct calls to memory_log::insert and 
    // 		memory_log::remove.
    // 
    // MemoryLog.hpp:
    // 		The static MemoryLog class and helper classes.
    //
    // MemoryRsrc.hpp:
    // 		The custom pmr::memory_resource class. Also a STLMemoryResourceHandler
    // 		class, for tracking instantiations and reallocations of STL containers.
    // 		This latter feature is controlled through the DEBUG_MEMORY_STL_ALLOC
    // 		#define in Synapes/Core.hpp.
    // 
    // MemoryTypes.hpp:
    // 	All memory request overrides/custom types for STL containers,
    // 	shared pointers and new/delete requests.


    // Function pointer for malloc_size_func (linux) or _msize (windows).
    //
    typedef size_t (*malloc_size_func_)(void*);
    extern malloc_size_func_ malloc_size_func;


    // Output formatting helper functions.
    //
    extern std::string prettyFunc(const char* _fnc);
    extern std::string getCallerSignature(const char* _c_file, const char* _c_line, const char* _c_fnc, const char* _c_type="");
    extern std::string formatMemAddr(void* _mem_addr, uint8_t _width=16, char _fill='_');


    // Enumeration to deduce the type of memory allocation.
    //
    enum class AllocType
    {
		STL		 = 0,
		SHARED	 = 1,
		EXPLICIT = 2,
		NONE 	 = 3
    };


    // Helper function for outputting memory allocation type.
    //
    static inline std::string allocTypeStr(AllocType _alloc_type)
    {
		switch (_alloc_type)
		{
			case AllocType::STL:		return "AllocType::STL";
			case AllocType::SHARED:		return "AllocType::SHARED";
			case AllocType::EXPLICIT:	return "AllocType::EXPLICIT";
			case AllocType::NONE: 		return "AllocType::NONE";
		}
		return "AllocType::NONE";
	}


	
    // Allocation info struct, stores actual and virtual allocations 
    // and deallocations in m_allocBytes/m_deallocBytes and m_deallocBlock/
    // m_deallocBlock, respectively. The struct is mapped to a void* memory
    // address in the static memory_log class.
    typedef struct memory_alloc_info_t
    {
		uint32_t m_allocBytes;		// raw bytes
		uint32_t m_allocBlock;		// block-aligned bytes
		uint32_t m_deallocBytes;
		uint32_t m_deallocBlock;
		AllocType m_allocType;		// type of allocation
		std::string m_callerFnc;	// caller signature

		memory_alloc_info_t() : 
			m_allocBytes(0), 
			m_allocBlock(0),
			m_deallocBytes(0), 
			m_deallocBlock(0),
			m_allocType(AllocType::NONE),
			m_callerFnc("")
		{}

		memory_alloc_info_t(uint32_t _alloc_bytes,
					uint32_t _alloc_block,
					uint32_t _dealloc_bytes=0,
					uint32_t _dealloc_block=0,
					AllocType _alloc_type=AllocType::NONE,
					const std::string& _caller_fnc="") :
			m_allocBytes(_alloc_bytes), 
			m_allocBlock(_alloc_block),
			m_deallocBytes(_dealloc_bytes), 
			m_deallocBlock(_dealloc_block),
			m_allocType(_alloc_type),
			m_callerFnc(_caller_fnc)
		{}
    } memory_alloc_info_t;


    // Helper struct for the memory_log. Actual and virtual (block-aligned)
    // allocation sizes are stored.
    typedef struct memory_usage_t
    {
		uint32_t m_physicalAlloc;
		uint32_t m_virtualAlloc;
		uint32_t m_physicalDealloc;
		uint32_t m_virtualDealloc;

		memory_usage_t() :
			m_physicalAlloc(0), m_virtualAlloc(0),
			m_physicalDealloc(0), m_virtualDealloc(0)
		{}

		inline void updateAlloc(uint32_t _mem_bytes, uint32_t _mem_block) 
		{ 
			m_physicalAlloc += _mem_bytes; 
			m_virtualAlloc  += _mem_block;
		}
		inline void updateDealloc(uint32_t _mem_bytes, uint32_t _mem_block)
		{
			m_physicalDealloc += _mem_bytes;
			m_virtualDealloc  += _mem_block;
		}
    } memory_usage_t;


    // The memory tracking log. Every time memory is allocated, a call to the insert
    // function is made. This stores a memory_alloc_info struct under the address
    // (_mem_addr). In addition, the size of the allocation is added to the respective
    // memory_usage variable, one for each type of allocation (according to the
    // AllocType enum class).
    class MemoryLog
    {
    public:
		// Insert a new allocation into record.
		static void insert(void* _mem_addr, uint32_t _alloc_bytes, uint32_t _alloc_block, AllocType _alloc_type, const std::string& _caller_fnc);
		// Remove (deallocation) an allocation from record.
		static void remove(void* _mem_addr, uint32_t _dealloc_bytes, uint32_t _dealloc_block, AllocType _alloc_type);

		// Heap usage accessors.
		static const memory_usage_t& getUsageAllocType(AllocType _alloc_type) { return s_usageType[(int)_alloc_type]; }
		static const memory_usage_t& getUsageTotal() { return s_usageTotal; }
			
		// Get memory allocations, sorted on AllocType, as a string (table).
		static const std::string& strAllocAll(bool _omit_deallocated=true, bool _use_std_out=false);

		// Get memory allocations of AllocType _alloc_type as a string.
		static std::string strAllocType(AllocType _alloc_type, bool _omit_deallocated);
			
		// Get allocated bytes at memory adress _mem_addr.
		static uint32_t getAllocBytes(void* _mem_addr);

    public:
		// Formatting of bytes into KB and MB.
		static inline const std::string& _fmt_sz(uint32_t _bytes)
		{
			static constexpr uint32_t mb = 1024 * 1024;
			std::ostringstream ss;
			if (_bytes <= 1024)			ss << _bytes << " B";
			else if (_bytes < mb) 		ss << std::fixed << std::setprecision(2) << (float)_bytes / 1024.0f << " kB";
			else if (_bytes < 1024*mb)	ss << std::fixed << std::setprecision(2) << (float)_bytes / (float)mb << " MB";
			else						ss << std::fixed << std::setprecision(2) << (float)_bytes / ((float)mb * 1024.0f) << " GB";
			s_sz = ss.str();
			return s_sz;
		}

    private:
		static std::unordered_map<void*, memory_alloc_info_t> s_memory;
		static memory_usage_t s_usageType[4];
		static memory_usage_t s_usageTotal;
		static std::string s_sz;
		static std::string s_lastLogEntry;
    };


    // MemoryLog insert and remove function pointers.
    //
    typedef void (*insert_func)(void*, uint32_t, uint32_t, AllocType, const std::string&);
    typedef void (*remove_func)(void*, uint32_t, uint32_t, AllocType);


}

