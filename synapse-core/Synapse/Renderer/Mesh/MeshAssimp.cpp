
#include "pch.hpp"

#pragma warning(disable: 26495)
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#include "Synapse/Renderer/Mesh/MeshAssimp.hpp"
#include "Synapse/Debug/Log.hpp"
#include "Synapse/Types.hpp"


namespace Syn {


	static const uint32_t s_meshImportFlags =
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split meshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_GenUVCoords |             // Convert UVs if required 
		aiProcess_OptimizeMeshes |          // Batch draws where possible
		aiProcess_ValidateDataStructure;    // Validation


	//-----------------------------------------------------------------------------------
	struct LogStream : public Assimp::LogStream
	{
		static void Initialize()
		{
			if (Assimp::DefaultLogger::isNullLogger())
			{
				Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
				Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
			}
		}

		virtual void write(const char* _message) override
		{
			SYN_CORE_ERROR("assimp: ", _message);
		}
	};
	

	//-----------------------------------------------------------------------------------
	MeshAssimp::MeshAssimp(const std::string& _file_path, uint32_t _mesh_load_flags, const Transform& _mesh_load_transform)
	{
		m_assetPath = _file_path;
		m_loadFlags = _mesh_load_flags;

		LogStream::Initialize();

		SYN_CORE_TRACE("loading mesh '", m_assetPath, "'.");

		m_importer = std::make_unique<Assimp::Importer>();

		const aiScene* scene = m_importer->ReadFile(m_assetPath, s_meshImportFlags);
		if (!scene)
		{
			SYN_CORE_FATAL_ERROR("scene=nullptr, incorrect file name entered?");
		}

		#ifdef DEBUG_MESH
			SYN_CORE_TRACE("submesh count = ", scene->mNumMeshes);
			SYN_CORE_TRACE("has animations = ", scene->HasAnimations() ? "TRUE" : "FALSE");
		#endif

		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;

		m_submeshes.reserve(scene->mNumMeshes);
		for (size_t m = 0; m < scene->mNumMeshes; m++)
		{
			aiMesh* mesh = scene->mMeshes[m];

			Submesh submesh;
			submesh.baseVertex = vertexCount;
			submesh.baseIndex = indexCount;
			submesh.materialIndex = mesh->mMaterialIndex;
			submesh.indexCount = mesh->mNumFaces * 3;
			m_submeshes.push_back(submesh);
			#ifdef DEBUG_MESH
				SYN_CORE_TRACE("submesh ", m, ":");
				SYN_CORE_TRACE("\tsubmesh.baseVertex = ", submesh.baseVertex);
				SYN_CORE_TRACE("\tsubmesh.baseIndex = ", submesh.baseIndex);
				SYN_CORE_TRACE("\tsubmesh.materialIndex = ", submesh.materialIndex);
				SYN_CORE_TRACE("\tsubmesh.indexCount = ", submesh.indexCount);
				//if (mesh->mMaterialIndex >= 0)
				//{
				//	aiMaterial* mat = scene->mMaterials[m];
				//	aiString str;
				//	uint32_t n = mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
				//	SYN_CORE_TRACE("\tmaterial path = ", str.C_Str());
				//}
			#endif

			vertexCount += mesh->mNumVertices;
			indexCount += submesh.indexCount;

			// vertices
			for (size_t i = 0; i < mesh->mNumVertices; i++)
			{
				VertexBase vertex;
				vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

				// update AABB
				m_aabb.min.x = MIN(m_aabb.min.x, vertex.position.x);
				m_aabb.max.x = MAX(m_aabb.max.x, vertex.position.x);
				m_aabb.min.y = MIN(m_aabb.min.y, vertex.position.y);
				m_aabb.max.y = MAX(m_aabb.max.y, vertex.position.y);
				m_aabb.min.z = MIN(m_aabb.min.z, vertex.position.z);
				m_aabb.max.z = MAX(m_aabb.max.z, vertex.position.z);

				vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

				if (mesh->HasTangentsAndBitangents())
				{
					#ifdef DEBUG_MESH
						if (m == 0 && i == 0)
						{
							SYN_CORE_TRACE("extracting tangents/bitangents.");
						}
					#endif
					vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
					vertex.bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
				}

				if (mesh->HasTextureCoords(0))
				{
					#ifdef DEBUG_MESH
						if (m == 0 && i == 0)
						{
							SYN_CORE_TRACE("extracting UV coordinates.");
						}
					#endif
					vertex.uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
				}

				m_vertices.push_back(vertex);

			}

			// indices
			for (size_t i = 0; i < mesh->mNumFaces; i++)
			{
				SYN_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3, "face must have 3 indices.");
				m_indices.push_back({ mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] });
			}
		}

		// post-load flags
		std::cout << "";
		if (m_loadFlags != MESH_TRANSFORM_NONE)
		{
			// centering
			if (m_loadFlags & MESH_TRANSFORM_CENTER_X || m_loadFlags & MESH_TRANSFORM_CENTER_Y || m_loadFlags & MESH_TRANSFORM_CENTER_Z)

			{
				float x_disp, y_disp, z_disp;
				(m_loadFlags & MESH_TRANSFORM_CENTER_X) ? x_disp = -(m_aabb.max.x + m_aabb.min.x) / 2.0f : x_disp = 0.0f;
				(m_loadFlags & MESH_TRANSFORM_CENTER_Y) ? y_disp = -(m_aabb.max.y + m_aabb.min.y) / 2.0f : y_disp = 0.0f;
				(m_loadFlags & MESH_TRANSFORM_CENTER_Z) ? z_disp = -(m_aabb.max.z + m_aabb.min.z) / 2.0f : z_disp = 0.0f;
				glm::vec3 disp(x_disp, y_disp, z_disp);
				#ifdef DEBUG_MESH
					SYN_CORE_TRACE("displacement vector [ ", disp.x, "  ", disp.y, "  ", disp.z, " ]");
				#endif
				// update vertices
				for (VertexBase& v : m_vertices)
					v.position += disp;
				// adjust aabb
				m_aabb.min += disp;
				m_aabb.max += disp;
			}

			// transform
			if (m_loadFlags & MESH_TRANSFORM_MATRIX)
			{
				#ifdef DEBUG_MESH
					if (m_loadFlags & MESH_TRANSFORM_MATRIX)
					{
						SYN_CORE_TRACE("mesh transform requested");
						Log::debug_matrix("", "", _mesh_load_transform.getModelMatrix());
					}
				#endif

				// update vertices
				glm::mat4 mat = _mesh_load_transform.getModelMatrix();
				for (VertexBase& v : m_vertices)
				{
					glm::vec4 pos = glm::vec4(v.position, 1.0f);
					pos = mat * pos;
					v.position = glm::vec3(pos.x, pos.y, pos.z);
				}
				// transform aabb
				glm::vec4 min = glm::vec4(m_aabb.min, 1.0f);
				glm::vec4 max = glm::vec4(m_aabb.max, 1.0f);
				min = mat * min;
				max = mat * max;
				m_aabb.min = glm::vec3(min.x, min.y, min.z);
				m_aabb.max = glm::vec3(max.x, max.y, max.z);
			}
		}

		// setup VAO, VBO, IBO
		Ref<VertexBuffer> vertexBuffer = MakeRef<VertexBuffer>();
		vertexBuffer->setBufferLayout({
			{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_color" },
			{ VERTEX_ATTRIB_LOCATION_NORMAL, ShaderDataType::Float3, "a_normal" },
			{ VERTEX_ATTRIB_LOCATION_TANGENT, ShaderDataType::Float3, "a_tangent" },
			{ VERTEX_ATTRIB_LOCATION_BITANGENT, ShaderDataType::Float3, "a_bitangent" },
			{ VERTEX_ATTRIB_LOCATION_UV, ShaderDataType::Float2, "a_uv" }
		});
		vertexBuffer->setData(m_vertices.data(), sizeof(VertexBase) * m_vertices.size());
		#ifdef DEBUG_MESH
			SYN_CORE_TRACE("vertex buffer size = ", sizeof(VertexBase) * m_vertices.size(), " bytes.");
		#endif

		Ref<IndexBuffer> indexBuffer = MakeRef<IndexBuffer>();
		indexBuffer->setData(m_indices.data(), m_indices.size() * 3);
		#ifdef DEBUG_MESH
			SYN_CORE_TRACE("index buffer size = ", sizeof(Index) * m_indices.size(), " bytes.");
		#endif

		m_vertexArray = MakeRef<VertexArray>(vertexBuffer, indexBuffer);

		// store scene pointer
		m_scene = scene;

		SYN_CORE_TRACE(m_vertices.size(), " vertices and ", m_indices.size(), " indices loaded.");

	}


	//-----------------------------------------------------------------------------------
	void MeshAssimp::render(const Ref<Shader>& _shader_ptr)
	{
		_shader_ptr->setMatrix4fv("u_modelMatrix", m_transform.getModelMatrix());
		m_vertexArray->bind();
		//Renderer::drawIndexed(m_vertexArray->getNumIndices());
		SYN_RENDER_S0({
			for (Submesh& submesh : self->m_submeshes)
			{
				glDrawElementsBaseVertex(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, (void*)(sizeof(uint32_t) * submesh.baseIndex), submesh.baseVertex);
			}
		});
	}


	//-----------------------------------------------------------------------------------
	void MeshAssimp::printVertices(uint32_t _mesh_attrib_flags)
	{
		SYN_CORE_TRACE("model '", m_assetPath, "' vertex data (n=", m_vertices.size(), "):");
		for (auto& vertex : m_vertices)
		{
			std::printf("glm::vec3(%.1f, %.1f, %.1f), ", vertex.position.x, vertex.position.y, vertex.position.z);
			std::printf("glm::vec3(%.1f, %.1f, %.1f), ", vertex.normal.x, vertex.normal.y, vertex.normal.z);
			std::printf("glm::vec2(%.1f, %.1f), ", vertex.uv.x, vertex.uv.y);
			std::printf("\n");
		}
		std::printf("\n");

		for (auto& index : m_indices)
		{
			std::printf("%d, %d, %d, ", index.i0, index.i1, index.i2);
		}
		std::printf("\n");
	}


}