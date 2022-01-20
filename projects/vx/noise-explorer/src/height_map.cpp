
#include <Synapse/API/OpenGLBindings.hpp>
#include <Synapse/Utils/Random/Random.hpp>
#include <Synapse/Utils/Timer/Timer.hpp>
#include <Synapse/Utils/Noise/Noise.hpp>

#include "height_map.hpp"



namespace Syn 
{

	HeightMap::HeightMap(uint32_t _width, uint32_t _resolution) :
		m_constructed(false), 
		m_width(_width),
		m_resolution(_resolution)
	{
        m_generator = new cnoise;
		m_generator->set_octaves(7);
        
	}


	//-----------------------------------------------------------------------------------
	HeightMap::~HeightMap()
	{
		if (m_vertices)
			delete[] m_vertices;
		if (m_indices)
			delete[] m_indices;
        if (m_generator)
            delete m_generator;
	}


	//-----------------------------------------------------------------------------------
	glm::vec2 HeightMap::computeMeshSliceVertices(mesh_vertex_t* _data, uint32_t _z_begin, uint32_t _z_end)
	{
		//printf("m_stepSize = %.2f, m_width = %d, m_resolution = %d\n", m_stepSize, m_width, m_resolution);
		glm::vec2 y_lim;
		y_lim.x = std::numeric_limits<float>::max();
		y_lim.y = std::numeric_limits<float>::min();

		NoiseParameters params;
		params.octaveCount = 3;
		params.baseAmp = 40.0f;
		params.baseFreq = 0.02f;
		Noise::set_noise_parameters(params);

		for (uint32_t z = _z_begin; z < _z_end; z++)
		{
			for (uint32_t x = 0; x < m_resolution + 1; x++)
			{
				//glm::vec2 v = glm::vec2(m_width * (x * m_stepSize), m_width * (z * m_stepSize));
				glm::vec3 v = glm::vec3(m_width * x * m_stepSize, 0, m_width * z * m_stepSize);
				// compute noise and derivatives
				glm::vec4 n = Noise::fbm_perlin3_d(v);
				glm::vec3 derivs = { n.y, n.z, n.w };
				// store min/max y
				y_lim.x = std::min(y_lim.x, n.x);
				y_lim.y = std::max(y_lim.y, n.x);

				// store for upload
				uint32_t index = z * (m_resolution + 1) + x;
				_data[index].position = glm::vec3(v.x, n.x, v.z);
				_data[index].normal = glm::normalize(glm::vec3(-derivs.x, 1.0f, -derivs.z));
				_data[index].tangent = glm::normalize(glm::vec3(1.0f, derivs.x, 0.0f));
				_data[index].bitangent = glm::normalize(glm::vec3(0.0f, derivs.z, 1.0f));
			}
		}

		return y_lim;
	}


	//-----------------------------------------------------------------------------------
	void HeightMap::computeMeshSliceIndices(uint32_t* _data, uint32_t _z_begin, uint32_t _z_end)
	{
		for (uint32_t z = _z_begin; z < _z_end; z++)
		{
			for (uint32_t x = 0; x < m_resolution; x++)
			{
				uint32_t index = 6 * (z * m_resolution + x);
				// upper left triangle				
				_data[index+0] = z * (m_resolution + 1) + x;
				_data[index+1] = (z + 1) * (m_resolution + 1) + x;
				_data[index+2] = z * (m_resolution + 1) + x + 1;
				// lower right triangle
				_data[index+3] = z * (m_resolution + 1) + x + 1;
				_data[index+4] = (z + 1) * (m_resolution + 1) + x;
				_data[index+5] = (z + 1) * (m_resolution + 1) + x + 1;
			}
		}
	}


	//-----------------------------------------------------------------------------------
	void HeightMap::setupVertexData()
	{
        Timer timer(__func__);

		// check if resolution is power of 2
		if (ceil(log2(m_resolution)) != floor(log2(m_resolution)))
		{
			SYN_CORE_WARNING("resolution of mesh needs to be power of 2. No geometry set.");
			return;
		}

		// set aabb for x and z
		m_aabb.min = glm::vec3(0);
		m_aabb.max = glm::vec3(m_width, 0, m_width);
		m_aabb.min.y = std::numeric_limits<float>::max();
		m_aabb.max.y = std::numeric_limits<float>::lowest();


		// compute vertices
		//
		// vertices, noise and partial derivatives
		m_vertexCount = (m_resolution + 1) * (m_resolution + 1);
		m_vertices = new mesh_vertex_t[m_vertexCount];
		#ifdef DEBUG_MESH_TERRAIN
			SYN_CORE_TRACE("allocating vertex memory: ", m_vertexCount * sizeof(mesh_vertex_t));
		#endif
		m_stepSize = 1.0f / m_resolution;

		// prepare threading of mesh computation
		uint32_t threadCount = std::max(2U, std::thread::hardware_concurrency());
		uint32_t zSlicesPerThread = m_resolution / threadCount;

		// threaded meshing
		std::vector<std::future<glm::vec2>> threadedVertexResult;
		for (uint32_t i = 0; i < threadCount; i++)
		{
			// all except the last thread
			if (i < threadCount - 1)
			{
				threadedVertexResult.push_back(std::async(
					std::launch::async,
					&HeightMap::computeMeshSliceVertices,
					this,
					m_vertices,
					i * zSlicesPerThread,
					(i + 1) * zSlicesPerThread
				));
			}
			/* This will be the last thread, so we need to make sure that all vertices
				* are computed. Since zSlicesPerThread * threadCount may not be equal
				* to vertexCount, this last thread takes all remaining vertices up to
				* _z_end = vertexCount / (resolution + 1).
				*/
			else
			{
				threadedVertexResult.push_back(std::async(
					std::launch::async,
					&HeightMap::computeMeshSliceVertices,
					this,
					m_vertices,
					i * zSlicesPerThread,
					m_resolution + 1
				));
			}
		}

		// sync vertex meshing threads
		for (auto& res : threadedVertexResult)
		{
			glm::vec2 y_lim = res.get();
			// update AABB from threads
			m_aabb.min.y = std::min(m_aabb.min.y, y_lim.x);
			m_aabb.max.y = std::max(m_aabb.max.y, y_lim.y);
		}

		
		// compute indices
		//
		m_indexCount = m_resolution * m_resolution * 6;
		m_indices = new uint32_t[m_indexCount];
		#ifdef DEBUG_MESH_TERRAIN
			SYN_CORE_TRACE("allocating index memory: ", m_indexCount * sizeof(uint32_t));
		#endif

		/* Although indices is of size m_resolution^2 and m_resolution is
		 * always a power of 2, the number of threads MAY be odd, hence a similar
		 * approach as the vertices have to be employed.
		 */
		//zSlicesPerThread = m_resolution / threadCount;

		std::vector<std::future<void>> threadedIndexResult;

		for (uint32_t i = 0; i < threadCount; i++)
		{
			if (i < threadCount - 1)
			{
				threadedIndexResult.push_back(std::async(
					std::launch::async,
					&HeightMap::computeMeshSliceIndices,
					this,
					m_indices,
					i * zSlicesPerThread,
					(i + 1) * zSlicesPerThread
				));
			}
			else
			{
				threadedIndexResult.push_back(std::async(
					std::launch::async,
					&HeightMap::computeMeshSliceIndices,
					this,
					m_indices,
					i * zSlicesPerThread,
					m_resolution
				));
			}
		}

		// sync indexing threads before GPU upload
		for (auto& res : threadedIndexResult)
			res.get();


		// GPU upload
		//
		m_vertexArray = MeshCreator::createVertexArray(
			m_vertices,
			sizeof(mesh_vertex_t) * m_vertexCount,
			m_indices,
			m_indexCount,
			MESH_ATTRIB_POSITION | MESH_ATTRIB_NORMAL | MESH_ATTRIB_TANGENT | MESH_ATTRIB_BITANGENT,
            GL_DYNAMIC_DRAW
		);
		

		// debug : lines enclosing chunk
		//std::vector<glm::vec3> lines;
		//lines.push_back(glm::vec3(m_aabb.min.x, 0.0f, m_aabb.min.z));
		//lines.push_back(glm::vec3(m_aabb.max.x, 0.0f, m_aabb.min.z));
		//lines.push_back(glm::vec3(m_aabb.max.x, 0.0f, m_aabb.max.z));
		//lines.push_back(glm::vec3(m_aabb.min.x, 0.0f, m_aabb.max.z));
		//lines.push_back(glm::vec3(m_aabb.min.x, 0.0f, m_aabb.min.z));
		//MeshCreator::createDebugLineStrip(lines);

		// set flag
		m_constructed = true;

	}


	//-----------------------------------------------------------------------------------
	void HeightMap::render(const Ref<Shader>& _shader_ptr)
	{
		if (!this->isRenderable())
			return;

		//glm::mat4 modelMatrix = m_transform.getModelMatrix();
		//glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));
		//_shader_ptr->setMatrix4fv("u_model_matrix", glm::translate(modelMatrix, m_xzOffset));
		//_shader_ptr->setMatrix4fv("u_model_matrix", modelMatrix);
		//_shader_ptr->setMatrix3fv("u_normal_matrix", normalMatrix);
		m_vertexArray->bind();
		Renderer::drawIndexed(m_vertexArray->getIndexCount());
	}

}


