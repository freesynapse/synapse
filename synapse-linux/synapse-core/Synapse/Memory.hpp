#pragma once

#include "pch.hpp"

#include "Synapse/Types.hpp"
#include "Synapse/Core.hpp"

#include "Synapse/Memory/MemoryLog.hpp"
#include "Synapse/Memory/MemoryRsrc.hpp"
#include "Synapse/Memory/MemoryTypes.hpp"


namespace Syn {

	// Function pointer for malloc_size_func (linux) or _msize (windows).
	typedef size_t (*malloc_size_func_)(void*);
	extern malloc_size_func_ malloc_size_func;

}

// Memory allocation macros, to be used instead of Syn::MakeRef<T>, 
// Syn::allocate<T> etc.
// 

// Utility macros
//
#define STR(x) #x
#define TOSTR(x) STR(x)

#ifdef __linux__
	#define FUNCSIG Syn::prettyFunc(__PRETTY_FUNCTION__).c_str()
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
	#define SYN_VECTOR(T) 				std::vector<T>()
	#define SYN_LIST(T)					std::list<T>()
	#define SYN_MAP(K, T)				std::map<K, T>()
	#define SYN_UNORDERED_MAP(K, T)		std::unordered_map<K, T>()
#endif


// Shared pointer allocation macros
//
#ifdef DEBUG_MEMORY_ALLOC
	#define SYN_MAKE_REF(T, ...) Syn::_make_shared<T>(__FILE__, TOSTR(__LINE__), FUNCSIG, ##__VA_ARGS__)
#else
	#define SYN_MAKE_REF(T, ...) std::make_shared<T>(__VA_ARGS__)
#endif


// Explicit allocation macros
//
#ifdef DEBUG_MEMORY_ALLOC
	// Allocate a single type_T. Optionally, an initializing value is passed as 2nd argument.
	#define SYN_ALLOCATE(type_T, ...)		  		Syn::_allocate<type_T>(__FILE__, TOSTR(__LINE__), FUNCSIG, ##__VA_ARGS__)
	// Allocate an array of type_T. The number of elements is passed as 2nd argument.
	#define SYN_ALLOCATE_ARRAY(type_T, ...)	  		Syn::_allocate_array<type_T>(__FILE__, TOSTR(__LINE__), FUNCSIG, ##__VA_ARGS__)
	// Deallocate memory of type_T at address of ptr.
	#define SYN_DEALLOCATE(type_T, ptr)   			Syn::_deallocate<type_T>(ptr);
	// Deallocate an array of type_T at address of ptr.
	#define SYN_DEALLOCATE_ARRAY(type_T, ptr) 		Syn::_deallocate_array<type_T>(ptr)
#else
	#define SYN_ALLOCATE(type_T, ...) 				new type_T(__VA_ARGS__)
	#define SYN_ALLOCATE_ARRAY(type_T, n)			new type_T[n]
	#define SYN_DEALLOCATE(type_T, ptr)   			delete ptr
	#define SYN_DEALLOCATE_ARRAY(type_T, ptr)		delete[] ptr
#endif


