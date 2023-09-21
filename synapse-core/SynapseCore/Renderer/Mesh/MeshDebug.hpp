#pragma once


#include "Mesh.hpp"
#include "../../Core.hpp"
#include "../Shader/Shader.hpp"
#include "../Buffers/VertexArray.hpp"


namespace Syn {


	enum class MeshDebugType
	{
		NONE = 1,
		CUBE,
		QUAD,
		SPHERE,
		LINES,
		LINE_STRIP,
		AABB,
		AXES,
		POINTS,
		GRID_PLANE,
		GRID_PLANE_FS,
	};

	
	std::string MeshDebugTypeToString(MeshDebugType _type);
	GLenum MeshDebugTypeToPrimitiveType(MeshDebugType _type);


	//-----------------------------------------------------------------------------------
	class MeshDebug : public Mesh
	{
	public:
		friend class MeshCreator;

	public:
		MeshDebug(MeshDebugType _type=MeshDebugType::NONE);
		~MeshDebug();

		void updateVertexBuffer(void* _data, uint32_t _size_in_bytes, uint32_t _offset=0);
		void render(const Ref<Shader>& _shader_ptr) override;

		// accessors
		void setModelMatrix(const glm::mat4& _m) { m_transform.setModelMatrix(_m); }
		void setVertexArray(Ref<VertexArray> _vertex_array) { m_vertexArray = _vertex_array; }
		const std::string& getShaderName() const { return m_shaderName; }
		MeshDebugType getType() const { return m_type; }
		inline float getPointSize() const { return m_pointSize; }
		inline void setPointSize(float _sz) { m_pointSize = _sz; }
		inline float getGridSize() const { return m_gridSize; }
		inline void setGridSize(float _sz) { m_gridSize = _sz; }
		inline float getLineWidth() const { return m_lineWidth; }
		inline void setLineWidth(float _sz) { m_lineWidth = _sz; }
		inline const glm::vec3& getPosition() const { return m_position; }
		inline void setPosition(const glm::vec3& _position) { m_position = _position; }

	private:
		std::string m_shaderName;	// reference to which shader is used with this debug mesh
		MeshDebugType m_type = MeshDebugType::NONE;
		GLenum m_primitiveType = GL_TRIANGLES;
		float m_pointSize = 0.0f;
		float m_gridSize = 21.0f;
		float m_lineWidth = 1.0f;
		glm::vec3 m_position = glm::vec3(0.0f);
	};

}
