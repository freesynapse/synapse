#pragma once

#include "pch.hpp"

#include "Synapse/Types.hpp"
#include "Synapse/Core.hpp"

#include "Synapse/Memory/MemoryLog.hpp"
#include "Synapse/Memory/MemoryRsrc.hpp"
#include "Synapse/Memory/MemoryTypes.hpp"


namespace Syn {

	// function pointer for malloc_size_func (linux) or _msize (windows)
	//
	typedef size_t (*malloc_size_func_)(void*);
	extern malloc_size_func_ malloc_size_func;

}

/* 
 * Memory allocation macros, to be used instead of Syn::MakeRef<T>, 
 * Syn::allocate<T> etc.
 */

// utility macros
//
#define STR(x) #x
#define TOSTR(x) STR(x)

#ifdef __linux__
#define FUNCSIG Syn::pretty_func(__PRETTY_FUNCTION__).c_str()
#elif defined(_WIN32)
#define FUNCSIG __FUNCSIG__
#endif


// STL container allocation macros
//
#ifdef DEBUG_MEMORY_ALLOC
	#ifdef DEBUG_MEMORY_STL_ALLOC
		#define SYN_VECTOR(T) 			Syn::_syn_vector<T>(__FILE__, TOSTR(__LINE__), FUNCSIG)
		#define SYN_LIST(T) 			Syn::_syn_list<T>(__FILE__, TOSTR(__LINE__), FUNCSIG)
		#define SYN_MAP(K, T) 			Syn::_syn_map<K, T>(__FILE__, TOSTR(__LINE__), FUNCSIG)
		#define SYN_UNORDERED_MAP(K, T) Syn::_syn_unordered_map<K, T>(__FILE__, TOSTR(__LINE__), FUNCSIG)
	#else
		#define SYN_VECTOR(T) 			Syn::_syn_vector<T>()
		#define SYN_LIST(T) 			Syn::_syn_list<T>()
		#define SYN_MAP(K, T) 			Syn::_syn_map<K, T>()
		#define SYN_UNORDERED_MAP(K, T) Syn::_syn_unordered_map<K, T>()
	#endif
#else
	#define SYN_VECTOR(T) 			std::vector<T>()
	#define SYN_LIST(T)				std::list<T>()
	#define SYN_MAP(K, T)			std::map<K, T>()
	#define SYN_UNORDERED_MAP(K, T)	std::unordered_map<K, T>()
#endif


// shared pointer allocation macros
//
#ifdef DEBUG_MEMORY_ALLOC
#define SYN_MAKE_REF(T, ...) Syn::_make_shared<T>(__FILE__, TOSTR(__LINE__), FUNCSIG, ##__VA_ARGS__)
#else
#define SYN_MAKE_REF(T, ...) std::make_shared<T>(##__VA_ARGS__)
#endif


// explicit allocation macros
//
#ifdef DEBUG_MEMORY_ALLOC
#define SYN_ALLOCATE(T, ...)		  Syn::_allocate<T>(__FILE__, TOSTR(__LINE__), FUNCSIG, ##__VA_ARGS__)
#define SYN_ALLOCATE_N(T, ...) 		  Syn::_allocate_n<T>(__FILE__, TOSTR(__LINE__), FUNCSIG, ##__VA_ARGS__)
#define SYN_DEALLOCATE(T, mem_addr)   Syn::_deallocate<T>(mem_addr);
#define SYN_DEALLOCATE_N(T, mem_addr) Syn::_deallocate_n<T>(mem_addr)
#else
#define SYN_ALLOCATE(T, ...) 		  new T(__VA_ARGS__)
#define SYN_ALLOCATE_N(T, n) 		  new T[n]
#define SYN_DEALLOCATE(T, mem_addr)   delete mem_addr
#define SYN_DEALLOCATE_N(T, mem_addr) delete[] mem_addr
#endif


