
#include "../../../pch.hpp"

#include "VertexBuffer.hpp"
#include "../Renderer.hpp"


namespace Syn {


	//-----------------------------------------------------------------------------------
	VertexBuffer::VertexBuffer(GLenum _usage)
	{
		m_usage = _usage;
		SYN_RENDER_S0({
			glGenBuffers(1, &self->m_bufferID);
		});
	}
	
	//-----------------------------------------------------------------------------------
	VertexBuffer::~VertexBuffer()
	{
		SYN_RENDER_S0({
			glDeleteBuffers(1, &self->m_bufferID);
		});
	}

	//-----------------------------------------------------------------------------------
	void VertexBuffer::setData(void* _data, uint32_t _size_in_bytes)
	{
		SYN_RENDER_S2(_data, _size_in_bytes, {
			glBindBuffer(GL_ARRAY_BUFFER, self->m_bufferID);
			glBufferData(GL_ARRAY_BUFFER, _size_in_bytes, _data, self->m_usage);
		});
	}

	//-----------------------------------------------------------------------------------
	void VertexBuffer::updateBufferData(void* _data, uint32_t _size_in_bytes, uint32_t _offset)
	{
		SYN_RENDER_S3(_data, _size_in_bytes, _offset, {
			glBindBuffer(GL_ARRAY_BUFFER, self->m_bufferID);
			glBufferSubData(GL_ARRAY_BUFFER, _offset, _size_in_bytes, _data);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		});
	}

	//-----------------------------------------------------------------------------------
	void VertexBuffer::startDataBlock(uint32_t _total_size_in_bytes)
	{
		SYN_RENDER_S1(_total_size_in_bytes, {
			glBindBuffer(GL_ARRAY_BUFFER, self->m_bufferID);
			glBufferData(GL_ARRAY_BUFFER, _total_size_in_bytes, nullptr, self->m_usage);
		});
	}

	//-----------------------------------------------------------------------------------
	void VertexBuffer::addSubData(void* _data, uint32_t _size_in_bytes, uint32_t _offset_in_bytes)
	{
		SYN_RENDER_S3(_data, _size_in_bytes, _offset_in_bytes, {
			glBufferSubData(GL_ARRAY_BUFFER, _offset_in_bytes, _size_in_bytes, _data);
		});
	}

	//-----------------------------------------------------------------------------------
	void VertexBuffer::endDataBlock()
	{
		SYN_RENDER_S0({ 
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		});
	}

	//-----------------------------------------------------------------------------------
	const void VertexBuffer::bind() const
	{
		SYN_RENDER_S0({
			glBindBuffer(GL_ARRAY_BUFFER, self->m_bufferID);
		});
	}
	
	//-----------------------------------------------------------------------------------
	const void VertexBuffer::unbind() const
	{
		SYN_RENDER_S0({
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		});
	}

}
