#pragma once


#include <string>
#include <assimp/Importer.hpp>
#include <limits>

#include "Synapse/Core.hpp"
#include "Synapse/Renderer/Renderer.hpp"
#include "Synapse/Renderer/Transform.hpp"
#include "Synapse/Types.hpp"


//struct aiNode;
//struct aiAnimation;
//struct aiNodeAnim;
//struct aiScene;


#define MESH_TRANSFORM_NONE				BIT_SHIFT(0)
#define MESH_TRANSFORM_CENTER_X			BIT_SHIFT(1)
#define MESH_TRANSFORM_CENTER_Y			BIT_SHIFT(2)
#define MESH_TRANSFORM_CENTER_Z			BIT_SHIFT(3)
#define MESH_TRANSFORM_MATRIX			BIT_SHIFT(4)

#define MESH_ATTRIB_NONE				BIT_SHIFT(0)
#define MESH_ATTRIB_POSITION			BIT_SHIFT(1)
#define MESH_ATTRIB_NORMAL				BIT_SHIFT(2)
#define MESH_ATTRIB_TANGENT				BIT_SHIFT(3)
#define MESH_ATTRIB_BITANGENT			BIT_SHIFT(4)
#define MESH_ATTRIB_UV					BIT_SHIFT(5)
#define MESH_ATTRIB_COLOR				BIT_SHIFT(6)

#define MESH_NORMALS_APPROX_FAST		BIT_SHIFT(0)
#define MESH_NORMALS_APPROX				BIT_SHIFT(1)
#define MESH_NORMALS_ANALYTIC			BIT_SHIFT(2)

namespace Syn {


	struct Submesh
	{
		uint32_t baseVertex;
		uint32_t baseIndex;
		uint32_t materialIndex;
		uint32_t indexCount;
	};


	// pure abstract
	class Mesh
	{
	public:
		virtual ~Mesh() = default;

		// render function; bind vertex array, uploads model matrix and drawIndexed
		virtual void render(const Ref<Shader>& _shader_ptr) = 0;

		// accessors
		virtual void setTransform(Transform _t) { m_transform = _t; }
		inline const Transform& getTransform() const { return m_transform; }
		inline Transform* getTransformPtr() { return (Transform*)& m_transform; }

		inline const glm::mat4& getModelMatrix() const { return m_transform.getModelMatrix(); }

		inline const AABB& getAABB() const { return m_aabb; }
		inline AABB& getAABB() { return m_aabb; }
		inline void setAABB(const AABB& _aabb) { m_aabb = _aabb; }

		inline const Ref<VertexArray>& getVertexArray() { return m_vertexArray; }
		inline void setVertexArray(Ref<VertexArray> _vertex_array) { m_vertexArray = _vertex_array; }
		inline uint32_t getIndexCount() { return m_vertexArray->getIndexCount(); }
		inline uint32_t getTriangleCount() { return m_vertexArray->getIndexCount() / 3; }


	protected:
		std::string m_assetPath = "";
		AABB m_aabb;
		Ref<VertexArray> m_vertexArray = nullptr;
		Transform m_transform = Transform();

	};


}