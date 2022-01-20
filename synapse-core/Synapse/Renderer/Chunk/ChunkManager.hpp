
#pragma once

#include <deque>
#include "Synapse/Renderer/Mesh/Mesh.hpp"


namespace Syn
{

	class ChunkManager
	{
	public:
		virtual ~ChunkManager() = default;

		/* Update chunks around the camera.
		 * Chunks outside of viewing distance gets flagged for deletion, and are deleted
		 * at the end of this method.
		 * New chunks needed to be loaded are identified and added to m_pendingChunks and
		 * one chunk is loaded per call to this->loadPendingChunks();
		 */
		virtual void updateChunks(const glm::vec3& _camera_position) = 0;


	protected:
		// size of each chunk, in units. x = z.
		uint32_t m_chunkSize = 16;

		// resolution of chunk grid
		uint32_t m_resolution = 0;

		// storage of last camera position in chunk coordinates
		glm::ivec3 m_currentCameraChunk = glm::ivec3(0);

		// radius of chunks to be loaded around the current camera position
		int m_chunkRenderDistance = 3;

	};


}