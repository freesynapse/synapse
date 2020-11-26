#pragma once


#include "VertexBuffer.h"
#include "IndexBuffer.h"


namespace Syn {	


	class VertexArray
	{

	public:
		VertexArray();
		VertexArray(Ref<VertexBuffer> _vertex_buffer_ref, Ref<IndexBuffer> _index_buffer_ref);
		VertexArray(Ref<VertexBuffer> _vertex_buffer_ref);
		~VertexArray();

		void setVertexBuffer(Ref<VertexBuffer> _vertex_buffer);
		void setIndexBuffer(Ref<IndexBuffer> _index_buffer);

		void updateVertexBuffer(void* _vertices, uint32_t _size, uint32_t _offset=0);

		void bind() const;
		void unbind() const;

		inline const GLuint& getArrayID() { return m_arrayID; }
		inline const uint32_t& getIndexCount() { return m_indexBuffer->getIndexCount(); }
		inline const Ref<IndexBuffer>& getIndexBuffer() { return m_indexBuffer; }
		inline const Ref<VertexBuffer>& getVertexBuffer() { return m_vertexBuffer; }

	private:
		GLuint m_arrayID = 0;
		Ref<VertexBuffer> m_vertexBuffer = nullptr;
		Ref<IndexBuffer> m_indexBuffer = nullptr;

	};


}


