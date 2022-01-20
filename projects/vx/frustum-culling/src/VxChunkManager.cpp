
#include "pch.hpp"

#include <Synapse/Utils/Timer/TimeStep.hpp>
#include <Synapse/Utils/Timer/Timer.hpp>
#include <Synapse/Debug/Log.hpp>
// for debug functions
#include <Synapse/Renderer/MeshCreator.hpp>

#include "VxChunkManager.hpp"
#include "MeshVxChunk.hpp"

#include <ctime>
#include <time.h>


namespace Syn 
{

	// static declaration
	MeshVxChunk* VxChunkManager::s_nullChunk = nullptr;

	//-----------------------------------------------------------------------------------
	VxChunkManager::VxChunkManager()
	{
		// radius of rendering in chunks (x and z) and
		// how many chunks to load and mesh each frame, respectively
		#ifdef DEBUG_VX_CHUNK_MANAGER
			m_chunkRenderDistance = 4;
			m_chunkLoadCount = 2;
			m_chunkMeshCount = 2;
		#else
			m_chunkRenderDistance = 8;
			m_chunkLoadCount = 16;
			m_chunkMeshCount = 16;
		#endif

		m_currentCameraChunk = glm::ivec3(0);

		// initialize the null chunk 
		s_nullChunk = new MeshVxChunk(this, glm::ivec3(0));

		// set all null chunk voxels to 0
		auto V = s_nullChunk->getVoxels();
		VX_FOR_BEGIN_DEF
			V[x][y][z].data = 0;
		VX_FOR_END
	}

	//-----------------------------------------------------------------------------------
	VxChunkManager::~VxChunkManager()
	{
		if (m_chunkMap.size())
		{
			for (auto item : m_chunkMap)
				delete item.second;
		}

		if (s_nullChunk != nullptr)
			delete s_nullChunk;
	}

	//-----------------------------------------------------------------------------------
	void VxChunkManager::debugCreateChunks(int _diameter)
	{
		Timer t0(__func__, true);
		// create a block of chunks
		for (int x = 0; x < _diameter; x++)
			for (int y = 0; y < _diameter; y++)
				for (int z = 0; z < _diameter; z++)
					m_pendingChunksLoad.push_back(glm::ivec3(x, y, z));

		printf("size load = %zu\n", m_pendingChunksLoad.size());
		glm::ivec3 chunkPos;

		// Load chunks
		if (!m_pendingChunksLoad.size())
			return;

		for (size_t i = 0; i < m_pendingChunksLoad.size(); i++)
		{
			chunkPos = m_pendingChunksLoad[i];

			m_chunkMap[chunkPos] = new MeshVxChunk(this, chunkPos);
			m_pendingChunksMesh.push_back(chunkPos);
		}

		// Mesh chunks
		printf("size mesh = %zu\n", m_pendingChunksMesh.size());
		if ((m_pendingChunksMesh.size() == 0))
			return;

		Timer t("", false);
		for (size_t i = 0; i < m_pendingChunksMesh.size(); i++)
		{
			#ifdef VX_THREADED_MESHING
				m_chunkMap[chunkPos]->generateMeshThreaded();
			#else
				m_chunkMap[chunkPos]->generateMesh();
			#endif
			m_totalVertexCount += m_chunkMap[chunkPos]->getVertexCount();
		}
		debugMeshingTime = t.getDeltaTimeMs();

		debugSetupChunkBoundaries();

		m_pendingChunksLoad.clear();
		m_pendingChunksMesh.clear();

	}
	//-----------------------------------------------------------------------------------
	void VxChunkManager::updateChunks(const glm::vec3& _camera_position)
	{
		// 'chunk' position of camera
		m_currentCameraChunk = glm::ivec3(
			static_cast<int>(floor(_camera_position.x / VX_CHUNK_SIZE_XZ_F)),
			static_cast<int>(floor(_camera_position.y / VX_CHUNK_SIZE_Y_F)),
			static_cast<int>(floor(_camera_position.z / VX_CHUNK_SIZE_XZ_F))
		);

		#ifndef VX_AUTO_LOAD_CHUNKS
			if (TimeStep::getFrameCount() == 0)
			{
				int radius = m_chunkRenderDistance;
				radius = 5;
				//for (int y = 0; y <= 256 / VX_CHUNK_SIZE_Y; y++)
				for (int y = -3; y <= 3; y++)
					for (int x = -radius; x <= radius; x++)
						for (int z = -radius; z <= radius; z++)
							//if (x * x + z * z < m_chunkRenderDistance * m_chunkRenderDistance)
							//	m_pendingChunksLoad.push_back(glm::ivec3(x, y, z));
							m_pendingChunksLoad.push_back(glm::ivec3(x, y, z));
			}
		#endif

		#ifdef VX_AUTO_LOAD_CHUNKS
			// delete chunks outside rendering distance
			// step through all loaded chunks
			for (auto it = m_chunkMap.begin(); it != m_chunkMap.end(); it++)
			{
				// get position of chunk
				MeshVxChunk* chunk = it->second;
				glm::ivec3 pos = chunk->getChunkPosition();

				// find distance in 'chunks' to camera
				glm::ivec3 distance = glm::abs(pos - m_currentCameraChunk);

				// is this chunk already pending deletion?
				bool alreadyFlagged = std::find(m_pendingChunksDelete.begin(), m_pendingChunksDelete.end(), pos) != m_pendingChunksDelete.end();

				// this chunk is outside render distance and not flagged for deletion
				if ((distance.x > m_chunkRenderDistance || distance.z > m_chunkRenderDistance) && !alreadyFlagged)
					m_pendingChunksDelete.push_back(pos);
			}


			// load chunks within viewing distance from the camera
			for (int x = m_currentCameraChunk.x - m_chunkRenderDistance; x < m_currentCameraChunk.x + m_chunkRenderDistance + 1; x++)
			{
				for (int z = m_currentCameraChunk.z - m_chunkRenderDistance; z < m_currentCameraChunk.z + m_chunkRenderDistance + 1; z++)
				{
					for (uint32_t y = 0; y <= 256 / VX_CHUNK_SIZE_Y; y++)
					{
						glm::ivec3 pos = glm::ivec3(x, y, z);

						auto it = std::find(m_pendingChunksLoad.begin(), m_pendingChunksLoad.end(), pos);
						if (m_chunkMap.find(pos) == m_chunkMap.end() &&
							it == m_pendingChunksLoad.end())
						{
							m_pendingChunksLoad.push_back(pos);
						}
					}
				}
			}
		#endif

		// load the first chunk in the load list (base class method)
		loadPendingChunks();

		// mesh chunks if available
		meshPendingChunks();

		// delete the first chunk in the deletion list (base class method)
		deletePendingChunks();
	}
	
	//-----------------------------------------------------------------------------------
	void VxChunkManager::loadPendingChunks()
	{
		if (m_pendingChunksLoad.size() == 0)
			return;

		// how many chunks are processed each frame?
		for (uint32_t i = 0; i < m_chunkLoadCount; i++)
		{
			// get out if finished
			if (!m_pendingChunksLoad.size())
				break;

			// get first chunk in list, then remove it
			glm::ivec3 chunkPos = m_pendingChunksLoad.front();
			m_pendingChunksLoad.pop_front();

			// create new chunk with correct offset
			m_chunkMap[chunkPos] = new MeshVxChunk(this, chunkPos);

			// add chunk to meshing list
			m_pendingChunksMesh.push_back(chunkPos);

		}
	}

	//-----------------------------------------------------------------------------------
	void VxChunkManager::deletePendingChunks()
	{
		if (m_pendingChunksDelete.size() == 0)
			return;

		// get chunk to delete and remove from queue
		glm::ivec3 chunkPos = m_pendingChunksDelete.front();
		m_pendingChunksDelete.pop_front();

		m_totalVertexCount -= m_chunkMap[chunkPos]->getVertexCount();

		// delete from map and free memory
		delete m_chunkMap[chunkPos];
		m_chunkMap.erase(chunkPos);
	}

	//-----------------------------------------------------------------------------------
	void VxChunkManager::meshPendingChunks()
	{
		// don't mesh an empty list and don't mesh until
		// chunk loading is complete.
		if ((m_pendingChunksMesh.size() == 0) ||
			(m_pendingChunksLoad.size() > 0))
			return;

		Timer t("", false);
		// how many chunks are processed each frame (m_chunkMeshCount)?
		for (uint32_t i = 0; i < m_chunkMeshCount; i++)
		{
			// break if no more chunks to mesh
			if (!m_pendingChunksMesh.size())
				break;

			glm::ivec3 chunkPos = m_pendingChunksMesh.front();
			m_pendingChunksMesh.pop_front();

			// check that chunk isn't deleted
			if (m_chunkMap.find(chunkPos) != m_chunkMap.end())
			{
				m_totalVertexCount -= m_chunkMap[chunkPos]->getVertexCount();
				#ifdef VX_THREADED_MESHING
					m_chunkMap[chunkPos]->generateMeshThreaded();
				#else
					m_chunkMap[chunkPos]->generateMesh();
				#endif
				m_totalVertexCount += m_chunkMap[chunkPos]->getVertexCount();
			}
		}
		debugMeshingTime = t.getDeltaTimeMs();
	}

	//-----------------------------------------------------------------------------------
	void VxChunkManager::render(const Ref<Shader>& _shader_ptr)
	{
		// uniforms set from outside for now

		for (auto& item : m_chunkMap)
		{
			if (!item.second->isEmpty())
				item.second->render(_shader_ptr);
		}
	}

	//-----------------------------------------------------------------------------------
	void VxChunkManager::setVoxelData(int _x, int _y, int _z, uint32_t _vx_data)
	{
		// find chunk coordinates
		glm::ivec3 chunk = glm::ivec3(
			static_cast<int>(std::floor(static_cast<float>(_x) * VX_CHUNK_SIZE_XZ_DIV)),
			static_cast<int>(std::floor(static_cast<float>(_y) * VX_CHUNK_SIZE_Y_DIV)),
			static_cast<int>(std::floor(static_cast<float>(_z) * VX_CHUNK_SIZE_XZ_DIV))
		);
		
		// find voxel coordinates in chunk
		int vx_x = _x < 0 ? (VX_CHUNK_SIZE_XZ + (_x % VX_CHUNK_SIZE_XZ)) % VX_CHUNK_SIZE_XZ : _x % VX_CHUNK_SIZE_XZ;
		int vx_y = _y < 0 ? (VX_CHUNK_SIZE_Y  + (_y % VX_CHUNK_SIZE_Y )) % VX_CHUNK_SIZE_Y  : _y % VX_CHUNK_SIZE_Y;
		int vx_z = _z < 0 ? (VX_CHUNK_SIZE_XZ + (_z % VX_CHUNK_SIZE_XZ)) % VX_CHUNK_SIZE_XZ : _z % VX_CHUNK_SIZE_XZ;

		if (m_chunkMap.find(chunk) != m_chunkMap.end())
		{
			m_chunkMap[chunk]->setVoxel(vx_x, vx_y, vx_z, _vx_data);
			/*
			if (vx_x == 0)
				m_remeshSet.insert(chunk + glm::ivec3(-1, 0, 0));
			else if (vx_x == VX_CHUNK_SIZE_XZ_LIM)
				m_remeshSet.insert(chunk + glm::ivec3(1, 0, 0));

			if (vx_y == 0)
				m_remeshSet.insert(chunk + glm::ivec3(0, -1, 0));
			else if (vx_y == VX_CHUNK_SIZE_Y_LIM)
				m_remeshSet.insert(chunk + glm::ivec3(0, 1, 0));

			if (vx_z == 0)
				m_remeshSet.insert(chunk + glm::ivec3(0, 0, -1));
			else if (vx_z == VX_CHUNK_SIZE_XZ_LIM)
				m_remeshSet.insert(chunk + glm::ivec3(0, 0, 1));
			*/
		}
	}

	//-----------------------------------------------------------------------------------
	void VxChunkManager::setChunkVoxelData(const glm::ivec3& _chunk_pos, int _x, int _y, int _z, uint32_t _vx_data)
	{
		// find voxel coordinates in chunk
		int vx_x = _x < 0 ? (VX_CHUNK_SIZE_XZ + (_x % VX_CHUNK_SIZE_XZ)) % VX_CHUNK_SIZE_XZ : _x % VX_CHUNK_SIZE_XZ;
		int vx_y = _y < 0 ? (VX_CHUNK_SIZE_Y  + (_y % VX_CHUNK_SIZE_Y )) % VX_CHUNK_SIZE_Y  : _y % VX_CHUNK_SIZE_Y;
		int vx_z = _z < 0 ? (VX_CHUNK_SIZE_XZ + (_z % VX_CHUNK_SIZE_XZ)) % VX_CHUNK_SIZE_XZ : _z % VX_CHUNK_SIZE_XZ;

		if (m_chunkMap.find(_chunk_pos) != m_chunkMap.end())
		{
			m_chunkMap[_chunk_pos]->setVoxel(vx_x, vx_y, vx_z, _vx_data);
			m_remeshSet.insert(_chunk_pos);
			glm::ivec3 neighbour;
			if (vx_x == 0)
				m_remeshSet.insert(_chunk_pos + glm::ivec3(-1, 0, 0));
			else if (vx_x == VX_CHUNK_SIZE_XZ_LIM)
				m_remeshSet.insert(_chunk_pos + glm::ivec3(1, 0, 0));

			if (vx_y == 0)
				m_remeshSet.insert(_chunk_pos + glm::ivec3(0, -1, 0));
			else if (vx_y == VX_CHUNK_SIZE_Y_LIM)
				m_remeshSet.insert(_chunk_pos + glm::ivec3(0, 1, 0));

			if (vx_z == 0)
				m_remeshSet.insert(_chunk_pos + glm::ivec3(0, 0, -1));
			else if (vx_z == VX_CHUNK_SIZE_XZ_LIM)
				m_remeshSet.insert(_chunk_pos + glm::ivec3(0, 0, 1));
		}
	}

	//-----------------------------------------------------------------------------------
	float VxChunkManager::debugDestroyVoxels(const glm::ivec3& _min, const glm::ivec3& _max)
	{
		Timer t("", false);
		//std::unordered_map<glm::ivec3, glm::ivec3, ivec3Hash, ivec3Hash> chunksToRemesh;
		//std::unordered_map<uint32_t, bool> neighbourMap;

		// clear current remesh map
		m_remeshSet.clear();

		for (int x = _min.x; x < _max.x; x++)
		{
			for (int y = _min.y; y < _max.y; y++)
			{
				for (int z = _min.z; z < _max.z; z++)
				{
					glm::ivec3 chunk = getChunkPosition(x, y, z);
					setChunkVoxelData(chunk, x, y, z, 0);
				}
			}
		}

		// no need to check for validity of chunks, this is done in meshPendingChunks()
		for (auto& chunk : m_remeshSet)
			m_pendingChunksMesh.push_back(chunk);

		return t.getDeltaTimeMs();
	}

	//-----------------------------------------------------------------------------------
	AABB VxChunkManager::debugGetAABB()
	{

		AABB aabb;
		for (auto& it : m_chunkMap)
		{
			MeshVxChunk* chunk = it.second;
			AABB c_aabb = chunk->getAABB();
			aabb.min.x = MIN(c_aabb.min.x, aabb.min.x);
			aabb.min.y = MIN(c_aabb.min.y, aabb.min.y);
			aabb.min.z = MIN(c_aabb.min.z, aabb.min.z);

			aabb.max.x = MAX(c_aabb.max.x, aabb.max.x);
			aabb.max.y = MAX(c_aabb.max.y, aabb.max.y);
			aabb.max.z = MAX(c_aabb.max.z, aabb.max.z);
		}

		return aabb;
	}

	//-----------------------------------------------------------------------------------
	float VxChunkManager::debugRemeshChunks()
	{
		Timer t("", true);
		int i = 0;
		for (auto& it : m_chunkMap)
		{
			glm::ivec3 chunkPos = it.first;
			#ifdef VX_THREADED_MESHING
				m_chunkMap[chunkPos]->generateMeshThreaded();
			#else
				m_chunkMap[chunkPos]->generateMesh();
			#endif
			i++;
		}
		float tt = t.getDeltaTimeMs();
		SYN_CORE_TRACE(i, " chunks remeshed in ", tt, " ms.");
		return t.getDeltaTimeMs();
	}

	//-----------------------------------------------------------------------------------
	void VxChunkManager::debugSetupChunkBoundaries()
	{
		if (debugChunkBoundariesSet)
			return;

		std::vector<glm::vec3> lines;

		for (auto& it : m_chunkMap)
		{
			MeshVxChunk* chunk = it.second;
			AABB aabb = chunk->getAABB();
			
			glm::vec3 v0 = aabb.min;
			glm::vec3 x, y, z;
			x = y = z = aabb.min;
			x.x = aabb.max.x;
			y.y = aabb.max.y;
			z.z = aabb.max.z;

			lines.push_back(v0);
			lines.push_back(x);
			lines.push_back(v0);
			lines.push_back(y);
			lines.push_back(v0);
			lines.push_back(z);
		}

		// Warning	C26444	Avoid unnamed objects with custom construction and destruction.
		#pragma warning(disable : 26444)
		MeshCreator::createDebugLines(lines, "chunk_boundaries");

		debugChunkBoundariesSet = true;
	}

}