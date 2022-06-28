
#include "pch.hpp"

#include <limits>

#include "Synapse/Renderer/MeshCreator.hpp"

#include "Synapse/Core.hpp"
#include "Synapse/Renderer/Shader/ShaderLibrary.hpp"
#include "Synapse/Utils/FileIOHandler.hpp"
#include "Synapse/Utils/Timer/TimeStep.hpp"
#include "Synapse/Utils/Timer/Timer.hpp"
#include "Synapse/Utils/MathUtils.hpp"


namespace Syn 
{

	// static declarations
	std::multimap<MeshDebugType, Ref<MeshDebug>> MeshCreator::s_MeshDebugMap;
	/*boost::*/std::unordered_map<std::string, Ref<MeshDebug>> MeshCreator::s_MeshDebugSearchMap;
	glm::vec3 MeshCreator::s_debugRenderColor = glm::vec3(1.0f, 0.0f, 1.0f);
	bool MeshCreator::s_shadersCreated = false;
	Ref<Shader> MeshCreator::m_debugGeneralShader = nullptr;
	Ref<Shader> MeshCreator::m_debugPointShader = nullptr;
	Ref<Shader> MeshCreator::m_debugGridShader = nullptr;
	uint32_t MeshCreator::s_meshIndex = 0;

	
	//-----------------------------------------------------------------------------------
	Ref<VertexArray> MeshCreator::createVertexArray(
		void* _vertices, 
		uint32_t _vertices_size_bytes, 
		void* _indices, 
		uint32_t _index_count, 
		uint32_t _mesh_attrib_flags,
		GLenum _usage)
	{
		// vertex buffer with flags and layout
		Ref<VertexBuffer> vertexBuffer = MakeRef<VertexBuffer>(_usage);

		std::vector<BufferElement> elements;
		if (_mesh_attrib_flags & MESH_ATTRIB_POSITION)
			elements.push_back(BufferElement({ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" }));
		if (_mesh_attrib_flags & MESH_ATTRIB_NORMAL)
			elements.push_back(BufferElement({ VERTEX_ATTRIB_LOCATION_NORMAL, ShaderDataType::Float3, "a_normal" }));
		if (_mesh_attrib_flags & MESH_ATTRIB_TANGENT)
			elements.push_back(BufferElement({ VERTEX_ATTRIB_LOCATION_TANGENT, ShaderDataType::Float3, "a_tangent" }));
		if (_mesh_attrib_flags & MESH_ATTRIB_BITANGENT)
			elements.push_back(BufferElement({ VERTEX_ATTRIB_LOCATION_BITANGENT, ShaderDataType::Float3, "a_bitangent" }));
		if (_mesh_attrib_flags & MESH_ATTRIB_UV)
			elements.push_back(BufferElement({ VERTEX_ATTRIB_LOCATION_UV, ShaderDataType::Float2, "a_uv" }));
		if (_mesh_attrib_flags & MESH_ATTRIB_COLOR)
			elements.push_back(BufferElement({ VERTEX_ATTRIB_LOCATION_COLOR, ShaderDataType::Float4, "a_color" }));

		vertexBuffer->setBufferLayout(elements);
		vertexBuffer->setData(_vertices, _vertices_size_bytes);

		// index buffer
		Ref<IndexBuffer> indexBuffer = MakeRef<IndexBuffer>();
		indexBuffer->setData(_indices, _index_count);

		Ref<VertexArray> vertexArray = MakeRef<VertexArray>(vertexBuffer, indexBuffer);
		Renderer::executeRenderCommands();

		// create and return the vertex array
		return vertexArray;

	}


	//-----------------------------------------------------------------------------------
	void MeshCreator::trimVertexData(vertex_data* _vertex_data, uint32_t _n_vertices, uint32_t _mesh_attrib_flags, float* _raw)
	{
		/* Loops the vertex_data and fills the raw pointer with data based on which flags
		 * are set. This is to permit the vertex buffer layout to point to the correct 
		 * attributes.
		 * _n_vertices are the number of sizeof(vertex_data) in the _vertex_data pointer.
		 * trimming.
		 * The _raw pointer has to be created and destroyed in the caller function.
		 */

		// index of the current address in _raw to be written to
		uint32_t rawIdx = 0;

		for (uint32_t i = 0; i < _n_vertices; i++)
		{
			vertex_data v = _vertex_data[i];
			if (_mesh_attrib_flags & MESH_ATTRIB_POSITION)
			{
				// write the data and update the index
				_raw[rawIdx+0] = v.position.x;
				_raw[rawIdx+1] = v.position.y;
				_raw[rawIdx+2] = v.position.z;
				rawIdx += (sizeof(glm::vec3) / sizeof(float));
			}

			if (_mesh_attrib_flags & MESH_ATTRIB_NORMAL)
			{
				_raw[rawIdx+0] = v.normal.x;
				_raw[rawIdx+1] = v.normal.y;
				_raw[rawIdx+2] = v.normal.z;
				rawIdx += (sizeof(glm::vec3) / sizeof(float));
			}
			
			if (_mesh_attrib_flags & MESH_ATTRIB_UV)
			{
				_raw[rawIdx+0] = v.uv.x;
				_raw[rawIdx+1] = v.uv.y;
				rawIdx += (sizeof(glm::vec2) / sizeof(float));
			}
		}
	}
	
	//-----------------------------------------------------------------------------------
	uint32_t MeshCreator::vertexSize(uint32_t _mesh_attrib_flags)
	{
		uint32_t vertexSize = 0;
		if (_mesh_attrib_flags & MESH_ATTRIB_POSITION)	vertexSize += sizeof(glm::vec3);
		if (_mesh_attrib_flags & MESH_ATTRIB_NORMAL)	vertexSize += sizeof(glm::vec3);
		if (_mesh_attrib_flags & MESH_ATTRIB_TANGENT)	vertexSize += sizeof(glm::vec3);
		if (_mesh_attrib_flags & MESH_ATTRIB_BITANGENT)	vertexSize += sizeof(glm::vec3);
		if (_mesh_attrib_flags & MESH_ATTRIB_UV)		vertexSize += sizeof(glm::vec2);
		if (_mesh_attrib_flags & MESH_ATTRIB_COLOR)		vertexSize += sizeof(glm::vec4);
		return vertexSize;
	}

	//-----------------------------------------------------------------------------------
	/*
	Ref<MeshShape> MeshCreator::createMeshShape(void* _vertices, uint32_t _vertices_size_bytes, void* _indices, uint32_t _index_count, uint32_t _mesh_attrib_flags)
	{
		// create mesh from vertex/index data
		Ref<MeshShape> mesh = MakeRef<MeshShape>();
		mesh->setVertexArray(createVertexArray(_vertices, _vertices_size_bytes, _indices, _index_count, _mesh_attrib_flags));
		return mesh;
	}
	*/

	//-----------------------------------------------------------------------------------
	Ref<MeshShape> MeshCreator::createMeshShape(vertex_data* _vertex_data, 
												uint32_t _vertices_size_bytes, 
												uint32_t* _indices, 
												uint32_t _index_count, 
												uint32_t _mesh_attrib_flags)
	{
		// create mesh for vertex/index data
		Ref<MeshShape> mesh = MakeRef<MeshShape>();
		// infer number of vertices in _vertex_data
		uint32_t vertexCount = _vertices_size_bytes / sizeof(vertex_data);
		// compute the number of bytes per vertex based on set flags
		uint32_t trimmedVertexSzBytes = vertexSize(_mesh_attrib_flags);
		// dimension raw float pointer to store trimmed vertex data
		float *raw = new float[vertexCount * trimmedVertexSzBytes / sizeof(float)];
		// check if trimming is necessary
		if (trimmedVertexSzBytes != sizeof(vertex_data))
		{
			// trim the vertex data
			trimVertexData(_vertex_data, vertexCount, _mesh_attrib_flags, raw);
			// set vertex array buffer and index buffer accordingly
			mesh->setVertexArray(createVertexArray((void*)raw, trimmedVertexSzBytes * vertexCount, (void*)_indices, _index_count, _mesh_attrib_flags));
		}
		// no trimming needed, we can use the original data
		else
			mesh->setVertexArray(createVertexArray((void*)_vertex_data, _vertices_size_bytes, (void*)_indices, _index_count, _mesh_attrib_flags));

		delete[] raw; // clean your room!

		return mesh;
	}

	//-----------------------------------------------------------------------------------
	Ref<MeshDebug> MeshCreator::createMeshDebug(const std::string& _mesh_name,
												MeshDebugType _type, 
												void* _vertices, 
												uint32_t _vertices_size_bytes, 
												void* _indices, 
												uint32_t _index_count
	)
	{
		if (!s_shadersCreated)
			MeshCreator::createDebugShaders();

		// create mesh from vertex/index data
		Ref<MeshDebug> mesh = MakeRef<MeshDebug>(_type);
		if (_type == MeshDebugType::GRID_PLANE_FS)
			mesh->setVertexArray(createVertexArray(_vertices, _vertices_size_bytes, _indices, _index_count, MESH_ATTRIB_POSITION | MESH_ATTRIB_UV, GL_DYNAMIC_DRAW));
		else
			mesh->setVertexArray(createVertexArray(_vertices, _vertices_size_bytes, _indices, _index_count, MESH_ATTRIB_POSITION, GL_DYNAMIC_DRAW));

		// catch-up with the render command queue
		Renderer::executeRenderCommands();

		// store reference in multimap and in searchable, string-keyed map
		s_MeshDebugMap.insert(std::pair<MeshDebugType, Ref<MeshDebug>>(_type, mesh));
		
		std::string meshName = _mesh_name;
		if (_mesh_name == "")
		{
			std::string n = MeshDebugTypeToString(_type);
			// extract type part (omit 'MeshDebugType::')
			meshName = n.substr(n.find_last_of(':') + 1);
			// convert to lower case
			std::transform(meshName.begin(), meshName.end(), meshName.begin(), [](unsigned char c) { return std::tolower(c); });
			// add index
			meshName += "_" + std::to_string(s_meshIndex++);
		}
		// insert
		s_MeshDebugSearchMap[meshName] = mesh;
		
		SYN_CORE_TRACE("created '", meshName, "' : ", _vertices_size_bytes / sizeof(glm::vec3), " vertices, ", _index_count, " indices.");

		// return ref
		return mesh;

	}

	// Regular meshes -------------------------------------------------------------------
	Ref<MeshShape> MeshCreator::createShapeCube(const glm::vec3& _center, float _diameter, uint32_t _mesh_attrib_flags)
	{
		// remove unusable flags
		uint32_t flags = _mesh_attrib_flags;
		flags = flags & ~(MESH_ATTRIB_TANGENT | MESH_ATTRIB_BITANGENT | MESH_ATTRIB_COLOR);

		float x0 = _center.x - _diameter;
		float x1 = _center.x + _diameter;
		float y0 = _center.y - _diameter;
		float y1 = _center.y + _diameter;
		float z0 = _center.z - _diameter;
		float z1 = _center.z + _diameter;

		vertex_data vertexData[] =
		{
			glm::vec3(x1, y0, z1), glm::vec3( 0.0, -1.0,  0.0),	glm::vec2(1.0, 1.0),
			glm::vec3(x0, y0, z1), glm::vec3( 0.0, -1.0,  0.0),	glm::vec2(0.0, 1.0),
			glm::vec3(x0, y0, z0), glm::vec3( 0.0, -1.0,  0.0),	glm::vec2(0.0, 0.0),
			glm::vec3(x0, y1, z0), glm::vec3( 0.0,  1.0,  0.0),	glm::vec2(0.0, 1.0),
			glm::vec3(x0, y1, z1), glm::vec3( 0.0,  1.0,  0.0),	glm::vec2(0.0, 0.0),
			glm::vec3(x1, y1, z1), glm::vec3( 0.0,  1.0,  0.0),	glm::vec2(1.0, 0.0),
			glm::vec3(x1, y1, z0), glm::vec3( 1.0,  0.0,  0.0),	glm::vec2(1.0, 1.0),
			glm::vec3(x1, y1, z1), glm::vec3( 1.0,  0.0,  0.0),	glm::vec2(0.0, 1.0),
			glm::vec3(x1, y0, z1), glm::vec3( 1.0,  0.0,  0.0),	glm::vec2(0.0, 0.0),
			glm::vec3(x1, y1, z1), glm::vec3( 0.0,  0.0,  1.0),	glm::vec2(1.0, 1.0),
			glm::vec3(x0, y1, z1), glm::vec3( 0.0,  0.0,  1.0),	glm::vec2(0.0, 1.0),
			glm::vec3(x0, y0, z1), glm::vec3( 0.0,  0.0,  1.0),	glm::vec2(0.0, 0.0),
			glm::vec3(x0, y0, z1), glm::vec3(-1.0,  0.0,  0.0),	glm::vec2(1.0, 0.0),
			glm::vec3(x0, y1, z1), glm::vec3(-1.0,  0.0,  0.0),	glm::vec2(1.0, 1.0),
			glm::vec3(x0, y1, z0), glm::vec3(-1.0,  0.0,  0.0),	glm::vec2(0.0, 1.0),
			glm::vec3(x1, y0, z0), glm::vec3( 0.0,  0.0, -1.0),	glm::vec2(0.0, 0.0),
			glm::vec3(x0, y0, z0), glm::vec3( 0.0,  0.0, -1.0),	glm::vec2(1.0, 0.0),
			glm::vec3(x0, y1, z0), glm::vec3( 0.0,  0.0, -1.0),	glm::vec2(1.0, 1.0),
			glm::vec3(x1, y0, z0), glm::vec3( 0.0, -1.0,  0.0),	glm::vec2(1.0, 0.0),
			glm::vec3(x1, y0, z1), glm::vec3( 0.0, -1.0,  0.0),	glm::vec2(1.0, 1.0),
			glm::vec3(x0, y0, z0), glm::vec3( 0.0, -1.0,  0.0),	glm::vec2(0.0, 0.0),
			glm::vec3(x1, y1, z0), glm::vec3( 0.0,  1.0,  0.0),	glm::vec2(1.0, 1.0),
			glm::vec3(x0, y1, z0), glm::vec3( 0.0,  1.0,  0.0),	glm::vec2(0.0, 1.0),
			glm::vec3(x1, y1, z1), glm::vec3( 0.0,  1.0,  0.0),	glm::vec2(1.0, 0.0),
			glm::vec3(x1, y0, z0), glm::vec3( 1.0,  0.0,  0.0),	glm::vec2(1.0, 0.0),
			glm::vec3(x1, y1, z0), glm::vec3( 1.0,  0.0,  0.0),	glm::vec2(1.0, 1.0),
			glm::vec3(x1, y0, z1), glm::vec3( 1.0,  0.0,  0.0),	glm::vec2(0.0, 0.0),
			glm::vec3(x1, y0, z1), glm::vec3( 0.0,  0.0,  1.0),	glm::vec2(1.0, 0.0),
			glm::vec3(x1, y1, z1), glm::vec3( 0.0,  0.0,  1.0),	glm::vec2(1.0, 1.0),
			glm::vec3(x0, y0, z1), glm::vec3( 0.0,  0.0,  1.0),	glm::vec2(0.0, 0.0),
			glm::vec3(x0, y0, z0), glm::vec3(-1.0,  0.0,  0.0),	glm::vec2(0.0, 0.0),
			glm::vec3(x0, y0, z1), glm::vec3(-1.0,  0.0,  0.0),	glm::vec2(1.0, 0.0),
			glm::vec3(x0, y1, z0), glm::vec3(-1.0,  0.0,  0.0),	glm::vec2(0.0, 1.0),
			glm::vec3(x1, y1, z0), glm::vec3( 0.0,  0.0, -1.0),	glm::vec2(0.0, 1.0),
			glm::vec3(x1, y0, z0), glm::vec3( 0.0,  0.0, -1.0),	glm::vec2(0.0, 0.0),
			glm::vec3(x0, y1, z0), glm::vec3( 0.0,  0.0, -1.0),	glm::vec2(1.0, 1.0)
		};

		uint32_t indices[] = 
		{ 
			 0,  1,  2,  3,  4,  5, 
			 6,  7,  8,  9, 10, 11, 
			12, 13, 14, 15, 16, 17, 
			18, 19, 20, 21, 22, 23, 
			24, 25, 26, 27, 28, 29, 
			30, 31, 32, 33, 34, 35 
		};

		Ref<MeshShape> mesh = createMeshShape(vertexData, sizeof(vertexData), indices, sizeof(indices) / sizeof(uint32_t), _mesh_attrib_flags);

		AABB aabb = { { x0, y0, z0 }, { x1, y1, z1 } };
		mesh->setAABB(aabb);

		return mesh;
	}
	
	//-----------------------------------------------------------------------------------
	Ref<MeshShape> MeshCreator::createShapeQuad(const glm::vec3& _center, float _side, uint32_t _mesh_attrib_flags)
	{
		float x0 = _center.x - _side;
		float x1 = _center.x + _side;
		float y0 = _center.y - _side;
		float y1 = _center.y + _side;
		float z0 = _center.z;

		// remove unusable flags
		uint32_t flags = _mesh_attrib_flags;
		flags = flags & ~(MESH_ATTRIB_TANGENT | MESH_ATTRIB_BITANGENT | MESH_ATTRIB_COLOR);

		//
		vertex_data vertexData[] =
		{
			glm::vec3(x0, y0, z0), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f),
			glm::vec3(x1, y0, z0), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f),
			glm::vec3(x1, y1, z0), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f),
			glm::vec3(x0, y1, z0), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f),
		};

		uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
	
		Ref<MeshShape> mesh = createMeshShape(vertexData, sizeof(vertexData), indices, 6, _mesh_attrib_flags);
		
		AABB aabb = { { x0, y0, z0 }, { x1, y1, z0 } };
		mesh->setAABB(aabb);

		return mesh;
	}

	//-----------------------------------------------------------------------------------
	Ref<MeshShape> MeshCreator::createShapeViewportQuad()
	{
		uint32_t flags = MESH_ATTRIB_POSITION | MESH_ATTRIB_UV;
		
		vertex_data vertexData[] =
		{
			glm::vec3(-1.0f, -1.0f,  0.0f), glm::vec3(0.0), glm::vec2(0.0f, 0.0f),
			glm::vec3( 1.0f, -1.0f,  0.0f), glm::vec3(0.0), glm::vec2(1.0f, 0.0f),
			glm::vec3( 1.0f,  1.0f,  0.0f), glm::vec3(0.0), glm::vec2(1.0f, 1.0f),
			glm::vec3(-1.0f,  1.0f,  0.0f), glm::vec3(0.0), glm::vec2(0.0f, 1.0f),
		};

		uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

		return createMeshShape(vertexData, sizeof(vertexData), indices, 6, flags);
	}

	//-----------------------------------------------------------------------------------
	Ref<MeshShape> MeshCreator::createShapeViewportQuadFraction(const glm::vec2& _lo, 
																const glm::vec2& _hi)
	{
		uint32_t flags = MESH_ATTRIB_POSITION | MESH_ATTRIB_UV;

		vertex_data vertexData[] = 
		{
			glm::vec3(_lo.x, _lo.y,  0.0f), glm::vec3(0.0), glm::vec2(0.0f, 0.0f),
			glm::vec3(_hi.x, _lo.y,  0.0f), glm::vec3(0.0), glm::vec2(1.0f, 0.0f),
			glm::vec3(_hi.x, _hi.y,  0.0f), glm::vec3(0.0), glm::vec2(1.0f, 1.0f),
			glm::vec3(_lo.x, _hi.y,  0.0f), glm::vec3(0.0), glm::vec2(0.0f, 1.0f),
		};

		uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

		return createMeshShape(vertexData, sizeof(vertexData), indices, 6, flags);
	}
	
	//-----------------------------------------------------------------------------------
	Ref<MeshShape> MeshCreator::createShapeSphere(const glm::vec3& _center, float _radius, 
												  uint32_t _stack_count, uint32_t _sector_count, 
												  uint32_t _mesh_attrib_flags)
	{
		// remove unsupported vertex attributes (for now)
		// TODO: implement these
		uint32_t flags = _mesh_attrib_flags;
		flags = flags & ~(MESH_ATTRIB_TANGENT | MESH_ATTRIB_BITANGENT | MESH_ATTRIB_COLOR);

		// create vertices
		//
		std::vector<vertex_data> vertices;
		float x, y, z, xy;
		float sectorAngle, stackAngle;
		float lenInv = 1.0f / _radius;

		float sectorStep = 2 * M_PI / (float)_sector_count;
		float stackStep = M_PI / (float)_stack_count;

		for (uint32_t i = 0; i <= _stack_count; i++)
		{
			// from pi/2 to -pi/2
			stackAngle = M_PI / 2.0f - (float)i * stackStep;
			// r * cos(u)
			xy = _radius * cosf(stackAngle);
			// r * sin(u)
			z = _radius * sinf(stackAngle);

			for (uint32_t j = 0; j <= _sector_count; j++)
			{
				vertex_data vertex;

				// position
				sectorAngle = j * sectorStep;
				// r * cos(u) * cos(v)
				x = xy * cosf(sectorAngle);
				// r * cos(u) * sin(v)
				y = xy * sinf(sectorAngle);
				vertex.position = glm::vec3(x, y, z);

				// normal
				vertex.normal = vertex.position * lenInv;

				// UV coordinates
				vertex.uv = glm::vec2((float)i / _sector_count, (float)j / _stack_count);

				// store vertex
				vertices.push_back(vertex);
			}
		}

		// create indices
		//
		std::vector<uint32_t> indices;
		uint32_t k1, k2;

		for (uint32_t i = 0; i < _stack_count; i++)
		{
			k1 = i * (_sector_count + 1);
			k2 = k1 + _sector_count + 1;

			for (uint32_t j = 0; j < _sector_count; j++, k1++, k2++)
			{
				if (i != 0)
				{
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				if (i != (_stack_count - 1))
				{
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		}


		// return ref
		Ref<MeshShape> mesh = createMeshShape(vertices.data(), vertices.size() * sizeof(vertex_data), indices.data(), indices.size(), flags);

		// set position of sphere through the model matrix
		Transform t;
		t.translate(_center);
		mesh->setTransform(t);

		// set bounding-box
		AABB aabb = {
			{ _center - glm::vec3(_radius) }, 
			{ _center + glm::vec3(_radius) }
		};
		mesh->setAABB(aabb);

		return mesh;
	}

	//-----------------------------------------------------------------------------------
	Ref<MeshShape> MeshCreator::createShapeMeshgrid(float* _y,
													uint32_t _y_size,
													const Linspace<float>& _x,
													const Linspace<float>& _z,
											  		uint32_t _mesh_attrib_flags,
													uint32_t _normal_flag)
	{
		#ifdef DEBUG_MESH
			Timer timer("", false);
		#endif

		// check validity of data
		if (_y == nullptr || _y_size == 0)
		{
			SYN_CORE_WARNING("supplied data invalid.");
			createDebugCube(glm::vec3(0.0f), 0.5f, "error_ShapeMeshgrid");
			return createMeshShape(nullptr, 0, nullptr, 0, MESH_ATTRIB_NONE);
		}

		uint32_t ny = _y_size;
		uint32_t nx = _x.size();
		uint32_t nz = _z.size();

		int vertexCount = nx * nz;
		vertex_data* vertices = new vertex_data[vertexCount];

		int indexCount = (nx - 1) * (nz - 1) * 6;
		uint32_t* indices = new uint32_t[indexCount];
		
		#ifdef DEBUG_MESH
			SYN_CORE_TRACE("allocating ", sizeof(vertex_data) * vertexCount, " bytes (vertices).");
			SYN_CORE_TRACE("allocating ", sizeof(uint32_t) * indexCount, " bytes (indices).");
		#endif

		// find y range
		limit_2D_t<float> ylim = { std::numeric_limits<float>::max(), 
								   std::numeric_limits<float>::min() };
		for (int i = 0; i < ny; i++)
		{
			float y = _y[i];
			ylim.min_ = min(ylim.min_, y);
			ylim.max_ = max(ylim.max_, y);
		}

		// normalized to [0.0 .. 100.0] for all axes -- later adjusted to 
		float xrange_i = 1.0f / _x.range();
		float yrange_i = 1.0f / (ylim.max_ - ylim.min_);
		float zrange_i = 1.0f / _z.range();

		float xmin = _x.get(0);
		float ymin = ylim.min_;
		float zmin = _z.get(0);

		float* x = _x.getValues();
		float* z = _z.getValues();

		for (uint32_t i = 0; i < nz; i++)
		{
			for (uint32_t j = 0; j < nx; j++)
			{
				int index = i * nx + j;
				
				vertices[index].position = {
					(static_cast<float>(x[j]) - xmin) * xrange_i,
					(static_cast<float>(_y[index]) - ymin) * yrange_i,
					(static_cast<float>(z[i]) - zmin) * zrange_i
				};
			}
		}

		// set indices
		for (uint32_t j = 0; j < nz - 1; j++)
		{
			for (uint32_t i = 0; i < nx - 1; i++)
			{
				int index = 6 * (j * (nz - 1) + i);
				// upper left triangle
				indices[index + 0] = j * nx + i;
				indices[index + 1] = (j + 1) * nx + i;
				indices[index + 2] = j * nx + i + 1;
				// lower right triangle
				indices[index + 3] = j * nx + i + 1;
				indices[index + 4] = (j + 1) * nx + i;
				indices[index + 5] = (j + 1) * nx + i + 1;
			}
		}
		
		// compute grid normals
		for (uint32_t i = 0; i < nz; i++)
		{
			for (uint32_t j = 0; j < nx; j++)
			{
				int k = i * nx + j;

				int L = i * nx + j - 1;
				int R = i * nx + j + 1;
				int U = (i - 1) * nx + j;
				int D = (i + 1) * nx + j;

				glm::vec3 sum;

				// normals 'inside' borders
				if (i > 0 && i < (nz-1) && j > 0 && j < (nx - 1))
				{
					if (_normal_flag & MESH_NORMALS_APPROX)
					{
						// L -> P :  [ x - (x-1),  f(x,z) - f(x-1,z),  z - z ] =
						//           [ 1.0, f(x,z) - f(x-1,z), 0.0 ]
						glm::vec3 LP = glm::vec3(x[j]-x[j-1], _y[k]-_y[L], 0.0f);
						// P -> R :  [ (x+1) - x,  f(x+1,z) - f(x,z),  z - z ] =
						//           [ 1.0, f(x+1,z) - f(x,z), 0.0 ]
						glm::vec3 PR = glm::vec3(x[j+1]-x[j], _y[R]-_y[k], 0.0f);
						// U -> P :  [ x - x,  f(x,z) - f(x,z-1),  z - (z-1) ] =
						//           [ 0.0, f(x,z) - f(x,z-1), 1.0 ]
						glm::vec3 UP = glm::vec3(0.0f, _y[k]-_y[U], z[i]-z[i-1]);
						// P -> D :  [ x - x,  f(x,z+1) - f(x,z),  (z+1) - z ] =
						//           [ 0.0, f(x,z+1) - f(x,z), 1.0 ]
						glm::vec3 PD = glm::vec3(0.0f, _y[D]-_y[k], z[i+1]-z[i]);
						
						glm::vec3 UP_LP = glm::cross(UP, LP);
						glm::vec3 UP_PR = glm::cross(UP, PR);
						glm::vec3 PD_LP = glm::cross(PD, LP);
						glm::vec3 PD_PR = glm::cross(PD, PR);

						sum = UP_LP + UP_PR + PD_LP + PD_PR;
					}
					else if (_normal_flag & MESH_NORMALS_APPROX_FAST)
					{
						// * Simpler method, only calculating the vectors from U to D, and from L to R, 
						// * respectively, and using their normalized cross product as the derivative,
						// * which is faster.
						//
						// U -> D :  [ x - x,  f(x, z+1) - f(x, z-1),  (z+1) - (z-1) ] =
						//           [ 0.0, f(x, z+1) - f(x, z-1), 2.0 ]
						glm::vec3 UD = glm::vec3(0.0, _y[D]-_y[U], z[i+1]-z[i-1]);
						// L -> R :  [ (x+1) - (x-1),  f(x+1, z) - f(x-1, z),  z - z ] =
						//           [ 2.0, f(x+1, z) - f(x-1, z), 0.0 ]
						glm::vec3 LR = glm::vec3(x[j+1]-x[j-1], _y[R]-_y[L], 0.0);
						sum = glm::cross(UD, LR);
					}
				}

				// z borders, excluding corners
				else if (i > 0 && i < (nz-1) && (j == 0 || j == (nx-1)))
				{
					glm::vec3 PX;
					if (j == 0) 	PX = { x[1]-x[0], _y[R]-_y[k], 0.0f };
					else			PX = { x[nx-1]-x[nx-2], _y[k]-_y[L], 0.0f };
					
					glm::vec3 UP = glm::vec3(0.0f, _y[k]-_y[U], z[i]-z[i-1]);
					glm::vec3 PD = glm::vec3(0.0f, _y[D]-_y[k], z[i+1]-z[i]);

					glm::vec3 UP_PX = glm::cross(UP, PX);
					glm::vec3 PD_PX = glm::cross(PD, PX);

					sum = UP_PX + PD_PX;
				}

				// x borders, excluding corners
				else if (j > 0 && j < (nx-1) && (i == 0 || i == (nz-1)))
				{
					glm::vec3 PZ;
					if (i == 0)		PZ = glm::vec3(0.0f, _y[D]-_y[k], z[1]-z[0]);
					else 			PZ = glm::vec3(0.0f, _y[k]-_y[U], z[nz-1]-z[nz-2]);

					glm::vec3 PR = glm::vec3(x[j+1]-x[j], _y[R]-_y[k], 0.0f);
					glm::vec3 LP = glm::vec3(x[j]-x[j-1], _y[k]-_y[L], 0.0f);

					glm::vec3 PZ_LP = glm::cross(PZ, LP);					
					glm::vec3 PZ_PR = glm::cross(PZ, PR);

					sum = PZ_LP + PZ_PR;
				}

				// corners
				else if (i == 0 && j == 0)
				{
					glm::vec3 PR = glm::vec3(x[1]-x[0], _y[R]-_y[k], 0.0f);
					glm::vec3 PD = glm::vec3(0.0f, _y[D]-_y[k], z[1]-z[0]);
					sum = glm::cross(PD, PR);
				}
				else if (i == 0 && j == (nx-1))
				{
					glm::vec3 LP = glm::vec3(x[nx-1]-x[nx-2], _y[k]-_y[L], 0.0f);
					glm::vec3 PD = glm::vec3(0.0f, _y[D]-_y[k], z[1]-z[0]);
					sum = glm::cross(PD, LP);
				}
				else if (i == (nz-1) && j == 0)
				{
					glm::vec3 PR = glm::vec3(x[1]-x[0], _y[R]-_y[k], 0.0f);
					glm::vec3 UP = glm::vec3(0.0f, _y[k]-_y[U], z[nz-1]-z[nz-2]);
					sum = glm::cross(UP, PR);
				}
				else if (i == (nz-1) && j == (nx-1))
				{
					glm::vec3 LP = glm::vec3(x[nx-1]-x[nx-2], _y[k]-_y[L], 0.0f);
					glm::vec3 UP = glm::vec3(0.0f, _y[k]-_y[U], z[nz-1]-z[nz-2]);
					sum = glm::cross(UP, LP);
				}

				// set vertex normal
				vertices[k].normal = glm::normalize(sum);
			}
		}

		Ref<MeshShape> mesh = createMeshShape(vertices, 
											  sizeof(vertex_data) * vertexCount, 
											  indices,
											  indexCount, 
											  _mesh_attrib_flags);

		AABB aabb = {
			{ (_x.lim.min_ - _x.lim.min_) * xrange_i, 
			  (  ylim.min_ -   ylim.min_) * yrange_i, 
			  (_z.lim.min_ - _z.lim.min_) * zrange_i },

			{ (_x.lim.max_ - _x.lim.min_) * xrange_i, 
			  (	 ylim.max_ -   ylim.min_) * yrange_i, 
			  (_z.lim.max_ - _z.lim.min_) * zrange_i }
		};
		mesh->setAABB(aabb);

		// Translate to origin
		Transform t;
		t.translate(glm::vec3(-0.5f));
		mesh->setTransform(t);

		// cleanup
		delete[] vertices;
		delete[] indices;

		#ifdef DEBUG_MESH
			SYN_CORE_TRACE("mesh generated in ", timer.getDeltaTimeMs(), " ms.");
		#endif

		return mesh;
	}

	//-----------------------------------------------------------------------------------
	/*
	Ref<MeshShape> MeshCreator::createShapeMeshgridNormals(long double* _y,
														   long double* _y_normals,
														   uint32_t _y_size,
														   const Linspace<long double>& _x,
														   const Linspace<long double>& _z,
														   uint32_t _mesh_attrib_flags)
	{
		#ifdef DEBUG_MESH
			Timer timer("", false);
		#endif


		uint32_t nx = _x.size();
		uint32_t ny = _y_size;
		uint32_t nz = _z.size();

		int vertexCount = nx * nz;
		vertex_data* vertices = new vertex_data[vertexCount];

		int indexCount = (nx - 1) * (nz - 1) * 6;
		uint32_t* indices = new uint32_t[indexCount];

		#ifdef DEBUG_MESH
			SYN_CORE_TRACE("allocating ", sizeof(vertex_data) * vertexCount, " bytes (", vertexCount, " vertices).");
			SYN_CORE_TRACE("allocating ", sizeof(uint32_t) * indexCount, " bytes (", indexCount, " indices).");
		#endif

		// find y range
		limit_2D_t<float> ylim = { std::numeric_limits<float>::max(), 
								   std::numeric_limits<float>::min() };
		for (int i = 0; i < ny; i++)
		{
			float y = _y[i];
			ylim.min_ = std::min(ylim.min_, y);
			ylim.max_ = std::max(ylim.max_, y);
		}

		// normalize to [0.0 .. 1.0] for all axes -- later adjusted to [-0.5 .. 0.5] through 
		// translation after mesh creation
		float xrange_i = 1.0f / _x.range();
		float yrange_i = 1.0f / (ylim.max_ - ylim.min_);
		float zrange_i = 1.0f / _z.range();

		float xmin = _x.get(0);
		float ymin = ylim.min_;
		float zmin = _z.get(0);

		long double* x = _x.getValues();
		long double* z = _z.getValues();

		// set vertices and normals
		for (uint32_t i = 0; i < nz; i++)
		{
			for (uint32_t j = 0; j < nx; j++)
			{
				int index = i * nx + j;
				
				vertices[index].position =
				{
					(static_cast<float>(x[j]) - xmin) * xrange_i,
					(static_cast<float>(_y[index]) - ymin) * yrange_i,
					(static_cast<float>(z[i]) - zmin) * zrange_i
				};

				vertices[index].normal = 
				{
					_y_normals[3 * index + 0],
					_y_normals[3 * index + 1],
					_y_normals[3 * index + 2],
				};
			}
		}

		// set indices
		for (uint32_t j = 0; j < nz - 1; j++)
		{
			for (uint32_t i = 0; i < nx - 1; i++)
			{
				int index = 6 * (j * (nz - 1) + i);
				// upper left triangle
				indices[index + 0] = j * nx + i;
				indices[index + 1] = (j + 1) * nx + i;
				indices[index + 2] = j * nx + i + 1;
				// lower right triangle
				indices[index + 3] = j * nx + i + 1;
				indices[index + 4] = (j + 1) * nx + i;
				indices[index + 5] = (j + 1) * nx + i + 1;
			}
		}
		
		Ref<MeshShape> mesh = createMeshShape(vertices, 
											  sizeof(vertex_data) * vertexCount, 
											  indices,
											  indexCount, 
											  _mesh_attrib_flags);

		AABB aabb = {
			{ (_x.lim.min - _x.lim.min) * xrange_i, 
			  (   ylim.min_ -    ylim.min_) * yrange_i, 
			  (_z.lim.min - _z.lim.min) * zrange_i },

			{ (_x.lim.max - _x.lim.min) * xrange_i, 
			  (	  ylim.max_ -    ylim.min_) * yrange_i, 
			  (_z.lim.max - _z.lim.min) * zrange_i }
		};
		mesh->setAABB(aabb);

		// Translate to origin
		Transform t;
		t.translate(glm::vec3(-0.5f));
		mesh->setTransform(t);

		// cleanup
		delete[] vertices;
		delete[] indices;

		#ifdef DEBUG_MESH
			SYN_CORE_TRACE("mesh generated in ", timer.getDeltaTimeMs(), " ms.");
		#endif

		return mesh;

	}
	*/
	// Debug meshes ---------------------------------------------------------------------
	Ref<MeshDebug> MeshCreator::createDebugCube(const glm::vec3& _center, float _diameter, const std::string& _name)
	{
		// the _center vector will be what is used to translate the model during rendering
		// the vertices are always centered around the origin, with a spacing of _diameter.
		

		float x0 = -_diameter;
		float x1 =  _diameter;
		float y0 = -_diameter;
		float y1 =  _diameter;
		float z0 = -_diameter;
		float z1 =  _diameter;

		glm::vec3 vertices[] =
		{
			glm::vec3(x0, y0, z1),
			glm::vec3(x1, y0, z1),
			glm::vec3(x1, y1, z1),
			glm::vec3(x0, y1, z1),

			glm::vec3(x0, y0, z0),
			glm::vec3(x1, y0, z0),
			glm::vec3(x1, y1, z0),
			glm::vec3(x0, y1, z0),
		};

		uint32_t indices[] =
		{
			0, 1, 2, 2, 3, 0,	// front
			1, 5, 6, 6, 2, 1,	// right
			7, 6, 5, 5, 4, 7,	// back
			4, 0, 3, 3, 7, 4,	// left
			4, 5, 1, 1, 0, 4,	// down
			3, 2, 6, 6, 7, 3	// up
		};

		Ref<MeshDebug> mesh = MeshCreator::createMeshDebug(_name, MeshDebugType::CUBE, (void*)vertices, sizeof(vertices), (void*)indices, sizeof(indices) / sizeof(uint32_t));
		mesh->setPosition(_center);

		AABB aabb;
		aabb.min = glm::vec3(x0, y0, z0);
		aabb.max = glm::vec3(x1, y1, z1);
		mesh->setAABB(aabb);

		return mesh;
	}

	//-----------------------------------------------------------------------------------
	Ref<MeshDebug> MeshCreator::createDebugQuad(const glm::vec3& _center, float _side, const std::string& _name)
	{
		float s2 = _side / 2.0f;
		glm::vec3 vertices[] = 
		{
			glm::vec3(_center.x-s2, _center.y-s2, _center.z ),
			glm::vec3(_center.x+s2, _center.y-s2, _center.z ),
			glm::vec3(_center.x+s2, _center.y+s2, _center.z ),
			glm::vec3(_center.x-s2, _center.y+s2, _center.z ),
		};

		uint32_t indices[] { 0, 1, 2, 2, 3, 0 }; 

		Ref<MeshDebug> mesh = MeshCreator::createMeshDebug(_name, MeshDebugType::QUAD, (void*)vertices, sizeof(vertices), (void*)indices, sizeof(indices) / sizeof(uint32_t));

		AABB aabb;
		aabb.min = aabb.max = _center;
		mesh->setAABB(aabb);
		
		return mesh;
	}

	//-----------------------------------------------------------------------------------
	Ref<MeshDebug> MeshCreator::createDebugSphere(const glm::vec3& _center, float _radius, uint32_t _stack_count, uint32_t _sector_count, const std::string& _name)
	{
		// create vertices

		std::vector<glm::vec3> vertices;
		float x, y, z, xy;
		float sectorAngle, stackAngle;
		
		float sectorStep = 2 * M_PI / (float)_sector_count;
		float stackStep = M_PI / (float)_stack_count;

		for (uint32_t i = 0; i <= _stack_count; i++)
		{
			// from pi/2 to -pi/2
			stackAngle = M_PI / 2.0f - (float)i * stackStep;
			// r * cos(u)
			xy = _radius * cosf(stackAngle);
			// r * sin(u)
			z = _radius * sinf(stackAngle);

			for (uint32_t j = 0; j <= _sector_count; j++)
			{
				sectorAngle = j * sectorStep;
				// r * cos(u) * cos(v)
				x = xy * cosf(sectorAngle);
				// r * cos(u) * sin(v)
				y = xy * sinf(sectorAngle);

				vertices.push_back(glm::vec3(x, y, z));
			}
		}

		
		// create indices

		std::vector<uint32_t> indices;
		uint32_t k1, k2;
		
		for (uint32_t i = 0; i < _stack_count; i++)
		{
			k1 = i * (_sector_count + 1);
			k2 = k1 + _sector_count + 1;

			for (uint32_t j = 0; j < _sector_count; j++, k1++, k2++)
			{
				if (i != 0)
				{
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				if (i != (_stack_count - 1))
				{
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		}


		// return ref
		Ref<MeshDebug> mesh = MeshCreator::createMeshDebug(_name, MeshDebugType::SPHERE, vertices.data(), vertices.size() * sizeof(glm::vec3), indices.data(), indices.size());
		
		// set position of sphere, to be translated by during rendering.
		mesh->setPosition(_center);

		// set bounding-box
		AABB aabb;
		aabb.min = _center - glm::vec3(_radius);
		aabb.max = _center + glm::vec3(_radius);
		mesh->setAABB(aabb);
		
		return mesh;
	}

	//-----------------------------------------------------------------------------------
	Ref<MeshDebug> MeshCreator::createDebugPoints(const std::vector<glm::vec3>& _points, float _size, const std::string& _name)
	{
		// create sequential indices
		std::vector<uint32_t> indices;
		indices.reserve(_points.size());

		float x0, y0, z0, x1, y1, z1;
		x0 = y0 = z0 = std::numeric_limits<float>::max();
		x1 = y1 = z1 = std::numeric_limits<float>::min();

		// step through points
		for (uint32_t i = 0; i < _points.size(); i++)
		{	
			// assign index sequentially
			indices.push_back(i);

			x0 = std::min(x0, _points[i].x);
			x1 = std::max(x1, _points[i].x);
			y0 = std::min(y0, _points[i].y);
			y1 = std::max(y1, _points[i].y);
			z0 = std::min(z0, _points[i].z);
			z1 = std::max(z1, _points[i].z);
		}
		Ref<MeshDebug> mesh = std::dynamic_pointer_cast<MeshDebug>(MeshCreator::createMeshDebug(_name, MeshDebugType::POINTS, (void*)_points.data(), _points.size() * sizeof(glm::vec3), (void*)indices.data(), indices.size()));
		mesh->setPointSize(_size);

		AABB aabb;
		aabb.min = glm::vec3(x0, y0, z0);
		aabb.max = glm::vec3(x1, y1, z1);
		mesh->setAABB(aabb);

		// return ref
		return mesh;
	}

	//-----------------------------------------------------------------------------------
	Ref<MeshDebug> MeshCreator::createDebugLineStrip(const std::vector<glm::vec3>& _line_veritces, const std::string& _name)
	{
		//SYN_CORE_ASSERT(_line_pairs.size() % 2 == 0, "uneven number of vertices.");
		
		std::vector<uint32_t> indices;
		indices.reserve(_line_veritces.size());
		
		float x0, y0, z0, x1, y1, z1;
		x0 = y0 = z0 = std::numeric_limits<float>::max();
		x1 = y1 = z1 = std::numeric_limits<float>::min();

		for (size_t i = 0; i < _line_veritces.size(); i++)
		{
			indices.push_back(i);

			x0 = std::min(x0, _line_veritces[i].x);
			x1 = std::max(x1, _line_veritces[i].x);
			y0 = std::min(y0, _line_veritces[i].y);
			y1 = std::max(y1, _line_veritces[i].y);
			z0 = std::min(z0, _line_veritces[i].z);
			z1 = std::max(z1, _line_veritces[i].z);
			
		}

		Ref<MeshDebug> mesh = MeshCreator::createMeshDebug(_name, MeshDebugType::LINE_STRIP, (void*)_line_veritces.data(), _line_veritces.size() * sizeof(glm::vec3), indices.data(), indices.size());

		AABB aabb;
		aabb.min = glm::vec3(x0, y0, z0);
		aabb.max = glm::vec3(x1, y1, z1);
		mesh->setAABB(aabb);

		return mesh;
	}

	//-----------------------------------------------------------------------------------
	Ref<MeshDebug> MeshCreator::createDebugLines(const std::vector<glm::vec3>& _vertex_pairs, const std::string& _name)
	{
		SYN_CORE_ASSERT(_vertex_pairs.size() % 2 == 0, "uneven number of vertices.");

		std::vector<uint32_t> indices;
		indices.reserve(_vertex_pairs.size());

		float x0, y0, z0, x1, y1, z1;
		x0 = y0 = z0 = std::numeric_limits<float>::max();
		x1 = y1 = z1 = std::numeric_limits<float>::min();

		for (size_t i = 0; i < _vertex_pairs.size(); i++)
		{
			indices.push_back(i);

			x0 = std::min(x0, _vertex_pairs[i].x);
			x1 = std::max(x1, _vertex_pairs[i].x);
			y0 = std::min(y0, _vertex_pairs[i].y);
			y1 = std::max(y1, _vertex_pairs[i].y);
			z0 = std::min(z0, _vertex_pairs[i].z);
			z1 = std::max(z1, _vertex_pairs[i].z);

		}

		Ref<MeshDebug> mesh = MeshCreator::createMeshDebug(_name, MeshDebugType::LINES, (void*)_vertex_pairs.data(), _vertex_pairs.size() * sizeof(glm::vec3), indices.data(), indices.size());

		AABB aabb;
		aabb.min = glm::vec3(x0, y0, z0);
		aabb.max = glm::vec3(x1, y1, z1);
		mesh->setAABB(aabb);

		return mesh;
	}

	//-----------------------------------------------------------------------------------
	Ref<MeshDebug> MeshCreator::createDebugAABB(const AABB& _aabb, const std::string& _name)
	{
		// TODO : coordinates needs to be updated by the mesh transform.
		float x0 = _aabb.min.x;
		float x1 = _aabb.max.x;
		float y0 = _aabb.min.y;
		float y1 = _aabb.max.y;
		float z0 = _aabb.min.z;
		float z1 = _aabb.max.z;

		glm::vec3 vertices[] =
		{
			glm::vec3(x0, y0, z1),
			glm::vec3(x1, y0, z1),
			glm::vec3(x1, y1, z1),
			glm::vec3(x0, y1, z1),

			glm::vec3(x0, y0, z0),
			glm::vec3(x1, y0, z0),
			glm::vec3(x1, y1, z0),
			glm::vec3(x0, y1, z0),
		};

		uint32_t indices[] =
		{
			0, 1, 1, 2, 2, 3, 3, 0,
			4, 5, 5, 6, 6, 7, 7, 4,
			0, 4, 1, 5, 2, 6, 3, 7,
		};

		// return ref
		Ref<MeshDebug> mesh = MeshCreator::createMeshDebug(_name, MeshDebugType::AABB, vertices, sizeof(vertices), indices, sizeof(indices) / sizeof(uint32_t));
		mesh->setAABB(_aabb);

		return mesh;

	}

	//-----------------------------------------------------------------------------------
	Ref<MeshDebug> MeshCreator::createDebugPlane(const glm::vec3& _center, uint32_t _side, const std::string& _name)
	{
		// enforce origin at (0, 0, 0)
		int side = (_side % 2 == 0) ? (_side + 1) : _side;
		
		glm::vec3 vmin = _center - glm::vec3((float)side);
		glm::vec3 vmax = _center + glm::vec3((float)side);

		// loop through number of vertices
		std::vector<glm::vec3> vertices;	vertices.reserve(2 * side * 4);
		std::vector<uint32_t> indices;		indices.reserve(2 * side * 4);

		uint32_t k = 0;
		for (int i = -side; i < side; i++)
		{
			glm::vec3 vx0 = _center;	vx0.z = i;	vx0.x = vmin.x;
			glm::vec3 vx1 = _center;	vx1.z = i;	vx1.x = vmax.x;
			vertices.push_back(vx0);	vertices.push_back(vx1);
			indices.push_back(k++);		indices.push_back(k++);
			glm::vec3 vz0 = _center;	vz0.x = i;	vz0.z = vmin.z;
			glm::vec3 vz1 = _center;	vz1.x = i;	vz1.z = vmax.z;
			vertices.push_back(vz0);	vertices.push_back(vz1);
			indices.push_back(k++);		indices.push_back(k++);
		}

		uint32_t vertSz = 2 * side * 4 * sizeof(glm::vec3);
		
		// return ref
		Ref<MeshDebug> mesh = MeshCreator::createMeshDebug(_name, MeshDebugType::GRID_PLANE, (void*)vertices.data(), vertSz, (void*)indices.data(), indices.size());

		AABB aabb;
		aabb.min = vmin;
		aabb.max = vmax;
		mesh->setAABB(aabb);

		return mesh;
	}

	//-----------------------------------------------------------------------------------
	Ref<MeshDebug> MeshCreator::createDebugPlaneFS(const glm::vec3& _center, uint32_t _side, const std::string& _name)
	{
		float s2 = _side / 2.0f;
		struct vdata { glm::vec3 p; glm::vec2 uv; };
		vdata vertices[] = 
		{
			glm::vec3(_center.x-s2, _center.y, _center.z-s2), glm::vec2(0.0f, 0.0f),
			glm::vec3(_center.x+s2, _center.y, _center.z-s2), glm::vec2(1.0f, 0.0f),
			glm::vec3(_center.x+s2, _center.y, _center.z+s2), glm::vec2(1.0f, 1.0f),
			glm::vec3(_center.x-s2, _center.y, _center.z+s2), glm::vec2(0.0f, 1.0f),
		};

		// reverse winding order due to xz plane
		uint32_t indices[] { 0, 3, 2, 2, 1, 0 }; 

		Ref<MeshDebug> mesh = MeshCreator::createMeshDebug(_name, MeshDebugType::GRID_PLANE_FS, (void*)vertices, sizeof(vertices), (void*)indices, sizeof(indices) / sizeof(uint32_t));

		// set grid size
		mesh->setGridSize(_side);

		AABB aabb;
		aabb.min = aabb.max = _center;
		mesh->setAABB(aabb);
		
		return mesh;
	}

	//-----------------------------------------------------------------------------------
	Ref<MeshDebug> MeshCreator::getMeshDebugPtr(const std::string& _name)
	{
		if (s_MeshDebugSearchMap.find(_name) != s_MeshDebugSearchMap.end())
			return std::dynamic_pointer_cast<MeshDebug>(s_MeshDebugSearchMap[_name]);
		
		// key not found.
		// TODO: return something meaningful that won't crash?
		SYN_CORE_FATAL_ERROR("no mesh '", _name, "'.");
		return nullptr;
	}
	
	//-----------------------------------------------------------------------------------
	Ref<MeshDebug> MeshCreator::getMeshPtr(const std::string& _name)
	{
		if (s_MeshDebugSearchMap.find(_name) != s_MeshDebugSearchMap.end())
			return s_MeshDebugSearchMap[_name];

		// key not found
		SYN_CORE_FATAL_ERROR(false, "no mesh '", _name, "'.");
		return nullptr;
	}

	//-----------------------------------------------------------------------------------
	void MeshCreator::renderDebugMeshes(const Ref<Camera>& _camera_ptr, bool _wireframe)
	{
		if (!s_MeshDebugMap.size())
		{
			if (TimeStep::getFrameCount() % 1000 == 0)
			{
				SYN_CORE_WARNING("trying to render empty debug mesh list.");
			}
			return;
		}

		// store VP matrix
		glm::mat4 viewProjMatrix = _camera_ptr->getViewProjectionMatrix();
		Ref<MeshDebug> mesh;
		Ref<Shader> shader;
		MeshDebugType prevType;

		// initial conditions
		auto item = s_MeshDebugMap.begin();
		mesh = item->second;
		prevType = item->first;

		switch (prevType)
		{
		case MeshDebugType::POINTS:			shader = m_debugPointShader;	break;
		case MeshDebugType::GRID_PLANE_FS:	shader = m_debugGridShader;		break;
		default:							shader = m_debugGeneralShader;
		}
		//shader = (prevType == MeshDebugType::POINTS ? ShaderLibrary::get("static_debug_point_shader") : ShaderLibrary::get("static_debug_general_shader"));

		if (_wireframe)
		{
			Renderer::enableWireFrame();
			Renderer::disableCulling();
		}

		// render all objects
		for (auto& item : s_MeshDebugMap)
		{
			// re-cast as a MeshDebug ref
			mesh = item.second;
			MeshDebugType type = item.first;
			
			// has the type changed? if so, get a new shader
			if (prevType != type)
			{
				// enable different shader depending on type of mesh
				switch (type)
				{
				case MeshDebugType::POINTS:			shader = m_debugPointShader;	break;
				case MeshDebugType::GRID_PLANE_FS:	shader = m_debugGridShader;		break;
				default:							shader = m_debugGeneralShader;
				}
			}

			shader->enable();

			// type-specific uniform
			if (type == MeshDebugType::POINTS)
				shader->setUniform1f("u_point_size", mesh->getPointSize());
			else if (type == MeshDebugType::GRID_PLANE_FS)
			{
				if (_wireframe)
					Renderer::disableWireFrame();
				shader->setUniform1f("u_grid_size", mesh->getGridSize());
			}

			// shader-agnostic uniform (u_model_matrix is set in MeshDebug::render() ).
			shader->setMatrix4fv("u_view_projection_matrix", viewProjMatrix);
			shader->setUniform3fv("u_color", s_debugRenderColor);

			// render (threaded in MeshDebug::render() )
			mesh->render(shader);

			// reset wireframe status in case of GRID_PLANE
			if (type == MeshDebugType::GRID_PLANE_FS && _wireframe)
				Renderer::enableWireFrame();

			// update previous type
			prevType = type;

		} // end for every mesh

		if (_wireframe)
		{
			Renderer::disableWireFrame();
			Renderer::enableCulling();
		}
	}

	//-----------------------------------------------------------------------------------
	void MeshCreator::renderDebugMesh(const Ref<MeshDebug>& _mesh_ptr, const Ref<Camera>& _camera_ptr, bool _wireframe)
	{
		glm::mat4 vp = _camera_ptr->getViewProjectionMatrix();
		Ref<MeshDebug> mesh = _mesh_ptr;
		Ref<Shader> shader;
		MeshDebugType type = mesh->getType();

		if (type == MeshDebugType::POINTS)
			shader = ShaderLibrary::get("static_debug_point_shader");
		else
			shader = ShaderLibrary::get("static_debug_general_shader");

		if (_wireframe)
		{
			Renderer::enableWireFrame();
			Renderer::disableCulling();
		}

		shader->enable();
		if (type == MeshDebugType::POINTS)
			shader->setUniform1f("u_point_size", mesh->getPointSize());

		shader->setMatrix4fv("u_view_projection_matrix", vp);
		shader->setUniform3fv("u_color", s_debugRenderColor);

		mesh->render(shader);

		if (_wireframe)
		{
			Renderer::disableWireFrame();
			Renderer::enableCulling();
		}
	}

	//-----------------------------------------------------------------------------------
	void MeshCreator::clearDebugMeshes()
	{
		for (auto item : s_MeshDebugMap)
			item.second.reset();
	}
		
	//-----------------------------------------------------------------------------------
	void MeshCreator::createDebugShaders()
	{
		std::string srcPoint = R"(
			#type VERTEX_SHADER
			#version 330 core
			layout(location = 0) in vec3 a_position;
			uniform mat4 u_view_projection_matrix = mat4(1.0f);
			uniform mat4 u_model_matrix = mat4(1.0f);
			uniform float u_point_size = 10.0f;
			void main()
			{
				gl_Position = u_view_projection_matrix * u_model_matrix * vec4(a_position, 1.0f);
				gl_PointSize = u_point_size / (gl_Position.z * 0.1f);
			}

			#type FRAGMENT_SHADER
			#version 330 core
			layout(location = 0) out vec4 out_color;
			uniform vec3 u_color = vec3(1.0f, 0.0f, 1.0f);
			void main()
			{
				out_color = vec4(u_color, 1.0f);
			}
		)";

		std::string srcGeneral = R"(
			#type VERTEX_SHADER
			#version 330 core
			layout(location = 0) in vec3 a_position;
			uniform mat4 u_view_projection_matrix = mat4(1.0f);
			uniform mat4 u_model_matrix = mat4(1.0f);
			void main()
			{
				gl_Position = u_view_projection_matrix * u_model_matrix * vec4(a_position, 1.0f);
			}

			#type FRAGMENT_SHADER
			#version 330 core
			layout(location = 0) out vec4 out_color;
			uniform vec3 u_color = vec3(1.0f, 0.0f, 1.0f);
			void main()
			{
				out_color = vec4(u_color, 1.0f);
			}
		)";

		std::string srcGridPlaneFS = R"(
			#type VERTEX_SHADER
			#version 330 core
			layout(location=0) in vec3 a_position;
			layout(location=4) in vec2 a_uv;
			uniform mat4 u_view_projection_matrix = mat4(1.0);
			uniform mat4 u_model_matrix = mat4(1.0);
			out vec2 v_uv;
			void main()
			{
				v_uv = a_uv;
				gl_Position = u_view_projection_matrix * u_model_matrix * vec4(a_position, 1.0);
			}
			#type FRAGMENT_SHADER
			#version 330 core
			in vec2 v_uv;
			uniform vec3 u_color;
			uniform float u_grid_size;
			out vec4 frag_color;
			void main()
			{
				vec2 uv = v_uv - 0.5;
				float inv_w = (100.0 * u_grid_size) / (u_grid_size * 2.0);
				uv = fract(uv * u_grid_size);
				float grid = max(1.0 - abs((uv.y - 0.5) * inv_w), 1.0 - abs((uv.x - 0.5) * inv_w));
				frag_color = vec4(vec3(1) * grid, 1.0);
			}
		)";

		FileIOHandler::write_buffer_to_file("./static_debug_point_shader.glsl", srcPoint);
		FileIOHandler::write_buffer_to_file("./static_debug_general_shader.glsl", srcGeneral);
		FileIOHandler::write_buffer_to_file("./static_debug_grid_plane_fs_shader.glsl", srcGridPlaneFS);

		ShaderLibrary::load("./static_debug_point_shader.glsl");
		ShaderLibrary::load("./static_debug_general_shader.glsl");
		ShaderLibrary::load("./static_debug_grid_plane_fs_shader.glsl");
		m_debugPointShader = ShaderLibrary::get("static_debug_point_shader");
		m_debugGeneralShader = ShaderLibrary::get("static_debug_general_shader");
		m_debugGridShader = ShaderLibrary::get("static_debug_grid_plane_fs_shader");

		s_shadersCreated = true;
	}

}