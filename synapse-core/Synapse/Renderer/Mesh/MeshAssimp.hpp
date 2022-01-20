#pragma once


#include "Synapse/Renderer/Mesh/Mesh.hpp"
#include "Synapse/Types.hpp"


namespace Syn {


	class MeshAssimp : public Mesh
	{
	public:
		MeshAssimp(const std::string& _file_path, uint32_t _mesh_load_flags=MESH_TRANSFORM_NONE, const Transform& _mesh_load_transform=Transform());

		void render(const Ref<Shader>& _shader_ptr);
		void printVertices(uint32_t _mesh_attrib_flags);


	private:
		std::unique_ptr<Assimp::Importer> m_importer;
		const aiScene* m_scene;

		uint32_t m_loadFlags = 0;

		std::vector<Submesh> m_submeshes;
		std::vector<VertexBase> m_vertices;
		std::vector<Index> m_indices;


	};


}