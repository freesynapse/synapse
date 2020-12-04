
#include "pch.hpp"

#include <limits>

#include "Synapse/Renderer/MeshCreator.hpp"

#include "Synapse/Core.hpp"
#include "Synapse/Renderer/Mesh/MeshDebug.hpp"
#include "Synapse/Renderer/Shader/ShaderLibrary.hpp"
#include "Synapse/Utils/FileIOHandler.hpp"
#include "Synapse/Utils/Timer/TimeStep.hpp"


namespace Syn {


	// static declarations
	std::multimap<MeshDebugType, Ref<MeshDebug>> MeshCreator::s_MeshDebugMap;
	boost::unordered_map<std::string, Ref<MeshDebug>> MeshCreator::s_MeshDebugSearchMap;
	bool MeshCreator::s_shadersCreated = false;
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
	Ref<MeshShape> MeshCreator::createMeshShape(void* _vertices, uint32_t _vertices_size_bytes, void* _indices, uint32_t _index_count, uint32_t _mesh_attrib_flags)
	{
		// create mesh from vertex/index data
		Ref<MeshShape> mesh = MakeRef<MeshShape>();
		mesh->setVertexArray(createVertexArray(_vertices, _vertices_size_bytes, _indices, _index_count, _mesh_attrib_flags));
		return mesh;
	}


	//-----------------------------------------------------------------------------------
	Ref<MeshDebug> MeshCreator::createMeshDebug(
		const std::string& _mesh_name,
		MeshDebugType _type, 
		void* _vertices, 
		uint32_t _vertices_size_bytes, 
		void* _indices, 
		uint32_t _index_count
	)
	{
		if (!s_shadersCreated)
			MeshCreator::createShaders();

		// create mesh from vertex/index data
		Ref<MeshDebug> mesh = MakeRef<MeshDebug>(_type);
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
		

		//#ifdef DEBUG_MESH
		SYN_CORE_TRACE("created '", meshName, "' : ", _vertices_size_bytes / sizeof(glm::vec3), " vertices, ", _index_count, " indices.");
		//#endif

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

		struct vdata
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
		};

		vdata vertexData[] =
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

		return createMeshShape(vertexData, sizeof(vertexData), indices, sizeof(indices) / sizeof(uint32_t), _mesh_attrib_flags);
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
		struct vdata
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
		};
		vdata vertexData[] =
		{
			glm::vec3(x0, y0, z0), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f),
			glm::vec3(x1, y0, z0), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f),
			glm::vec3(x1, y1, z0), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f),
			glm::vec3(x0, y1, z0), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f),
		};

		uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

		return createMeshShape(vertexData, sizeof(vertexData), indices, 6, flags);
	}


	//-----------------------------------------------------------------------------------
	Ref<MeshShape> MeshCreator::createShapeViewportQuad()
	{
		uint32_t flags = MESH_ATTRIB_POSITION | MESH_ATTRIB_UV;
		
		struct vdata
		{
			glm::vec3 position;
			glm::vec2 uv;
		};
		vdata vertexData[] =
		{
			glm::vec3(-1.0f, -1.0f,  0.0f), glm::vec2(0.0f, 0.0f),
			glm::vec3( 1.0f, -1.0f,  0.0f), glm::vec2(1.0f, 0.0f),
			glm::vec3( 1.0f,  1.0f,  0.0f), glm::vec2(1.0f, 1.0f),
			glm::vec3(-1.0f,  1.0f,  0.0f), glm::vec2(0.0f, 1.0f),
		};

		uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

		return createMeshShape(vertexData, sizeof(vertexData), indices, 6, flags);
	}


	//-----------------------------------------------------------------------------------
	Ref<MeshShape> MeshCreator::createShapeSphere(const glm::vec3& _center, float _radius, uint32_t _stack_count, uint32_t _sector_count, uint32_t _mesh_attrib_flags)
	{
		// remove unsupported vertex attributes (for now)
		// TODO: implement these
		uint32_t flags = _mesh_attrib_flags;
		flags = flags & ~(MESH_ATTRIB_TANGENT | MESH_ATTRIB_BITANGENT | MESH_ATTRIB_COLOR);


		// create vertices
		//
		struct vdata
		{
			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec2 uv;
		};

		std::vector<vdata> vertices;
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
				vdata vertex;

				// position
				sectorAngle = j * sectorStep;
				// r * cos(u) * cos(v)
				x = xy * cosf(sectorAngle);
				// r * cos(u) * sin(v)
				y = xy * sinf(sectorAngle);
				vertex.pos = glm::vec3(x, y, z);

				// normal
				vertex.normal = vertex.pos * lenInv;

				// UV coordinates
				vertex.uv = glm::vec2((float)i / _sector_count, (float)j / _stack_count);

				// store vertex
				vertices.push_back(vertex);
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
		Ref<MeshShape> mesh = MeshCreator::createMeshShape(vertices.data(), vertices.size() * sizeof(glm::vec3), indices.data(), indices.size(), flags);

		// set position of sphere through the model matrix
		Transform t;
		t.translate(_center);
		mesh->setTransform(t);

		// set bounding-box
		AABB aabb;
		aabb.min = _center - glm::vec3(_radius);
		aabb.max = _center + glm::vec3(_radius);
		mesh->setAABB(aabb);

		return mesh;
	}


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
		shader = (prevType == MeshDebugType::POINTS ? ShaderLibrary::get("static_debug_point_shader") : ShaderLibrary::get("static_debug_general_shader"));

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
				// enable different shader depending on type of mesh (POINTS or not)
				shader = (type == MeshDebugType::POINTS ? ShaderLibrary::get("static_debug_point_shader") : ShaderLibrary::get("static_debug_general_shader"));
			}

			shader->enable();

			// POINT-specific uniform
			if (type == MeshDebugType::POINTS)
				shader->setUniform1f("u_point_size", mesh->getPointSize());

			// shader-agnostic uniform (u_model_matrix set in MeshDebug::render() ).
			shader->setMatrix4fv("u_view_projection_matrix", viewProjMatrix);

			// render (threaded in MeshDebug::render() )
			mesh->render(shader);

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
	void MeshCreator::createShaders()
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
			uniform vec3 u_color = vec3(1.0f, 0.0f, 0.0f);
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
			uniform vec3 u_color = vec3(1.0f, 0.0f, 0.0f);
			void main()
			{
				out_color = vec4(u_color, 1.0f);
			}
		)";

		FileIOHandler::write_buffer_to_file("./static_debug_point_shader.glsl", srcPoint);
		FileIOHandler::write_buffer_to_file("./static_debug_general_shader.glsl", srcGeneral);

		Ref<Shader> pointShader = MakeRef<Shader>("./static_debug_point_shader.glsl");
		Ref<Shader> generalShader = MakeRef<Shader>("./static_debug_general_shader.glsl");

		ShaderLibrary::add(pointShader);
		ShaderLibrary::add(generalShader);

		s_shadersCreated = true;
	}


}