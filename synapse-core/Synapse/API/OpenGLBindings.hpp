
#pragma once 


#include "Synapse/Core.hpp"

#include "Synapse/Renderer/Buffers/Framebuffer.hpp"
#include "Synapse/Renderer/Buffers/IndexBuffer.hpp"
#include "Synapse/Renderer/Buffers/VertexArray.hpp"
#include "Synapse/Renderer/Buffers/VertexBuffer.hpp"
#include "Synapse/Renderer/Camera/OrthographicCamera.hpp"
#include "Synapse/Renderer/Camera/PerspectiveCamera.hpp"
#include "Synapse/Renderer/Camera/OrbitCamera.hpp"
#include "Synapse/Renderer/Font/Font.hpp"
#include "Synapse/Renderer/Material/Texture2D.hpp"
#include "Synapse/Renderer/Mesh/MeshAssimp.hpp"
#include "Synapse/Renderer/Mesh/MeshDebug.hpp"
#include "Synapse/Renderer/Shader/Shader.hpp"
#include "Synapse/Types/Linspace.hpp"


namespace Syn {

	namespace API {

		/* Framebuffer parameters:
		 * 	Syn::ColorFormat _format : Format of the COLOR_ATTACHMENT used.
		 *	glm::ivec2 _size		 : Size in px. If not specified, Syn::Renderer::viewport is used.
		 *  size_t n_draw_buffers	 : Number of GL_COLOR_ATTACHMENT targets.
		 *  bool _use_depthbuffer	 : Flag for creation of an attached depthbuffer.
		 *	bool _update_on_resize	 : Controls response to Syn::ViewportResizeEvent:s.
		 *  std::string _name		 : Name ID (used for debugging).
		 */		
		static inline Ref<Framebuffer> newFramebuffer(ColorFormat _format=ColorFormat::RGBA16F, 
													  const glm::ivec2& _size=glm::ivec2(0), 
													  size_t _n_drawbuffers=1, 
													  bool _use_depthbuffer=true, 
													  bool _update_on_resize=true, 
													  const std::string& _name="")
		{	return MakeRef<Framebuffer>(_format, _size, _n_drawbuffers, _use_depthbuffer, _update_on_resize, _name); }

		static inline Ref<IndexBuffer> newIndexBuffer(GLenum _primitive, GLenum _usage)
		{	return MakeRef<IndexBuffer>(_primitive, _usage);	}
		static inline Ref<VertexBuffer> newVertexBuffer(GLenum _usage=GL_STATIC_DRAW)
		{	return MakeRef<VertexBuffer>(_usage);	}
		static inline Ref<VertexArray> newVertexArray(const Ref<VertexBuffer>& _vertex_buffer_ref, const Ref<IndexBuffer>& _index_buffer_ref)
		{	return MakeRef<VertexArray>(_vertex_buffer_ref, _index_buffer_ref);	}
		static inline Ref<VertexArray> newVertexArray(const Ref<VertexBuffer>& _vertex_buffer_ref)
		{	return MakeRef<VertexArray>(_vertex_buffer_ref);	}


		// cameras
		//
		static inline Ref<OrthographicCamera> newOrthographicCamera(float _aspect_ratio, float _zoom_level=10.0f)
		{	return MakeRef<OrthographicCamera>(_aspect_ratio, _zoom_level);	}
		//static inline Ref<PerspectiveCamera> newPerspectiveCamera(const glm::mat4& _projection_matrix)
		//{	return MakeRef<PerspectiveCamera>(_projection_matrix);	}
		static inline Ref<PerspectiveCamera> newPerspectiveCamera(float _fov_deg, float _screen_w, float _screen_h, float _z_near, float _z_far)
		{	return MakeRef<PerspectiveCamera>(_fov_deg, _screen_w, _screen_h, _z_near, _z_far); }
		static inline Ref<OrbitCamera> newOrbitCamera(float _fov_deg, float _screen_w, float _screen_h, float _z_near, float _z_far)
		{	return MakeRef<OrbitCamera>(_fov_deg, _screen_w, _screen_h, _z_near, _z_far);	}

		
		// font
		//
		static inline Ref<Font> newFont(const char *_filename, const int& _pixel_size=12, const Ref<Shader>& _shader=nullptr)
		{	return MakeRef<Font>(_filename, _pixel_size, _shader);	}


		// textures
		//
		static inline Ref<Texture2D> newTexture2D(const ::std::string& _asset_path)
		{	return MakeRef<Texture2D>(_asset_path);	}
		static inline Ref<Texture2D> newTexture2D(uint32_t _width, uint32_t _height, ColorFormat _color_format=ColorFormat::RGBA8)
		{	return MakeRef<Texture2D>(_width, _height, _color_format);	}


		// meshes
		//
		static inline Ref<MeshAssimp> newMeshAssimp(const ::std::string& _file_path, uint32_t _mesh_load_flags, const Transform& _mesh_load_transform=Transform())
		{	return MakeRef<MeshAssimp>(_file_path, _mesh_load_flags, _mesh_load_transform);	}
		static inline Ref<MeshDebug> newMeshDebug(MeshDebugType _type=MeshDebugType::NONE)
		{	return MakeRef<MeshDebug>(_type);	}


		// shaders
		//
		static inline Ref<Shader> newShader(const ::std::string& _shader_file_path)
		{	return MakeRef<Shader>(_shader_file_path);	}
		static inline Ref<Shader> newShader(const ::std::string& _shader_name, const ::std::string& _file_path)
		{	return MakeRef<Shader>(_shader_name, _file_path);	}


		// Macros forcing inlining
		//

		// buffers
		//
		// Ref<Framebuffer> newFramebuffer(uint32_t _width, uint32_t _height, ColorFormat _format=Syn::ColorFormat::RGBA8, bool _update_on_resize=true)
		//#define SYN_NEW_FRAMBUFFER_REF(_width, _height, _color_format_COLORFORMAT_RGBA8, _update_on_resize_TRUE) \
		//		SYN_MAKE_REF(Syn::Framebuffer, _width, _height, _color_format_COLORFORMAT_RGBA8, _update_on_resize_TRUE)
		//// Ref<IndexBuffer> newIndexBuffer(GLenum _primitive=GL_TRIANGLES, GLenum _usage=GL_STATIC_DRAW)
		//#define SYN_NEW_INDEX_BUFFER_REF(_primitive_GL_TRIANGLES, _usage_GL_STATIC_DRAW) \
		//		SYN_MAKE_REF(Syn::IndexBuffer, _primitive_GL_TRIANGLES, _usage_GL_STATIC_DRAW)
		//// Ref<VertexBuffer> newVertexBuffer(GLenum _usage=GL_STATIC_DRAW)
		//#define SYN_NEW_VERTEX_BUFFER_REF(_usage_GL_STATIC_DRAW) \
		//		SYN_MAKE_REF(Syn::VertexBuffer, _usage_GL_STATIC_DRAW)
		//// Ref<VertexArray> newVertexArray(const Ref<VertexBuffer>& _vertex_buffer_ref, const Ref<IndexBuffer>& _index_buffer_ref)
		//#define SYN_NEW_VERTEX_ARRAY_VBO_IBO(_vertex_buffer_ref, _index_buffer_ref) \
		//		SYN_MAKE_REF(Syn::VertexArray, _vertex_buffer_ref, _index_buffer_ref)
		//// Ref<VertexArray> newVertexArray(const Ref<VertexBuffer>& _vertex_buffer_ref)
		//#define SYN_NEW_VERTEX_ARRAY_VBO(_vertex_buffer_ref) \
		//		SYN_MAKE_REF(Syn::VertexArray, _vertex_buffer_ref)

		// cameras
		//
		// Ref<OrthographicCamera> newOrthographicCamera(float _aspect_ratio, float _zoom_level=1.0f)
		//#define SYN_NEW_ORTHOGRAPHIC_CAMERA_REF(_aspect_ratio, _zoom_level_1_0F) \
		//		SYN_MAKE_REF(Syn::OrthographicCamera, _aspect_ratio, _zoom_level_1_0F)
		//// Ref<PerspectiveCamera> newPerspectiveCamera(const glm::mat4& _projection_matrix)
		//#define SYN_NEW_PERSPECTIVE_CAMERA_REF(_projection_matrix_mat4) \
		//		SYN_MAKE_REF(Syn::PerspectiveCamera, _projection_matrix_mat4)

		// fonts
		//
		// Ref<Font> newFont(const char *_filename, const int& _pixel_size=12, const Ref<Shader>& _shader=nullptr)
		//#define SYN_NEW_FONT_REF(_filename, _pixel_size_12_0F, _shader_ref_NULLPTR) \
		//		SYN_MAKE_REF(Syn::Font, _filename, _pixel_size_12_0F, _shader_ref_NULLPTR)

		// textures
		//
		// Ref<Texture2D> newTexture2D(const std::string& _asset_path)
		//#define SYN_NEW_TEXTURE2D_REF_ASSET_PATH(_asset_path) \
		//		SYN_MAKE_REF(Syn::Texture2D, _asset_path)
		//// Ref<Texture2D> newTexture2D(uint32_t _width, uint32_t _height, ColorFormat _color_format=Syn::ColorFormat::RGBA8)
		//#define SYN_NEW_TEXTURE2D_REF_W_H_FORMAT(_width, _height, _color_format_SYN_COLORFORMAT_RGBA8) \
		//		SYN_MAKE_REF(Syn::Texture2D, _width, _height, _color_format_SYN_COLORFORMAT_RGBA8)

		// meshes
		//
		// Ref<MeshAssimp> newMeshAssimp(const std::string& _file_path, uint32_t _mesh_load_flags, const Transform& _mesh_load_transform=Syn::Transform())
		//#define SYN_NEW_MESH_ASSIMP_REF(_file_path, _mesh_load_flags, _mesh_load_transform_SYN_TRANSFORM) \
		//		SYN_MAKE_REF(Syn::MeshAssimp, _file_path, _mesh_load_flags, _mesh_load_transform_SYN_TRANSFORM)
		//// Ref<MeshDebug> newMeshDebug(MeshDebugType _type=Syn::MeshDebugType::NONE)
		//#define SYN_NEW_MESH_DEBUG_REF(_type_SYN_MESHDEBUGTYPE_NONE) \
		//		SYN_MAKE_REF(Syn::MeshDebug, _type_SYN_MESHDEBUGTYPE_NONE)

		// shaders
		//
		// Ref<Shader> newShader(const std::string& _shader_file_path)
		//#define SYN_NEW_SHADER_REF_FILE_PATH(_shader_file_path) \
		//		SYN_MAKE_REF(Syn::Shader, _shader_file_path)
		//// Ref<Shader> newShader(const std::string& _shader_name, const std::string& _file_path)
		//#define SYN_NEW_SHADER_REF_NAME_FILE_PATH(_shader_name, _file_path) \
		//		SYN_MAKE_REF(Syn::Shader, _shader_name, _file_path)

	} // namespace API

} // namespace Syn

