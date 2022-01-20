
#pragma once

#include <vector>
#include <list>
#include <map>
#include <unordered_map>

#include "Synapse/Memory/MemoryLog.hpp"
#include "Synapse/Memory/MemoryRsrc.hpp"
#include "Synapse/Core.hpp"


namespace Syn
{
	// The following macros are available:
	// 
	//	STL container creation:
	// 	SYN_VECTOR						std::vector<T>
	// 	SYN_LIST						std::list<T>
	// 	SYN_MAP							std::map<K, T>
	// 	SYN_UNORDERED_MAP				std::unordered_map<K, T>
	// 
	// 	Example usage: Syn::vector<int> v = SYN_VECTOR(T);
	// 
	// Shared pointer creation
	// 	SYN_MAKE_REF(T, ...)			std::shared_ptr<T>(...)
	// 
	// 	Example usage: Syn::Ref<Syn::Texture2D> m_texture = SYN_MAKE_REF(Syn::Texture2D, ...(args)...);
	// 
	// Explicit, raw allocations
	// 	SYN_ALLOCATE(T, ...)			new T
	// 	SYN_ALLOCATE_N(T, n)			new[] T[n]
	// 	SYN_DEALLOCATE(T, ...)			delete
	// 	SYN_DEALLOCATE_N(T, ...)		delete[]
	// 
	// 	Example usage: int* i = SYN_ALLOCATE(int, ...);
	// 				   // a 1000 ints!
	// 				   int* ii = SYN_ALLOCATE_N(int, 1000);


	// Function pointer for malloc_size_func (linux) or _msize (windows).
	typedef size_t (*malloc_size_func_)(void*);
	extern malloc_size_func_ malloc_size_func;


	// Aliases (for convenience)
	template<typename T>
	using pmr_alloc = std::pmr::polymorphic_allocator<T>;
	using mem_rsrc = std::pmr::memory_resource;


	// STL containers
	//
	#ifdef DEBUG_MEMORY_ALLOC
	
		// memory resource used for allocation and tracking
		extern MemoryResource* s_memorySTL;

		// templated aliases for STL container declaration
		template<typename T> 
		using vector = std::vector<T, pmr_alloc<T>>;
		template<typename T>
		using list = std::list<T, pmr_alloc<T>>;
		template<typename K, typename T>
		using map = std::map<K, T, std::less<K>, pmr_alloc<std::pair<const K, T>>>;
		template<typename K, typename T>
		using unordered_map = std::unordered_map<K, T, std::hash<K>, std::equal_to<K>, pmr_alloc<std::pair<const K, T>>>;

		#ifdef DEBUG_MEMORY_STL_ALLOC
			// vector
			template<typename T>
			static inline vector<T> _syn_vector(const char* _c_file, const char* _c_line, const char* _c_fnc)
			{
				MemoryResource* rsrc = s_STLMemRsrcHandler.getNewMemoryResource();
				rsrc->set_caller_signature(getCallerSignature(_c_file, _c_line, _c_fnc, "Syn::vector<>"));
				vector<T> v(0, rsrc);
				return v;
			}
			// list
			template<typename T>
			static inline list<T> _syn_list(const char* _c_file, const char* _c_line, const char* _c_fnc)
			{
				MemoryResource* rsrc = s_STLMemRsrcHandler.getNewMemoryResource();
				rsrc->set_caller_signature(getCallerSignature(_c_file, _c_line, _c_fnc, "Syn::list<>"));
				list<T> l(rsrc);
				return l;
			}
			// map
			template<typename K, typename T>
			static inline map<K, T> _syn_map(const char* _c_file, const char* _c_line, const char* _c_fnc)
			{
				MemoryResource* rsrc = s_STLMemRsrcHandler.getNewMemoryResource();
				rsrc->set_caller_signature(getCallerSignature(_c_file, _c_line, _c_fnc, "Syn::map<>"));
				map<K, T> m(rsrc);
				return m;
			}
			// unordered_map
			template<typename K, typename T>
			static inline unordered_map<K, T> _syn_unordered_map(const char* _c_file, const char* _c_line, const char* _c_fnc)
			{
				MemoryResource* rsrc = s_STLMemRsrcHandler.getNewMemoryResource();
				rsrc->set_caller_signature(getCallerSignature(_c_file, _c_line, _c_fnc, "Syn::unordered_map<>"));
				unordered_map<K, T> um(rsrc);
				return um;
			}
		#else // not DEBUG_MEMORY_STL_ALLOC
			template<typename T>
			static inline vector<T> _syn_vector()
			{
				vector<T> v(0, s_memorySTL);
				return v;
			}
			// list
			template<typename T>
			static inline list<T> _syn_list()
			{
				list<T> l(s_memorySTL);
				return l;
			}
			// map
			template<typename K, typename T>
			static inline map<K, T> _syn_map()
			{
				map<K, T> m(s_memorySTL);
				return m;
			}
			// unordered_map
			template<typename K, typename T>
			static inline unordered_map<K, T> _syn_unordered_map()
			{
				unordered_map<K, T> um(s_memorySTL);
				return um;
			}
		#endif // DEBUG_MEMORY_STL_ALLOC
	
	#else // not DEBUG_MEMORY_ALLOC -- Syn::containers are just aliases for std::containers
		template<typename T>
		using vector = std::vector<T>;
		template<typename T>
		using list = std::list<T>;
		template<typename K, typename T>
		using map = std::map<K, T>;
		template<typename K, typename T>
		using unordered_map = std::unordered_map<K, T>;
	#endif // DEBUG_MEMORY_ALLOC


	// Wrappers for std::allocate_shared<>.
	//

	// Basic typedef used in Synapse.
	template<typename T>
	using Ref = std::shared_ptr<T>;

	// TODO: remove when this has been replaced by SYN_MAKE_REF everywhere...
	template<typename T>
	inline Ref<T> MakeRef() __attribute__((always_inline));
	template<typename T, typename ...Args>
	inline Ref<T> MakeRef(Args ...args) __attribute__((always_inline));

	template<typename T>
	inline Ref<T> MakeRef() { return std::make_shared<T>(); }
	template<typename T, typename ...Args>
	inline Ref<T> MakeRef(Args ...args) { return std::make_shared<T>(args...); }

	// The memory resource used for allocation.
	extern MemoryResource* s_memoryAllocShared;

	#ifdef DEBUG_MEMORY_ALLOC
		template<typename T>
		static inline std::shared_ptr<T> _make_shared(const char* _c_file, 
													  const char* _c_line, 
													  const char* _c_fnc)
		{
			s_memoryAllocShared->set_caller_signature(getCallerSignature(_c_file, _c_line, _c_fnc, "std::shared_ptr<>"));
			pmr_alloc<T> alloc(s_memoryAllocShared);
			std::shared_ptr<T> ptr = std::allocate_shared<T>(alloc);
			s_memoryAllocShared->set_caller_signature("");
			return ptr;
		}

		template<typename T, typename ...Args>
		static inline std::shared_ptr<T> _make_shared(const char* _c_file, 
													  const char* _c_line, 
													  const char* _c_fnc,
													  Args ...args)
		{
			s_memoryAllocShared->set_caller_signature(getCallerSignature(_c_file, _c_line, _c_fnc, "std::shared_ptr<>"));
			pmr_alloc<T> alloc(s_memoryAllocShared);
			std::shared_ptr<T> ptr = std::allocate_shared<T>(alloc, args...);
			s_memoryAllocShared->set_caller_signature("");
			return ptr;
		}
	#else
		template<typename T, typename ...Args> static inline std::shared_ptr<T> _shared_ptr(Args ...args) { return std::make_shared<T>(args...); }
		template<typename T> static inline std::shared_ptr<T> _shared_ptr() { return std::make_shared<T>(); }
	#endif


	// Wrappers for unique smart pointers, for completeness.
	// Currently no unique pointers are used.
	//
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	inline Scope<T> MakeScope() { return std::make_unique<T>(); }

	template<typename T, typename ...Args>
	inline Scope<T> MakeScope(Args ...args) { return std::make_unique<T>(args...); }


	// Wrappers for explicit memory allocation.
	//
	#ifdef DEBUG_MEMORY_ALLOC
	
		template<typename T, typename ...Args>
		static inline T* _allocate(const char* _c_file, const char* _c_line, const char* _c_fnc, Args ...args)
		{ 
			T* ptr = new T(args...);
			void* void_ptr = reinterpret_cast<void*>(ptr);
			MemoryLog::insert(void_ptr, 
							  sizeof(T), 
							  malloc_size_func(void_ptr), 
							  AllocType::EXPLICIT, 
							  getCallerSignature(_c_file, _c_line, _c_fnc, "new(...)"));
			return ptr;
		}

		template<typename T>
		static inline T* _allocate(const char* _c_file, const char* _c_line, const char* _c_fnc)
		{ 
			T* ptr = new T;
			void* void_ptr = reinterpret_cast<void*>(ptr);
			MemoryLog::insert(void_ptr, 
							  sizeof(T), 
							  malloc_size_func(void_ptr), 
							  AllocType::EXPLICIT,
							  getCallerSignature(_c_file, _c_line, _c_fnc, "new"));
			return ptr;
		}

		template<typename T>
		static inline T* _allocate_array(const char* _c_file, const char* _c_line, const char* _c_fnc, const std::size_t& _n)
		{ 
			T* ptr = new T[_n];
			void* void_ptr = reinterpret_cast<void*>(ptr);
			MemoryLog::insert(void_ptr, 
							  sizeof(T) * _n, 
							  malloc_size_func(void_ptr), 
							  AllocType::EXPLICIT,
							  getCallerSignature(_c_file, _c_line, _c_fnc, "new[]"));
			return ptr;
		}

		template<typename T>
		static inline void _deallocate(T* _ptr) 
		{
			void* void_ptr = reinterpret_cast<void*>(_ptr);
			MemoryLog::remove(void_ptr, MemoryLog::getAllocBytes(void_ptr), malloc_size_func(void_ptr), AllocType::EXPLICIT);
			delete _ptr;
		}

		template<typename T>
		static inline void _deallocate_n(T* _ptr)
		{
			void* void_ptr = reinterpret_cast<void*>(_ptr);
			MemoryLog::remove(void_ptr, MemoryLog::getAllocBytes(void_ptr), malloc_size_func(void_ptr), AllocType::EXPLICIT);
			delete[] _ptr; 
		}
	#else
		template<typename T, typename ...Args> static inline T* _allocate(Args ...args) { return new T(args...); }
		template<typename T> static inline T* _allocate() { return new T; }
		template<typename T> static inline T* _allocate_n(const std::size_t& _n) {  return new T[_n]; }
		template<typename T> static inline void _deallocate(T* _ptr) { delete _ptr; }
		template<typename T> static inline void _deallocate_n(T* _ptr) { delete[] _ptr; }
	#endif // DEBUG_MEMORY_ALLOC

}


