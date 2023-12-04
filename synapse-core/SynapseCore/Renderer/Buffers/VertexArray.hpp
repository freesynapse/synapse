#pragma once


#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"


namespace Syn {	


	class VertexArray
	{

	public:
		VertexArray();
		VertexArray(const Ref<VertexBuffer>& _vertex_buffer_ref, const Ref<IndexBuffer>& _index_buffer_ref);
		VertexArray(const Ref<VertexBuffer>& _vertex_buffer_ref);
		~VertexArray();

		void setVertexBuffer(const Ref<VertexBuffer>& _vertex_buffer);
		void setIndexBuffer(const Ref<IndexBuffer>& _index_buffer);

		void updateVertexBuffer(void* _vertices, uint32_t _size_in_bytes, uint32_t _offset=0);

		void bind() const;
		void unbind() const;

		__always_inline const GLuint &getArrayID() { return m_arrayID; }
		__always_inline const uint32_t &getIndexCount() { return m_indexBuffer->getIndexCount(); }
		__always_inline const uint32_t getVertexCount() { return m_vertexBuffer->getVertexCount(); }
		__always_inline const Ref<IndexBuffer> &getIndexBuffer() { return m_indexBuffer; }
		__always_inline const Ref<VertexBuffer> &getVertexBuffer() { return m_vertexBuffer; }

	private:
		GLuint m_arrayID = 0;
		Ref<VertexBuffer> m_vertexBuffer = nullptr;
		Ref<IndexBuffer> m_indexBuffer = nullptr;
		
	};


}


