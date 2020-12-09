
#include "pch.hpp"

#include "Synapse/Renderer/RenderCommandQueue.hpp"

#include "Synapse/Debug/Profiler.hpp"


namespace Syn {


	RenderCommandQueue::RenderCommandQueue()
	{
		m_commandBuffer = new unsigned char[10 * 1024 * 1024];	// 10 MB
		m_commandBufferPtr = m_commandBuffer;
		memset(m_commandBuffer, 0, 10 * 1024 * 1024);
	}

	
	//-----------------------------------------------------------------------------------
	RenderCommandQueue::~RenderCommandQueue()
	{ 
		delete[] m_commandBuffer; 
	}


	//-----------------------------------------------------------------------------------
	void* RenderCommandQueue::allocate(RenderCommandFn _fnc, unsigned int _size)
	{
		*(RenderCommandFn*)m_commandBufferPtr = _fnc;
		m_commandBufferPtr += sizeof(RenderCommandFn);

		*(int*)m_commandBufferPtr = _size;
		m_commandBufferPtr += sizeof(unsigned int);

		void* memory = m_commandBufferPtr;
		m_commandBufferPtr += _size;

		m_commandCount++;

		return memory;
	}


	//-----------------------------------------------------------------------------------
	void RenderCommandQueue::execute()
	{
		SYN_PROFILE_FUNCTION();

		#if (defined DEBUG_ONE_FRAME) || (defined DEBUG_RENDER_COMMAND_QUEUE)		
			SYN_CORE_TRACE("RenderCommandQueue::execute -- ", m_commandCount, " commands, ", m_commandBufferPtr - m_commandBuffer, " bytes.");
		#endif

		unsigned char* buffer = m_commandBuffer;

		for (size_t i = 0; i < m_commandCount; i++)
		{
			RenderCommandFn function = *(RenderCommandFn*)buffer;
			buffer += sizeof(RenderCommandFn);

			unsigned int size = *(unsigned int*)buffer;
			buffer += sizeof(unsigned int);

			function(buffer);

			buffer += size;
		}

		m_commandBufferPtr = m_commandBuffer;
		m_commandCount = 0;
	}

}