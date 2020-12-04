
#include "pch.hpp"

#include "Synapse/Renderer/Buffers/IndexBuffer.hpp"
#include "Synapse/Renderer/Renderer.hpp"


namespace Syn {

	
	IndexBuffer::IndexBuffer(GLenum _primitive, GLenum _usage) :
		m_primitiveType(_primitive), m_usage(_usage)
	{
		SYN_RENDER_S0({
			glGenBuffers(1, &self->m_bufferID);
		});
	}


	//-----------------------------------------------------------------------------------
	IndexBuffer::~IndexBuffer()
	{
		SYN_RENDER_S0({
			glDeleteBuffers(1, &self->m_bufferID);
		});
	}


	//-----------------------------------------------------------------------------------
	void IndexBuffer::setData(void* _data, uint32_t _num_uint32_t)
	{
		m_numIndices = _num_uint32_t;
		SYN_RENDER_S2(_data, _num_uint32_t, {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->m_bufferID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, _num_uint32_t * sizeof(uint32_t), _data, self->m_usage);
		});
	}


	//-----------------------------------------------------------------------------------
	const void IndexBuffer::bind() const
	{
		SYN_RENDER_S0({
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->m_bufferID);
		});
	}


	//-----------------------------------------------------------------------------------
	const void IndexBuffer::unbind() const
	{
		SYN_RENDER_S0({
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		});
	}

}
