#pragma once


#include "Synapse/Core.hpp"
#include "Synapse/Memory.hpp"


namespace Syn {	

	
	enum class ShaderDataType 
	{ 
		None = 0,
		Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4
	};


	static uint32_t shader_data_type_size(ShaderDataType _type)
	{
		switch (_type)
		{
			case ShaderDataType::None:		return 0;
			case ShaderDataType::Float:		return 4;
			case ShaderDataType::Float2:	return 4 * 2;
			case ShaderDataType::Float3:	return 4 * 3;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Mat3:		return 4 * 3 * 3;
			case ShaderDataType::Mat4:		return 4 * 4 * 4;
			case ShaderDataType::Int:		return 4;
			case ShaderDataType::Int2:		return 4 * 2;
			case ShaderDataType::Int3:		return 4 * 3;
			case ShaderDataType::Int4:		return 4 * 4;
		}
		SYN_CORE_WARNING("unknown ShaderDataType (", (int)_type, ").");
		return 0;
	}


	static uint32_t get_component_count(ShaderDataType _type)
	{
		switch (_type)
		{
			case ShaderDataType::None:		return 0;
			case ShaderDataType::Float:		return 1;
			case ShaderDataType::Float2:	return 2;
			case ShaderDataType::Float3:	return 3;
			case ShaderDataType::Float4:	return 4;
			case ShaderDataType::Mat3:		return 3 * 3;
			case ShaderDataType::Mat4:		return 4 * 4;
			case ShaderDataType::Int:		return 1;
			case ShaderDataType::Int2:		return 2;
			case ShaderDataType::Int3:		return 3;
			case ShaderDataType::Int4:		return 4;
		}

		SYN_CORE_WARNING("unknown ShaderDataType (", (int)_type, ").");
		return 0;
	}



	struct BufferElement
	{
	public:
		std::string name = "";
		uint32_t shaderLocation = 0;
		ShaderDataType type = ShaderDataType::None;
		uint32_t size = 0;
		uint32_t offset = 0;
		bool normalized = false;

		BufferElement() {}
		BufferElement(uint32_t _shader_location, ShaderDataType _type, const std::string& _name, bool _normalized=false) :
			name(_name), shaderLocation(_shader_location), type(_type), size(shader_data_type_size(_type)), normalized(_normalized)
		{}

	};


	class BufferLayout
	{
	public:
		BufferLayout() {}
		BufferLayout(const std::initializer_list<BufferElement>& _elements) :
			m_elements(_elements)
		{
			CalcOffsetStride();
		}
		BufferLayout(const std::vector<BufferElement>& _elements) :
			m_elements(_elements)
		{
			CalcOffsetStride();
		}

		inline uint32_t getStride() const { return m_stride; }
		inline const std::vector<BufferElement>& getElements() { return m_elements; }

		std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_elements.end(); }


	private:
		void CalcOffsetStride()
		{
			uint32_t offset = 0;
			m_stride = 0;
			for (auto& element : m_elements)
			{
				element.offset = offset;
				offset += element.size;
				m_stride += element.size;
			}
		}

	private:
		std::vector<BufferElement> m_elements;
		uint32_t m_stride = 0;

	};


	class VertexBuffer
	{
	public:
		VertexBuffer(GLenum _usage=GL_STATIC_DRAW);
		~VertexBuffer();

		void setData(void* _data, uint32_t _size_in_bytes);
		void updateBufferData(void* _data, uint32_t _size_in_bytes, uint32_t _offset=0);

		/* Bind and allocates buffer; must be called before VertexBuffer::setSubData(). */
		void startDataBlock(uint32_t _total_size_in_bytes);
		/* Does NOT bind the buffer. Call VertexArray::startDataBlock() first. */
		void addSubData(void* _data, uint32_t _size_in_bytes, uint32_t _offset_in_bytes);
		/* Unbind buffer; use after VertexBuffer::setSubData(). */
		void endDataBlock();
		
		const void bind() const;
		const void unbind() const;

		inline const GLuint& getBufferID() { return m_bufferID; }
		inline const BufferLayout& getBufferLayout() { return m_bufferLayout; }
		/*
		Initializer lists are recommended, with BufferElements according to
		{ shaderLocation, type, name, (normalized=false) }, e.g.
		{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float4, "a_position" }
		*/
		inline void setBufferLayout(const BufferLayout& _layout) { m_bufferLayout = _layout; }


	protected:
		BufferLayout m_bufferLayout;
		
		GLuint m_bufferID = 0;
		GLenum m_usage = GL_STATIC_DRAW;

	};


}

