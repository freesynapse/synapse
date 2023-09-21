
#include "MemoryTypes.hpp"


namespace Syn {

	// function pointer for malloc_size_func (linux) or _msize (windows)
	#ifdef __linux__
		malloc_size_func_ malloc_size_func = malloc_usable_size;
	#elif defined(_WIN32)
		malloc_size_func_ malloc_size_func = _msize;
	#endif

}
