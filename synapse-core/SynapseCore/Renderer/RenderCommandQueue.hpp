#pragma once


#include "../Core.hpp"


namespace Syn {


	class RenderCommandQueue
	{
	public:
		typedef void(*RenderCommandFn)(void*);

		RenderCommandQueue();
		~RenderCommandQueue();

		void* allocate(RenderCommandFn _fnc, unsigned int _size);
		void execute();

	private:
		unsigned char* m_commandBuffer;
		unsigned char* m_commandBufferPtr;
		uint32_t m_commandCount = 0;
	};


}

