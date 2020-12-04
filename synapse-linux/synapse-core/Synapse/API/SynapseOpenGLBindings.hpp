
#pragma once 

#include "Synapse/Core.hpp"

#include "Synapse/Renderer/Buffers/Framebuffer.hpp"
#include "Synapse/Renderer/Buffers/IndexBuffer.hpp"
#include "Synapse/Renderer/Buffers/VertexArray.hpp"
#include "Synapse/Renderer/Buffers/VertexBuffer.hpp"

#include "Synapse/Renderer/Camera/OrthographicCamera.hpp"
#include "Synapse/Renderer/Camera/PerspectiveCamera.hpp"

#include "Synapse/Renderer/Font/Font.hpp"

#include "Synapse/Renderer/Material/Texture2D.hpp"

#include "Synapse/Renderer/Mesh/MeshAssimp.hpp"
#include "Synapse/Renderer/Mesh/MeshDebug.hpp"

#include "Synapse/Renderer/Shader/Shader.hpp"


namespace Syn {

	class API
	{
	public:
		// buffers
		//
		static Ref<Framebuffer> newFramebuffer(uint32_t _width, uint32_t _height, ColorFormat _format=ColorFormat::RGBA8, bool _update_on_resize=true)
		{	return MakeRef<Framebuffer>(_width, _height, _format, _update_on_resize);	}
		static Ref<IndexBuffer> newIndexBuffer(GLenum _primitive=GL_TRIANGLES, GLenum _usage=GL_STATIC_DRAW)
		{	return MakeRef<IndexBuffer>(_primitive, _usage);	}
		static Ref<VertexBuffer> newVertexBuffer(GLenum _usage=GL_STATIC_DRAW)
		{	return MakeRef<VertexBuffer>(_usage);	}
		static Ref<VertexArray> newVertexArray(const Ref<VertexBuffer>& _vertex_buffer_ref, const Ref<IndexBuffer>& _index_buffer_ref)
		{	return MakeRef<VertexArray>(_vertex_buffer_ref, _index_buffer_ref);	}
		static Ref<VertexArray> newVertexArray(const Ref<VertexBuffer>& _vertex_buffer_ref)
		{	return MakeRef<VertexArray>(_vertex_buffer_ref);	}


		// cameras
		//
		static Ref<OrthographicCamera> newOrthographicCamera(float _aspect_ratio, float _zoom_level=1.0f)
		{	return MakeRef<OrthographicCamera>(_aspect_ratio, _zoom_level);	}
		static Ref<PerspectiveCamera> newPerspectiveCamera(const glm::mat4& _projection_matrix)
		{	return MakeRef<PerspectiveCamera>(_projection_matrix);	}

		
		// font
		//
		static Ref<Font> newFont(const char *_filename, const int& _pixel_size=12, const Ref<Shader>& _shader=nullptr)
		{	return MakeRef<Font>(_filename, _pixel_size, _shader);	}


		// textures
		//
		static Ref<Texture2D> newTexture2D(const std::string& _asset_path)
		{	return MakeRef<Texture2D>(_asset_path);	}
		static Ref<Texture2D> newTexture2D(uint32_t _width, uint32_t _height, ColorFormat _color_format=ColorFormat::RGBA8)
		{	return MakeRef<Texture2D>(_width, _height, _color_format);	}


		// meshes
		//
		static Ref<MeshAssimp> newMeshAssimp(const std::string& _file_path, uint32_t _mesh_load_flags=MESH_TRANSFORM_NONE, const Transform& _mesh_load_transform=Transform())
		{	return MakeRef<MeshAssimp>(_file_path, _mesh_load_flags, _mesh_load_transform);	}
		static Ref<MeshDebug> newMeshDebug(MeshDebugType _type=MeshDebugType::NONE)
		{	return MakeRef<MeshDebug>(_type);	}


		// shaders
		//
		static Ref<Shader> newShader(const std::string& _shader_file_path)
		{	return MakeRef<Shader>(_shader_file_path);	}
		static Ref<Shader> newShader(const std::string& _shader_name, const std::string& _file_path)
		{	return MakeRef<Shader>(_shader_name, _file_path);	}


	};

}







