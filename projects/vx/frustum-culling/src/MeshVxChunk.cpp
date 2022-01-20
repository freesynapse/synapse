
#include "pch.hpp"

#include <SynapseAddons/Voxel/Types.hpp>
#include <SynapseAddons/Voxel/Utility.hpp>
#include <SynapseAddons/Voxel/VxNoise.hpp>
#include <Synapse/Renderer/MeshCreator.hpp>
#include <Synapse/Renderer/Renderer.hpp>
#include <Synapse/Utils/Timer/Timer.hpp>
#include <Synapse/Memory.hpp>
#include <Synapse/Utils/Noise/NoiseGenerator.hpp>
//#include <External/FastNoise/FastNoise.h>

#include "MeshVxChunk.hpp"


namespace Syn 
{

	//-----------------------------------------------------------------------------------
	MeshVxChunk::MeshVxChunk(VxChunkManager* _parent_ptr, 
							 const glm::ivec3& _chunk_position) :
		m_parentManager(_parent_ptr), m_chunkPosition(_chunk_position)
	{
		// calculate offset in world units
		m_chunkOffset = glm::vec3(m_chunkPosition) * glm::vec3(VX_CHUNK_SIZE_XZ_F, 
															   VX_CHUNK_SIZE_Y_F, 
															   VX_CHUNK_SIZE_XZ_F);

		// set dimensions
		m_aabb.min = m_chunkOffset;
		m_aabb.max = glm::vec3(VX_CHUNK_SIZE_XZ_F, 
							   VX_CHUNK_SIZE_Y_F, 
							   VX_CHUNK_SIZE_XZ_F) + m_chunkOffset;

		// allocate and reset voxel data
		ALLOCATE_3D_ARRAY(m_voxels, Voxel, VX_CHUNK_SIZE_XZ, VX_CHUNK_SIZE_Y, VX_CHUNK_SIZE_XZ);

		// generate some voxel data
		generateVoxels();

	}

	//-----------------------------------------------------------------------------------
	MeshVxChunk::MeshVxChunk(VxChunkManager* _parent_ptr, 
							 const glm::ivec3& _chunk_position, 
							 const Ref<Texture2DNoise>& _texture_ptr) :
		m_parentManager(_parent_ptr), m_chunkPosition(_chunk_position)
	{
		// calculate offset in world units
		m_chunkOffset = glm::vec3(m_chunkPosition) * glm::vec3(VX_CHUNK_SIZE_XZ_F, 
															   VX_CHUNK_SIZE_Y_F, 
															   VX_CHUNK_SIZE_XZ_F);

		// set dimensions
		m_aabb.min = m_chunkOffset;
		m_aabb.max = glm::vec3(VX_CHUNK_SIZE_XZ_F, 
							   VX_CHUNK_SIZE_Y_F, 
							   VX_CHUNK_SIZE_XZ_F) + m_chunkOffset;

		// allocate and reset voxel data
		ALLOCATE_3D_ARRAY(m_voxels, Voxel, VX_CHUNK_SIZE_XZ, VX_CHUNK_SIZE_Y, VX_CHUNK_SIZE_XZ);

		// generate the voxels
		generateVoxelsFromTexture(_texture_ptr);

	}

	//-----------------------------------------------------------------------------------
	MeshVxChunk::~MeshVxChunk()
	{
		if (m_voxels != nullptr)
		{
			FREE_3D_ARRAY(m_voxels, VX_CHUNK_SIZE_XZ, VX_CHUNK_SIZE_Y);
		}

		#ifdef VX_THREADED_MESHING
			if (m_verticesT != nullptr)
			{
				FREE_2D_ARRAY(m_verticesT, 6);
			}
		#else
			if (m_vertices != nullptr)
				delete[] m_vertices;
		#endif


	}
	
	//-----------------------------------------------------------------------------------
	void MeshVxChunk::generateVoxels()
	{
		// TODO : remove this, replaced by either generateVoxelsFromNoise() or generateVoxelsFromTexture()
		// Later maybe replaced by some sort of BiomeGenerator class call
		// m_parentManager->m_biomeGenerator->setNoiseParameters(chunk.x, chunk.z)?
		// for now static
		// Create the voxel data; Perlin fractal noise in 5 octaves.
		Noise::set_param_base_amp(1.0f);
		Noise::set_param_base_frequency(0.01f);
		Noise::set_param_octave_count(5);

		//float nmin = 1000.0f, nmax = -1000.0f;
		float step = 1.0f / (float)(1<<23);
		VxNoise::set_frequency_value(5.0f * (float)(1<<16));

		VX_FOR_BEGIN_DEF
			glm::vec2 v = (glm::vec2(x, z) + glm::vec2(m_chunkOffset.x, m_chunkOffset.z));
			//glm::vec3 v = glm::vec3(x, y, z) + m_chunkOffset;
			//float n = Noise::fbm_perlin3(v);
			//float n = VxNoise::value3_fbm_r(v*step);
			float n = VxNoise::value2_fbm_r(v*step);
			unsigned char h = static_cast<unsigned char>(n * 255.0f * 0.2f);
			
			//nmin = MIN(nmin, h);
			//nmax = MAX(nmax, h);

			if (y + m_chunkOffset.y < h)
				m_voxels[x][y][z].data = 1;
			else
				m_voxels[x][y][z].data = 0;
		VX_FOR_END
		
		//printf("(%2.0f %2.0f %2.0f) n[ %.3f ... %.3f ] (r = %.2f)\n", m_chunkOffset.x, m_chunkOffset.y, m_chunkOffset.z, nmin, nmax, nmax-nmin);
	}


	//-----------------------------------------------------------------------------------
	void MeshVxChunk::generateVoxelsFromTexture(const Ref<Texture2DNoise>& _texture_ptr)
	{
		// pointer to data
		float* noise = _texture_ptr->getNoiseData();
		
		// widht of texture (which is assumed to be m_width * m_width), used for indexing
		// into 1D noise array.
		uint32_t width = _texture_ptr->getWidth();

		// generate voxels from texture data
		int x0, z0;
		VX_FOR_BEGIN_DEF
			x0 = x + static_cast<int>(m_chunkOffset.x);
			z0 = z + static_cast<int>(m_chunkOffset.z);
			// offset into 1D array of noise values based on offset of this chunk
			int offsetIndex = z0 * width + x0;
			// get noise from texture heightmap
			float n = noise[offsetIndex] * m_meshScale;
			// recalculate into a height [0..255]
			int h = static_cast<unsigned char>(n * 255.0f);

			// fill voxels with a world space y coord lower than the scaled heightmap value
			if (y + m_chunkOffset.y < h)
				m_voxels[x][y][z] = 1;
			else
				m_voxels[x][y][z] = 0;
		VX_FOR_END
	}

	//-----------------------------------------------------------------------------------
	#ifdef VX_THREADED_MESHING
	void MeshVxChunk::generateMeshThreaded()
	{
		//Timer t("threaded meshing");

		// allocate vertex memory
		//NEW_2D_ARRAY(m_verticesT, VoxelVertex, 6, VX_CHUNK_SIZE_XZ_2 * VX_CHUNK_SIZE_Y * 2);
		ALLOCATE_2D_ARRAY(m_verticesT, VoxelVertex, 6, VX_CHUNK_SIZE_XZ_2 * VX_CHUNK_SIZE_Y * 2);

		// thread syncing
		std::vector<std::future<ThreadMeshResult>> threadedMeshResult;

		// perform meshing through six threads
		for (int i = 0; i < 6; i++)
		{
			threadedMeshResult.push_back(std::async(std::launch::async,
													&MeshVxChunk::generateMeshDirection,
													this,
													m_verticesT[i],
													i));
		}

		// sync and get number of written vertices from each thread
		std::vector<ThreadMeshResult> results;
		// reset vertex count if this is a remesh
		m_vertexCount = 0;
		// sync and collect worker thread results
		for (auto& threadReturn : threadedMeshResult)
		{
			// store direction and number of meshed vertices
			ThreadMeshResult res = threadReturn.get();
			m_vertexCount += res.meshedVertexCount;
			results.push_back(res);
		}

		// prepare data for upload; vertex buffer and array
		Ref<VertexBuffer> vertexBuffer = MakeRef<VertexBuffer>();
		vertexBuffer->setBufferLayout({
			{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
			{ VERTEX_ATTRIB_LOCATION_NORMAL,   ShaderDataType::Float3, "a_normal" }
		});

		// add in vertices from all threads
		vertexBuffer->startDataBlock(m_vertexCount * sizeof(VoxelVertex));
		int offset = 0;
		for (auto& result : results)
		{
			vertexBuffer->addSubData(m_verticesT[result.meshingDirection], 
									 result.meshedVertexCount * sizeof(VoxelVertex), 
									 offset);
			offset += result.meshedVertexCount * sizeof(VoxelVertex);
		}
				
		m_vertexArray = MakeRef<VertexArray>();
		m_vertexArray->setVertexBuffer(vertexBuffer);

		// upload
		Renderer::executeRenderCommands();

		// deallocate memory
		//DELETE_2D_ARRAY(m_verticesT, 6);
		FREE_2D_ARRAY(m_verticesT, 6);

	}
	#endif
	
	
	//-----------------------------------------------------------------------------------
	#ifdef VX_THREADED_MESHING
	ThreadMeshResult MeshVxChunk::generateMeshDirection(VoxelVertex* _vertices_ptr, int _direction)
	{
		int vertexCount = 0;
		int voxelMask[VX_CHUNK_SIZE_XZ * VX_CHUNK_SIZE_Y];

		size_t xyz[] = { 0, 0, 0 };
		size_t max[] = { VX_CHUNK_SIZE_XZ, VX_CHUNK_SIZE_Y, VX_CHUNK_SIZE_XZ };

		size_t d0 = (_direction + 0) % 3;
		size_t d1 = (_direction + 1) % 3;
		size_t d2 = (_direction + 2) % 3;

		int backface = _direction / 3 * 2 - 1; // -1, -1, -1, 1, 1, 1

		for (xyz[d0] = 0; xyz[d0] < max[d0]; xyz[d0]++)
		{
			for (xyz[d1] = 0; xyz[d1] < max[d1]; xyz[d1]++)
			{
				for (xyz[d2] = 0; xyz[d2] < max[d2]; xyz[d2]++)
				{
					int currentVoxel = getVoxelType(xyz[0], xyz[1], xyz[2]);
					if (currentVoxel)
					{
						// check next voxel in this pos by adding backface which follows the direction
						xyz[d0] += backface;

						if (getVoxelType(xyz[0], xyz[1], xyz[2]) > 0)
							voxelMask[xyz[d1] * max[d2] + xyz[d2]] = 0;
						else
							voxelMask[xyz[d1] * max[d2] + xyz[d2]] = currentVoxel;

						// return to the current coordinate
						xyz[d0] -= backface;
					}
					else
						voxelMask[xyz[d1] * max[d2] + xyz[d2]] = 0;
				}
			}

			// Mesh this slice (mask)
			for (xyz[d1] = 0; xyz[d1] < max[d1]; xyz[d1]++)
			{
				for (xyz[d2] = 0; xyz[d2] < max[d2]; /*xyz[d2]++*/)
				{
					int vxType = voxelMask[xyz[d1] * max[d2] + xyz[d2]];

					// skip air
					if (vxType == 0)
					{
						xyz[d2]++;
						continue;
					}

					// find longest line
					size_t width = 1;
					for (size_t d22 = xyz[d2] + 1; d22 < max[d2]; d22++)
					{
						if (voxelMask[xyz[d1] * max[d2] + d22] != vxType)
							break;
						width++;
					}

					// find largest rectangle
					size_t height = 1;
					bool done = false;
					for (size_t d11 = xyz[d1] + 1; d11 < max[d1]; d11++)
					{
						// find lines of same length
						for (size_t d22 = xyz[d2]; d22 < xyz[d2] + width; d22++)
						{
							if (voxelMask[d11 * max[d2] + d22] != vxType)
							{
								done = true;
								break;
							}
						}

						if (done)
							break;
						height++;
					}

					// prepare meshing
					float w[3] = { 0.0f, 0.0f, 0.0f };
					w[d2] = (float)width;
					float h[3] = { 0.0f, 0.0f, 0.0f };
					h[d1] = (float)height;

					glm::vec3 v = glm::vec3((float)xyz[0], (float)xyz[1], (float)xyz[2]);

					// shift front faces one block
					if (backface > 0)
					{
						float f[3] = { 0.0f, 0.0f, 0.0f };
						f[d0] += 1.0f;
						v += glm::vec3(f[0], f[1], f[2]);
					}

					// perform the actual meshing
					switch (_direction)
					{
						// positive x
						case VX_CHUNK_MESH_DIR_POS_X:
						{
							_vertices_ptr[vertexCount + 0].position = v + glm::vec3(w[0], w[1], w[2]);
							_vertices_ptr[vertexCount + 1].position = v + glm::vec3(w[0] + h[0], w[1] + h[1], w[2] + h[2]);
							_vertices_ptr[vertexCount + 2].position = v + glm::vec3(h[0], h[1], h[2]);
							_vertices_ptr[vertexCount + 3].position = v;
							_vertices_ptr[vertexCount + 4].position = v + glm::vec3(w[0], w[1], w[2]);
							_vertices_ptr[vertexCount + 5].position = v + glm::vec3(h[0], h[1], h[2]);

							assignFaceNormal(_vertices_ptr, vertexCount, glm::vec3(-1.0f, 0.0f, 0.0f));

							vertexCount += 6;
							break;
						}
						// positive y
						case VX_CHUNK_MESH_DIR_POS_Y:
						{
							_vertices_ptr[vertexCount + 0].position = v + glm::vec3(w[0], w[1], w[2]);
							_vertices_ptr[vertexCount + 1].position = v + glm::vec3(w[0] + h[0], w[1] + h[1], w[2] + h[2]);
							_vertices_ptr[vertexCount + 2].position = v + glm::vec3(h[0], h[1], h[2]);
							_vertices_ptr[vertexCount + 3].position = v;
							_vertices_ptr[vertexCount + 4].position = v + glm::vec3(w[0], w[1], w[2]);
							_vertices_ptr[vertexCount + 5].position = v + glm::vec3(h[0], h[1], h[2]);

							assignFaceNormal(_vertices_ptr, vertexCount, glm::vec3(0.0f, -1.0f, 0.0f));

							vertexCount += 6;
							break;
						}
						// positive z
						case VX_CHUNK_MESH_DIR_POS_Z:
						{
							_vertices_ptr[vertexCount + 0].position = v + glm::vec3(w[0], w[1], w[2]);
							_vertices_ptr[vertexCount + 1].position = v + glm::vec3(w[0] + h[0], w[1] + h[1], w[2] + h[2]);
							_vertices_ptr[vertexCount + 2].position = v + glm::vec3(h[0], h[1], h[2]);
							_vertices_ptr[vertexCount + 3].position = v;
							_vertices_ptr[vertexCount + 4].position = v + glm::vec3(w[0], w[1], w[2]);
							_vertices_ptr[vertexCount + 5].position = v + glm::vec3(h[0], h[1], h[2]);

							assignFaceNormal(_vertices_ptr, vertexCount, glm::vec3(0.0f, 0.0f, -1.0f));

							vertexCount += 6;
							break;
						}

						// negative x
						case VX_CHUNK_MESH_DIR_NEG_X:
						{
							_vertices_ptr[vertexCount + 0].position = v + glm::vec3(w[0] + h[0], w[1] + h[1], w[2] + h[2]);
							_vertices_ptr[vertexCount + 1].position = v + glm::vec3(w[0], w[1], w[2]);
							_vertices_ptr[vertexCount + 2].position = v + glm::vec3(h[0], h[1], h[2]);
							_vertices_ptr[vertexCount + 3].position = v + glm::vec3(w[0], w[1], w[2]);
							_vertices_ptr[vertexCount + 4].position = v;
							_vertices_ptr[vertexCount + 5].position = v + glm::vec3(h[0], h[1], h[2]);

							assignFaceNormal(_vertices_ptr, vertexCount, glm::vec3(1.0f, 0.0f, 0.0f));

							vertexCount += 6;
							break;
						}
						// negative y
						case VX_CHUNK_MESH_DIR_NEG_Y:
						{
							_vertices_ptr[vertexCount + 0].position = v + glm::vec3(w[0] + h[0], w[1] + h[1], w[2] + h[2]);
							_vertices_ptr[vertexCount + 1].position = v + glm::vec3(w[0], w[1], w[2]);
							_vertices_ptr[vertexCount + 2].position = v + glm::vec3(h[0], h[1], h[2]);
							_vertices_ptr[vertexCount + 3].position = v + glm::vec3(w[0], w[1], w[2]);
							_vertices_ptr[vertexCount + 4].position = v;
							_vertices_ptr[vertexCount + 5].position = v + glm::vec3(h[0], h[1], h[2]);

							assignFaceNormal(_vertices_ptr, vertexCount, glm::vec3(0.0f, 1.0f, 0.0f));

							vertexCount += 6;
							break;
						}
						// negative z
						case VX_CHUNK_MESH_DIR_NEG_Z:
						{
							_vertices_ptr[vertexCount + 0].position = v + glm::vec3(w[0] + h[0], w[1] + h[1], w[2] + h[2]);
							_vertices_ptr[vertexCount + 1].position = v + glm::vec3(w[0], w[1], w[2]);
							_vertices_ptr[vertexCount + 2].position = v + glm::vec3(h[0], h[1], h[2]);
							_vertices_ptr[vertexCount + 3].position = v + glm::vec3(w[0], w[1], w[2]);
							_vertices_ptr[vertexCount + 4].position = v;
							_vertices_ptr[vertexCount + 5].position = v + glm::vec3(h[0], h[1], h[2]);

							assignFaceNormal(_vertices_ptr, vertexCount, glm::vec3(0.0f, 0.0f, 1.0f));
							
							vertexCount += 6;
							break;
						}
					}

					// Zero (meshed) quad in slice
					for (size_t d11 = xyz[d1]; d11 < xyz[d1] + height; d11++)
						for (size_t d22 = xyz[d2]; d22 < xyz[d2] + width; d22++)
							voxelMask[d11 * max[d2] + d22] = 0;

					// Advance search position for next quad.
					xyz[d2] += width;
				}
			}
		}

		ThreadMeshResult result;
		result.meshingDirection = _direction;
		result.meshedVertexCount = vertexCount;

		return result;
	}
	#endif


	//-----------------------------------------------------------------------------------
	/* inline */void MeshVxChunk::assignFaceNormal(VoxelVertex* _array_ptr, int _offset, const glm::vec3& _normal)
	{
		_array_ptr[_offset + 0].normal = _normal;
		_array_ptr[_offset + 1].normal = _normal;
		_array_ptr[_offset + 2].normal = _normal;
		_array_ptr[_offset + 3].normal = _normal;
		_array_ptr[_offset + 4].normal = _normal;
		_array_ptr[_offset + 5].normal = _normal;
	}


	//-----------------------------------------------------------------------------------
	#ifndef VX_THREADED_MESHING
	void MeshVxChunk::generateMesh()
	{
		//Timer t("meshing (incl. GPU upload)");

		// allocate vertex memory
		m_vertices = new VoxelVertex[VX_CHUNK_SIZE_XZ_2 * VX_CHUNK_SIZE_Y * 12];

		// vertex count
		m_vertexCount = 0;
		// normal vector
		glm::vec3 n;

		/* mask for meshing -- the size of one face of the 'chunk cube', i.e. size^2. */
		int voxelMask[VX_CHUNK_SIZE_XZ * VX_CHUNK_SIZE_Y];
		memset(&voxelMask, 0, sizeof(int) * VX_CHUNK_SIZE_XZ * VX_CHUNK_SIZE_Y);

		size_t xyz[] = { 0, 0, 0 };
		size_t max[] = { VX_CHUNK_SIZE_XZ, VX_CHUNK_SIZE_Y, VX_CHUNK_SIZE_XZ };

		for (size_t i = 0; i < 6; i++)
		{
			// (-x -y -z +x +y +z)
			size_t d0 = (i + 0) % 3;
			size_t d1 = (i + 1) % 3;
			size_t d2 = (i + 2) % 3;

			int backface = i / 3 * 2 - 1; // -1, -1, -1, 1, 1, 1

			for (xyz[d0] = 0; xyz[d0] < max[d0]; xyz[d0]++)
			{
				for (xyz[d1] = 0; xyz[d1] < max[d1]; xyz[d1]++)
				{
					for (xyz[d2] = 0; xyz[d2] < max[d2]; xyz[d2]++)
					{
						int currentVoxel = getVoxelType(xyz[0], xyz[1], xyz[2]);
						if (currentVoxel)
						{
							// check next voxel in this pos by adding backface which follows the direction
							xyz[d0] += backface;

							if (getVoxelType(xyz[0], xyz[1], xyz[2]) > 0)
								voxelMask[xyz[d1] * max[d2] + xyz[d2]] = 0;
							else
								voxelMask[xyz[d1] * max[d2] + xyz[d2]] = currentVoxel;

							// return to the current coordinate
							xyz[d0] -= backface;
						}
						else
							voxelMask[xyz[d1] * max[d2] + xyz[d2]] = 0;
					}
				}
				
				// Mesh this slice (mask)
				for (xyz[d1] = 0; xyz[d1] < max[d1]; xyz[d1]++)
				{
					for (xyz[d2] = 0; xyz[d2] < max[d2]; /*xyz[d2]++*/)
					{
						int vxType = voxelMask[xyz[d1] * max[d2] + xyz[d2]];

						// skip air
						if (vxType == 0)
						{
							xyz[d2]++;
							continue;
						}

						// find longest line
						size_t width = 1;
						for (size_t d22 = xyz[d2] + 1; d22 < max[d2]; d22++)
						{
							if (voxelMask[xyz[d1] * max[d2] + d22] != vxType)
								break;
							width++;
						}

						// find largest rectangle
						size_t height = 1;
						bool done = false;
						for (size_t d11 = xyz[d1] + 1; d11 < max[d1]; d11++)
						{
							// find lines of same length
							for (size_t d22 = xyz[d2]; d22 < xyz[d2] + width; d22++)
							{
								if (voxelMask[d11 * max[d2] + d22] != vxType)
								{
									done = true;
									break;
								}
							}

							if (done)
								break;
							height++;
						}

						// prepare meshing
						float w[3] = { 0.0f, 0.0f, 0.0f };
						w[d2] = (float)width;
						float h[3] = { 0.0f, 0.0f, 0.0f };
						h[d1] = (float)height;

						glm::vec3 v = glm::vec3((float)xyz[0], (float)xyz[1], (float)xyz[2]);
						//glm::vec3 v = glm::vec3(xyz[0], xyz[1], xyz[2]);

						// shift front faces one block
						if (backface > 0)
						{
							float f[3] = { 0.0f, 0.0f, 0.0f };
							f[d0] += 1.0f;
							v += glm::vec3(f[0], f[1], f[2]);
						}

						// perform the actual meshing
						switch (i)
						{
							// positive x
							case 0:
							{
								m_vertices[m_vertexCount + 0].position = v + glm::vec3(w[0], w[1], w[2]);
								m_vertices[m_vertexCount + 1].position = v + glm::vec3(w[0] + h[0], w[1] + h[1], w[2] + h[2]);
								m_vertices[m_vertexCount + 2].position = v + glm::vec3(h[0], h[1], h[2]);
								m_vertices[m_vertexCount + 3].position = v;
								m_vertices[m_vertexCount + 4].position = v + glm::vec3(w[0], w[1], w[2]);
								m_vertices[m_vertexCount + 5].position = v + glm::vec3(h[0], h[1], h[2]);

								assignFaceNormal(m_vertices, m_vertexCount, glm::vec3(-1.0f, 0.0f, 0.0f));

								m_vertexCount += 6;
								break;
							}
							// positive y
							case 1:
							{
								m_vertices[m_vertexCount + 0].position = v + glm::vec3(w[0], w[1], w[2]);
								m_vertices[m_vertexCount + 1].position = v + glm::vec3(w[0] + h[0], w[1] + h[1], w[2] + h[2]);
								m_vertices[m_vertexCount + 2].position = v + glm::vec3(h[0], h[1], h[2]);
								m_vertices[m_vertexCount + 3].position = v;
								m_vertices[m_vertexCount + 4].position = v + glm::vec3(w[0], w[1], w[2]);
								m_vertices[m_vertexCount + 5].position = v + glm::vec3(h[0], h[1], h[2]);

								assignFaceNormal(m_vertices, m_vertexCount, glm::vec3(0.0f, -1.0f, 0.0f));

								m_vertexCount += 6;
								break;
							}
							// positive z
							case 2:
							{
								m_vertices[m_vertexCount + 0].position = v + glm::vec3(w[0], w[1], w[2]);
								m_vertices[m_vertexCount + 1].position = v + glm::vec3(w[0] + h[0], w[1] + h[1], w[2] + h[2]);
								m_vertices[m_vertexCount + 2].position = v + glm::vec3(h[0], h[1], h[2]);
								m_vertices[m_vertexCount + 3].position = v;
								m_vertices[m_vertexCount + 4].position = v + glm::vec3(w[0], w[1], w[2]);
								m_vertices[m_vertexCount + 5].position = v + glm::vec3(h[0], h[1], h[2]);

								assignFaceNormal(m_vertices, m_vertexCount, glm::vec3(0.0f, 0.0f, -1.0f));

								m_vertexCount += 6;
								break;
							}

							// negative x
							case 3:
							{
								m_vertices[m_vertexCount + 0].position = v + glm::vec3(w[0] + h[0], w[1] + h[1], w[2] + h[2]);
								m_vertices[m_vertexCount + 1].position = v + glm::vec3(w[0], w[1], w[2]);
								m_vertices[m_vertexCount + 2].position = v + glm::vec3(h[0], h[1], h[2]);
								m_vertices[m_vertexCount + 3].position = v + glm::vec3(w[0], w[1], w[2]);
								m_vertices[m_vertexCount + 4].position = v;
								m_vertices[m_vertexCount + 5].position = v + glm::vec3(h[0], h[1], h[2]);

								assignFaceNormal(m_vertices, m_vertexCount, glm::vec3(1.0f, 0.0f, 0.0f));

								m_vertexCount += 6;
								break;
							}
							// negative y
							case 4:
							{
								m_vertices[m_vertexCount + 0].position = v + glm::vec3(w[0] + h[0], w[1] + h[1], w[2] + h[2]);
								m_vertices[m_vertexCount + 1].position = v + glm::vec3(w[0], w[1], w[2]);
								m_vertices[m_vertexCount + 2].position = v + glm::vec3(h[0], h[1], h[2]);
								m_vertices[m_vertexCount + 3].position = v + glm::vec3(w[0], w[1], w[2]);
								m_vertices[m_vertexCount + 4].position = v;
								m_vertices[m_vertexCount + 5].position = v + glm::vec3(h[0], h[1], h[2]);

								assignFaceNormal(m_vertices, m_vertexCount, glm::vec3(0.0f, 1.0f, 0.0f));

								m_vertexCount += 6;
								break;
							}
							// negative z
							case 5:
							{
								m_vertices[m_vertexCount + 0].position = v + glm::vec3(w[0] + h[0], w[1] + h[1], w[2] + h[2]);
								m_vertices[m_vertexCount + 1].position = v + glm::vec3(w[0], w[1], w[2]);
								m_vertices[m_vertexCount + 2].position = v + glm::vec3(h[0], h[1], h[2]);
								m_vertices[m_vertexCount + 3].position = v + glm::vec3(w[0], w[1], w[2]);
								m_vertices[m_vertexCount + 4].position = v;
								m_vertices[m_vertexCount + 5].position = v + glm::vec3(h[0], h[1], h[2]);

								assignFaceNormal(m_vertices, m_vertexCount, glm::vec3(0.0f, 0.0f, 1.0f));

								m_vertexCount += 6;
								break;
							}
						}

						// Zero (meshed) quad in slice
						for (size_t d11 = xyz[d1]; d11 < xyz[d1] + height; d11++)
							for (size_t d22 = xyz[d2]; d22 < xyz[d2] + width; d22++)
								voxelMask[d11 * max[d2] + d22] = 0;

						// Advance search position for next quad.
						xyz[d2] += width;
					}
				}
			}
		}

		{
			//Timer t0("vao setup");

			Ref<VertexBuffer> vertexBuffer = MakeRef<VertexBuffer>();
			vertexBuffer->setBufferLayout({
				{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
				{ VERTEX_ATTRIB_LOCATION_NORMAL,   ShaderDataType::Float3, "a_normal" }
			});
			vertexBuffer->setData(m_vertices, sizeof(VoxelVertex) * m_vertexCount);

			// no indices -- render with Renderer::drawArrays(0, m_vertexCount)
			m_vertexArray = MakeRef<VertexArray>();
			m_vertexArray->setVertexBuffer(vertexBuffer);

			// upload
			Renderer::executeRenderCommands();
		}

		// deallocate vertex memory
		delete[] m_vertices;
	}
	#endif

	//-----------------------------------------------------------------------------------
	void MeshVxChunk::render(const Ref<Shader>& _shader_ptr)
	{
		if (m_vertexArray == nullptr)// || !m_vertexCount)
			return;

		// translate by chunk position offset
		m_transform.translateReset(m_chunkOffset);
		_shader_ptr->setMatrix4fv("u_model_matrix", m_transform.getModelMatrix());
		
		m_vertexArray->bind();
		// draw non-indexed
		Renderer::drawArrays(m_vertexCount, 0, true, GL_TRIANGLES);
	}


}

