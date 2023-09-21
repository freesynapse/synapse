
#include "../../../pch.hpp"

#include "MeshDebug.hpp"


namespace Syn {


	std::string MeshDebugTypeToString(MeshDebugType _type)
	{
		switch (_type)
		{
		case MeshDebugType::CUBE:			return "MeshDebugType::CUBE";
		case MeshDebugType::QUAD:			return "MeshDebugType::QUAD";
		case MeshDebugType::SPHERE:			return "MeshDebugType::SPHERE";
		case MeshDebugType::POINTS:			return "MeshDebugType::POINTS";
		case MeshDebugType::LINES:			return "MeshDebugType::LINES";
		case MeshDebugType::LINE_STRIP:		return "MeshDebugType::LINE_STRIP";
		case MeshDebugType::AABB:			return "MeshDebugType::AABB";
		case MeshDebugType::AXES:			return "MeshDebugType::AXES";
		case MeshDebugType::GRID_PLANE:		return "MeshDebugType::GRID_PLANE";
		case MeshDebugType::GRID_PLANE_FS:	return "MeshDebugType::GRID_PLANE_FS";
		default:							return "";
		}
	}


	//-----------------------------------------------------------------------------------
	GLenum MeshDebugTypeToPrimitiveType(MeshDebugType _type)
	{
		switch (_type)
		{
		case MeshDebugType::POINTS:		return GL_POINTS;
		case MeshDebugType::LINES:
		case MeshDebugType::AABB:
		case MeshDebugType::GRID_PLANE:
		case MeshDebugType::AXES:		return GL_LINES;
		case MeshDebugType::LINE_STRIP:	return GL_LINE_STRIP;
		case MeshDebugType::CUBE:
		case MeshDebugType::QUAD:
		case MeshDebugType::GRID_PLANE_FS:	
		case MeshDebugType::SPHERE:		return GL_TRIANGLES;
		default:						return GL_TRIANGLES;
		}

	}


	//-----------------------------------------------------------------------------------
	MeshDebug::MeshDebug(MeshDebugType _type)
	{
		m_type = _type;
		m_primitiveType = MeshDebugTypeToPrimitiveType(m_type);
	}


	//-----------------------------------------------------------------------------------
	MeshDebug::~MeshDebug()
	{
	}


	//-----------------------------------------------------------------------------------
	void MeshDebug::updateVertexBuffer(void* _data, uint32_t _size_in_bytes, uint32_t _offset)
	{
		m_vertexArray->updateVertexBuffer(_data, _size_in_bytes, _offset);
	}


	//-----------------------------------------------------------------------------------
	void MeshDebug::render(const Ref<Shader>& _shader_ptr)
	{
		// shader already enabled in MeshCreator

		// translate by position
		_shader_ptr->setMatrix4fv("u_model_matrix", glm::translate(m_transform.getModelMatrix(), m_position));
		m_vertexArray->bind();

		Renderer::drawIndexed(m_vertexArray->getIndexCount(), true, m_primitiveType);
	}


}