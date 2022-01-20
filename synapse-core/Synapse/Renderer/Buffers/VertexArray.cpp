
#include "pch.hpp"

#include "Synapse/Renderer/Buffers/VertexArray.hpp"
#include "Synapse/Renderer/Buffers/VertexBuffer.hpp"

#include "Synapse/Core.hpp"
#include "Synapse/Renderer/Renderer.hpp"


/* Suppression of casting errors from GLuint to void*. */
#ifdef __clang__
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif


namespace Syn {	

	
	static GLenum shader_data_type_to_openGL_enum(ShaderDataType _type)
	{
		switch (_type)
		{
			case ShaderDataType::Float:
			case ShaderDataType::Float2:
			case ShaderDataType::Float3:
			case ShaderDataType::Float4:
			case ShaderDataType::Mat3:		
			case ShaderDataType::Mat4:		return GL_FLOAT;
			case ShaderDataType::Int:
			case ShaderDataType::Int2:
			case ShaderDataType::Int3:
			case ShaderDataType::Int4:		return GL_INT;
			case ShaderDataType::None:		break;
		}
		SYN_CORE_WARNING("Unknown data type: ", (int)_type, ".");
		return GL_NONE;
	}


	//-----------------------------------------------------------------------------------
	VertexArray::VertexArray()
	{
		SYN_RENDER_S0({
			glGenVertexArrays(1, &self->m_arrayID);
		});
	}


	//-----------------------------------------------------------------------------------
	VertexArray::VertexArray(const Ref<VertexBuffer>& _vertex_buffer_ref, const Ref<IndexBuffer>& _index_buffer_ref)
	{
		if (!m_arrayID)
		{
			SYN_RENDER_S0({
				glGenVertexArrays(1, &self->m_arrayID);
			});
		}

		setVertexBuffer(_vertex_buffer_ref);
		setIndexBuffer(_index_buffer_ref);
	}


	//-----------------------------------------------------------------------------------
	VertexArray::VertexArray(const Ref<VertexBuffer>& _vertex_buffer_ref)
	{
		if (!m_arrayID)
		{
			SYN_RENDER_S0({
				glGenVertexArrays(1, &self->m_arrayID);
			});
		}

		setVertexBuffer(_vertex_buffer_ref);
	}


	//-----------------------------------------------------------------------------------
	VertexArray::~VertexArray()
	{
		SYN_RENDER_S0({
			glDeleteVertexArrays(1, &self->m_arrayID);
		});
	}


	//-----------------------------------------------------------------------------------
	void VertexArray::setVertexBuffer(const Ref<VertexBuffer>& _vertex_buffer)
	{
		SYN_RENDER_S1(_vertex_buffer, {
			glBindVertexArray(self->m_arrayID);
			glBindBuffer(GL_ARRAY_BUFFER, _vertex_buffer->getBufferID());

			BufferLayout layout = _vertex_buffer->getBufferLayout();

			if (layout.getElements().size() == 0)
			{
				SYN_CORE_WARNING("VertexBuffer [", _vertex_buffer->getBufferID(), "] have no layout set.");
			}

			for (const auto& element : layout)
			{
				glEnableVertexAttribArray(element.shaderLocation);
				glVertexAttribPointer(
					element.shaderLocation,
					get_component_count(element.type),
					shader_data_type_to_openGL_enum(element.type),
					element.normalized ? GL_TRUE : GL_FALSE,
					layout.getStride(),
					(const void*)element.offset
				);
			}
		
			//_vertex_buffer->unbind();	--> /synapse-core/notes/VertexArray-flow.txt
			glBindVertexArray(0);
		});

		// store pointer
		m_vertexBuffer = _vertex_buffer;
	}


	//-----------------------------------------------------------------------------------
	void VertexArray::setIndexBuffer(const Ref<IndexBuffer>& _index_buffer)
	{
		SYN_RENDER_S1(_index_buffer, {
			glBindVertexArray(self->m_arrayID);
			//_index_buffer->bind();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _index_buffer->getBufferID());

			//_index_buffer->unbind();	--> /synapse-core/notes/VertexArray-flow.txt
			glBindVertexArray(0);
		});

		// store pointer
		m_indexBuffer = _index_buffer;
	}


	//-----------------------------------------------------------------------------------
	void VertexArray::updateVertexBuffer(void* _vertices, uint32_t _size_in_bytes, uint32_t _offset)
	{
		m_vertexBuffer->updateBufferData(_vertices, _size_in_bytes, _offset);
	}


	//-----------------------------------------------------------------------------------
	void VertexArray::bind() const
	{
		SYN_RENDER_S0({
			glBindVertexArray(self->m_arrayID);
		});
	}


	//-----------------------------------------------------------------------------------
	void VertexArray::unbind() const
	{
		SYN_RENDER_S0({
			glBindVertexArray(0);
		});
	}

}
