
#pragma once

#include <Synapse/Renderer/MeshCreator.hpp>
#include "noise_2.hpp"


#define DEBUG_MESH_TERRAIN

namespace Syn
{
	typedef struct mesh_vertex_t
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
	} mesh_vertex_t;

	class HeightMap : public Mesh
	{
	public:
		HeightMap(uint32_t _width=128, uint32_t _resolution=128);
		~HeightMap();

		/*
		 * Setup vertices (positions, normals, tangents, bitangents) and indices for the terrain chunk.
		 * _octaves specifies the number of octaves used for fBm and _energy is a noise amplitude multiplier.
		 * Multi-threaded (where applicable), uses std::async and MeshTerrain::computeMeshSliceVertices() / 
		 * computeMeshSliceIndices() for performance increase.
		 */
		void setupVertexData();
		
		// render method for when a single MeshTerrain instance is created.
		void render(const Ref<Shader>& _shader_ptr) override;

		// accessors
		const bool isRenderable() { return m_constructed; }


	private:
		/* Threaded computation of mesh vertices -- worker thread
		 * _data		: ptr to the allocated mesh vertex data 
		 * _z_begin		: where the slice begins
		 * _z_end		: end of slice, note that the last thread will be responsible for any odd vertices, eg [192..257].
		 * Returns a glm::vec2 containing the min and max y values of this slice.
		 */
		glm::vec2 computeMeshSliceVertices(mesh_vertex_t* _data, uint32_t _z_begin, uint32_t _z_end);
		/* Threaded computation of mesh indices -- worker thread
		 * _data		: ptr to the allocated mesh index data
		 * _z_begin		: where the slice begins
		 * _z_end		: end of slice, note that the last thread will be responsible for any odd vertices, eg [192..257].
		 */
		void computeMeshSliceIndices(uint32_t* _data, uint32_t _z_begin, uint32_t _z_end);
		

	private:
		// width (and height) of the grid in world units
		uint32_t m_width = 4;
		// resolution of the grid, i.e. how many subdivisions in total there is
		uint32_t m_resolution = 128;
		// step size in world units, i.e. spacing between vertices
		float m_stepSize = 0.0f;
		// number of vertices and indices in mesh
		uint32_t m_vertexCount = 0;
		uint32_t m_indexCount = 0;
		
		// vertex and index data
		mesh_vertex_t* m_vertices = nullptr;
		uint32_t* m_indices = nullptr;

        cnoise* m_generator = nullptr;

		// flags
		bool m_constructed = false;

	};


}
