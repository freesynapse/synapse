
#pragma once

#include <Synapse/Renderer/Mesh/Mesh.hpp>
#include <Synapse/Core.hpp>
#include <SynapseAddons/Voxel/Types.hpp>
#include <SynapseAddons/Voxel/Utility.hpp>
#include <Synapse/Renderer/Shader/Shader.hpp>

#include "VxChunkManager.hpp"


namespace Syn
{
	
	struct VoxelVertex
	{
		glm::vec3 position;
		glm::vec3 normal;
	};

	struct ThreadMeshResult
	{
		int meshingDirection;
		int meshedVertexCount;
	};


	class MeshVxChunk : public Mesh
	{
	public:
		friend class VxChunkManager;

	public:
		/* Default constructor
		 * _parent_ptr		:	Pointer to the parent VxChunkManager class.
		 *						Needed for lookup of voxels outside the limits of this chunk.
		 * _chunk_position	:	Position of this chunk in the world, in 'chunk coordinates'.
		 *						Needed for lookup of voxels outside the limits of this chunk.
		 */
		MeshVxChunk(VxChunkManager* _parent_ptr, 
					const glm::ivec3& _chunk_position);

		/* DEBUG */
		/* Overloaded constructor -- initialize voxel data using a pointer to a texture with stored noise data.
		 * _parent_ptr		:	Pointer to parent VxChunkManager class.
		 * _chunk_position	:	Position of chunk in world space.
		 * _texture_ptr		:	Pointer to texture used for voxel generation.
		 */
		MeshVxChunk(VxChunkManager* _parent_ptr, 
					const glm::ivec3& _chunk_position, 
					const Ref<Texture2DNoise>& _texture_ptr);

		/* Destructor */
		~MeshVxChunk();

		virtual void render(const Ref<Shader>& _shader_ptr) override;


		// accessors
		const uint32_t getVertexCount() const { return m_vertexCount; }
		const bool isRenderable() const { return m_loadingComplete && m_voxelsGenerated && m_meshingComplete; }
		const glm::ivec3& getChunkPosition() const { return m_chunkPosition; }


	private:
		/* Called by constructor. */
		void generateVoxels();
		/* DEBUG (for now) -- called from overloaded constructor */
		void generateVoxelsFromTexture(const Ref<Texture2DNoise>& _texture_ptr);

		/* Called by parent VxChunkManager after load is complete. */
		#ifndef VX_THREADED_MESHING
			void generateMesh();
		#endif

		/* Threaded meshing -- six concurrent threads, one for each meshing direction. */
		#ifdef VX_THREADED_MESHING
			void generateMeshThreaded();
			/* Worker thread, one of six. */
			ThreadMeshResult generateMeshDirection(VoxelVertex* _vertices_ptr, int _direction);
		#endif

		inline void assignFaceNormal(VoxelVertex* _array_ptr, int _offset, const glm::vec3& _normal);
	
		/* Voxel accessors */
		Voxel*** getVoxels() { return m_voxels; }
		int getVoxelType(int _x, int _y, int _z)
		{
			// voxel is in this chunk, not a problem
			if (_x >= 0 && _x < VX_CHUNK_SIZE_XZ &&
				_y >= 0 && _y < VX_CHUNK_SIZE_Y &&
				_z >= 0 && _z < VX_CHUNK_SIZE_XZ)
				return m_voxels[_x][_y][_z].type();

			/* Voxel is not in this chunk -- the switch VX_CHUNK_BORDER_MESHING 
			 * determines whether to end meshing here or go to the neighbouring
			 * chunk to acquire the next voxel. The latter reduces the number of
			 * primitives rendered but requires more frequent meshing to occur
			 * on voxel updates in this chunk.
			 */
			#ifndef VX_CHUNK_STITCHING
				else
					return 0;
			#else
				int x = static_cast<int>(std::floor((float)_x / (VX_CHUNK_SIZE_XZ_F)));
				int y = static_cast<int>(std::floor((float)_y / (VX_CHUNK_SIZE_Y_F)));
				int z = static_cast<int>(std::floor((float)_z / (VX_CHUNK_SIZE_XZ_F)));

				glm::ivec3 neighbourChunk = m_chunkPosition + glm::ivec3(x, y, z);

				int x1 = _x + (-1 * x * VX_CHUNK_SIZE_XZ);
				int y1 = _y + (-1 * y * VX_CHUNK_SIZE_Y);
				int z1 = _z + (-1 * z * VX_CHUNK_SIZE_XZ);

				// return corresponing voxel in the neighbouring chunk
				return m_parentManager->getChunk(neighbourChunk)->getVoxelType(x1, y1, z1);
			#endif
		}
		inline Voxel getVoxel(int _x, int _y, int _z) { return m_voxels[_x][_y][_z]; }
		inline void setVoxel(int _x, int _y, int _z, uint32_t _voxel_data) { m_voxels[_x][_y][_z].data = _voxel_data; }

		// misc. accessors
		/* Is this chunk empty? */
		const bool isEmpty() const { return !(static_cast<bool>(m_vertexCount)); }
		/* m_meshScale is the scaling factor for the chunk mesh. Default = 0.5f. */
		const float getMeshScale() const { return m_meshScale; }
		/* m_meshScale is the scaling factor for the chunk mesh. Default = 0.5f. */
		void setMeshScale(float _f) { m_meshScale = _f; }


	private:
		/* Pointer to parent, needed in chunk edges in greedy meshing. 
		 * Storing as raw pointer; this chunk class is obviously not 
		 * responsible for deallocation. */
		VxChunkManager* m_parentManager = nullptr;
		
		/* Pointer to voxels. */
		Voxel*** m_voxels = nullptr;

		/* Poitner to vertex data. */
		#ifdef VX_THREADED_MESHING
			VoxelVertex** m_verticesT = nullptr;
		#else
			VoxelVertex* m_vertices = nullptr;
		#endif
	
		/* Offset of this chunk in world coordinates. In render, 
		 * the model matrix will be translated by this.
		 */
		glm::vec3 m_chunkOffset = glm::vec3(0.0f);

		/* This chunks position in the world, in 'chunk coordinates'. */
		glm::ivec3 m_chunkPosition = glm::ivec3(0);

		/* number of vertices in mesh */
		uint32_t m_vertexCount = 0;

		/* Scaling factor of mesh from heightmap/noise */
		float m_meshScale = 0.3f;

		/* Flags */
		// TODO : fix (or implement) flags!
		bool m_isEmpty = true;
		bool m_loadingComplete = false;
		bool m_voxelsGenerated = false;
		bool m_meshingComplete = false;
		bool m_needsUpdate = false;

	};


}
