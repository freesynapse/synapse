
#pragma once

#include <set>

#include <Synapse/Core.hpp>
#include <Synapse/Renderer/Shader/Shader.hpp>
#include <Synapse/Renderer/Material/Texture2DNoise.hpp>
#include <Synapse/Renderer/Chunk/ChunkManager.hpp>
#include <SynapseAddons/Voxel/Types.hpp>


namespace Syn 
{

	class MeshVxChunk;
	class VxChunkManager : public ChunkManager
	{
	public:
		friend class MeshVxChunk;
	
	public:
		AABB debugGetAABB();
		float debugDestroyVoxels(const glm::ivec3& _min, const glm::ivec3& _max);
		float debugRemeshChunks();
		void debugSetupChunkBoundaries();
		float getDebugMeshingTime() { return debugMeshingTime; }

		void debugUpdateChunksFromTexture(const Ref<Texture2DNoise>& _noise_texture_ptr);
		void debugUpdateVoxelDataFromTexture(const Ref<Texture2DNoise>& _noise_texture_ptr, float _mesh_scale);


	private:
		float debugMeshingTime = 0.0f;
		bool debugChunkBoundariesSet = false;


	public:
		VxChunkManager();
		~VxChunkManager();

		/* Update chunks around the camera.
		 * Chunks outside of viewing distance gets flagged for deletion, and are deleted
		 * at the end of this method.
		 * New chunks needed to be loaded are identified and added to m_pendingChunks and
		 * one chunk is loaded per call to this->loadPendingChunks();
		 */
		virtual void updateChunks(const glm::vec3& _camera_position) override;

		/* Calls render() method of each chunk. */
		void render(const Ref<Shader>& _shader_ptr);


		// accessors
		const uint32_t getVertexCount() const { return m_totalVertexCount; }
		const uint32_t getTriangleCount() const { return m_totalVertexCount / 3; }
		const uint32_t getFaceCount() const { return m_totalVertexCount / 6; }
		const uint32_t getChunkCount() const { return m_chunkMap.size(); }
		glm::ivec3 getChunkPosition(const glm::vec3& _world_pos) 
		{ 
			return getChunkPosition(static_cast<int>(_world_pos.x), 
									static_cast<int>(_world_pos.y), 
									static_cast<int>(_world_pos.z)); 
		}


	private:
		
		/* Loads one (1) chunk per function call. Loads the first chunk in 
		 * m_pendingChunksLoad. Called by updateChunks().
		 */
		void loadPendingChunks();

		/* Deletes one (1) chunk per function call. Loads the first chunk in
		 * m_pendingChunksDelete. Called by updateChunks().
		 */
		void deletePendingChunks();

		/* Meshes one (1) chunk per function call. Loads the first chunk in
		 * m_pendingChunksMesh. Called by updateChunks().
		 */
		void meshPendingChunks();

		
		/* ACCESSORS */

		/* Retrieves a specific chunk; if not present, the null chunk is returned. */
		inline MeshVxChunk* getChunk(const glm::ivec3& _chunk_pos)
		{
			// return null chunk if not present
			if (m_chunkMap.find(_chunk_pos) == m_chunkMap.end())
				return s_nullChunk;
			// else, return indexed chunk
			return m_chunkMap[_chunk_pos];
		}

		/* World to chunk coordinates (within voxel coordinates not accounted for). */
		inline glm::ivec3 getChunkPosition(int _x, int _y, int _z)
		{
			return glm::ivec3(
				static_cast<int>(std::floor(static_cast<float>(_x) * VX_CHUNK_SIZE_XZ_DIV)),
				static_cast<int>(std::floor(static_cast<float>(_y) * VX_CHUNK_SIZE_Y_DIV)),
				static_cast<int>(std::floor(static_cast<float>(_z) * VX_CHUNK_SIZE_XZ_DIV))
			);
		}

	public:
		/* Sets a voxel within a chunk based on world coordinates.
		 * _x, _y, _z	: world coordinates. Mapped to chunk and voxel within chunk.
		 * _vx_data		: value of voxel.
		 */
		void setVoxelData(int _x, int _y, int _z, uint32_t _vx_data);
		/* Sets a voxel in a specified chunk
		 * _chunk_pos	: coordinates of chunk in 'chunk space'.
		 * _x, _y, _z	: coordinates within chunk, in 'voxel space' (i.e. model space).
		 * _vx_data		: value of voxel.
		 */
		void setChunkVoxelData(const glm::ivec3& _chunk_pos, int _x, int _y, int _z, uint32_t _vx_data);


	private:
		glm::ivec3 m_vxChunkSize = glm::ivec3(0);

		/* Static chunk containing only type 0 voxels. For reference at chunk borders during 
		 * meshing. I.e. if a chunk requests a voxel at [-1, 0, 0], a lookup through 
		 * the MeshTerrainChunk::m_parentManager (=this VxChunkManager) will return the
		 * neighbouring chunk of that chunk, and the corresponding voxel (which would be the 
		 * voxel [m_chunkSize-1, 0, 0] in the neighbouring chunk. However, if that chunk is at 
		 * the edge of the loaded chunks, the null chunk will be referenced, hence always 
		 * returning a zero @ [m_chunkSize-1, 0, 0].
		 *
		 * This hack prevents nullptr exceptions on chunk lookup (VxChunkManager::getChunk).
		 */
		static MeshVxChunk* s_nullChunk;

		/* Map of all chunks, sorted by 3d integer vector.
		 * The use of std::unordered_map, which is sorted by a hash of the key, ensures
		 * O(1) look-up time.
		 */
		std::unordered_map<glm::ivec3, MeshVxChunk*, ivec3Hash, ivec3Hash> m_chunkMap;

		/* updateChunks() parameters :
		 * how many chunks are loaded / meshed per frame (i.e. every call to updateChunks()).
		 */
		uint32_t m_chunkLoadCount = 2;
		uint32_t m_chunkMeshCount = 2;

		/* number of vertices in all chunks */
		uint32_t m_totalVertexCount = 0;

		/* queue of chunk positions flagged for loading and deletion. */
		std::deque<glm::ivec3> m_pendingChunksLoad;
		std::deque<glm::ivec3> m_pendingChunksDelete;
		/* queue of chunk positions flagged for meshing. */
		std::deque<glm::ivec3> m_pendingChunksMesh;

		/* Set for remesh of neighbouring chunks on changing of voxel data */
		std::unordered_set<glm::ivec3, ivec3Hash> m_remeshSet;


	};


}