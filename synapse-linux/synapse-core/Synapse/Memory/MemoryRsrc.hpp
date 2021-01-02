
#pragma once


#include <memory_resource>

#include "Synapse/Memory/MemoryLog.hpp"


namespace Syn {

	/*
	 * Polymorfic resource override, using the global new/delete
	 * memory resource (i.e. the global heap): new_delete_resource().
	 */
 	class MemoryResource : public std::pmr::memory_resource
	{
	public:
		MemoryResource() { m_memory = std::pmr::new_delete_resource(); }
		explicit MemoryResource(insert_func _insert_func,
								remove_func _remove_func,
								AllocType _malloc_type = AllocType::NONE,
								std::pmr::memory_resource* _memory=std::pmr::new_delete_resource()) :
			m_insertFunc(_insert_func), 
			m_removeFunc(_remove_func), 
			m_allocType(_malloc_type), 
			m_memory(_memory)
		{}
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


	/* STL memory resource handler:
	 * Everytime a Syn::vector<> is instantiated, a new MemoryResource is needed to
	 * track reallocations for that vector. Since the memory is allocated before it's
	 * copied when the vector is dynamically extended, the previuos address' signature
	 * is unavailable. To circumvent this, a new MemoryResource is created everytime
	 * a new vector is requested, a new MemoryResource is created and stored in this
	 * handler. This will increase the memory footprint of the vector by 16 bytes;
	 * 8 bytes for the MemoryResource pointer in the vector in the current class and 8
	 * 8 bytes for the MemoryResource instance itself.
	 * Upon application shutdown, the destructor handles deallocation of the heap-
	 * allocated MemoryResource instances.
	 */
	class STLMemoryResourceHandler
	{
	public:
		STLMemoryResourceHandler() {};
		~STLMemoryResourceHandler()
		{
			for (auto i : m_rsrcs)
				delete i;
		}
		// return a new MemoryResource and out store address
		// for deallocation upon destruction of this.
		MemoryResource* getNewMemoryResource(insert_func _in_fnc=memory_log::insert, 
											 remove_func _rm_fnc=memory_log::remove, 
											 AllocType _alloc_type=AllocType::STL)
		{
			MemoryResource* ptr = new MemoryResource(_in_fnc, _rm_fnc, _alloc_type);
			m_rsrcs.push_back(ptr);
			return ptr;
		}
		// return the memory footprint of this class and all
		// created pointers and the MemoryResource:s they point to.
		constexpr std::size_t getMemSize() const
		{ return sizeof(STLMemoryResourceHandler) + m_rsrcs.size() * (sizeof(MemoryResource*) + sizeof(MemoryResource)); }

	private:
		std::vector<MemoryResource*> m_rsrcs;

	};
	// global handler instance.
	extern STLMemoryResourceHandler s_STLMemRsrcHandler;




}