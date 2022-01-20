#pragma once


namespace Syn {


	class IndexBuffer
	{
	public:
		IndexBuffer(GLenum _primitive=GL_TRIANGLES, GLenum _usage=GL_STATIC_DRAW);
		//IndexBuffer(uint32_t* _indices, uint32_t _num_indices, GLenum _primitive=GL_TRIANGLES, GLenum _usage=GL_STATIC_DRAW);
		~IndexBuffer();

		const void bind() const;
		const void unbind() const;

		/* _data as uint32_t* and number of indices (_num_uint32_t). */
		void setData(void* _data, uint32_t _num_uint32_t);

		const uint32_t& getIndexCount() const { return m_numIndices; }
		const GLenum& getPrimitiveType() const { return m_primitiveType; }
		const uint32_t getBufferID() const { return m_bufferID; }

	private:
		uint32_t m_bufferID = 0;
		uint32_t m_numIndices = 0;
		GLenum m_usage = GL_STATIC_DRAW;
		GLenum m_primitiveType = GL_TRIANGLES;
	};


}