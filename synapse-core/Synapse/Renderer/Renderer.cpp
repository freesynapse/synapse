

#include "pch.hpp"

#include <string>

#include "Synapse/Renderer/Renderer.hpp"

#include "Synapse/Debug/Error.hpp"
#include "Synapse/Renderer/Mesh/Mesh.hpp"
#include "Synapse/Utils/FileIOHandler.hpp"
#include "Synapse/Renderer/Shader/ShaderLibrary.hpp"
#include "Synapse/Event/EventHandler.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"


namespace Syn {


	// declaration of static members
	Ref<Camera> Renderer::s_camera 				= nullptr;
	glm::mat4 Renderer::s_viewProjectionMatrix 	= glm::mat4(1.0f);
	Renderer* Renderer::s_instance  			= nullptr;
	glm::ivec2 Renderer::s_viewport 			= glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT);

	glm::ivec2 Renderer::s_imGuiViewportPos		= glm::ivec2(0);
	glm::ivec2 Renderer::s_imGuiDockPos			= glm::ivec2(0);
	glm::ivec2 Renderer::s_imGuiWinPos			= glm::ivec2(0);
	glm::ivec2 Renderer::s_imGuiViewportOffset	= glm::ivec2(0);
	std::string Renderer::s_imGuiRendererName 	= "synapse-core::renderer";

	glm::vec4 Renderer::s_clearColor			= glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	Ref<Shader> Renderer::s_normalShader 		= nullptr;
	Ref<Shader> Renderer::s_tangentShader 		= nullptr;
	Ref<Shader> Renderer::s_bitangentShader 	= nullptr;
	bool Renderer::s_hasNormalShader 			= false;
	bool Renderer::s_hasTangentShader 			= false;
	bool Renderer::s_hasBitangentShader 		= false;


	//-----------------------------------------------------------------------------------
	static void GLAPIENTRY openGLLogMessage(GLenum _src, GLenum _type, GLuint _id, GLenum _severity, GLsizei _len, const GLchar* _msg, const void* _params)
	{
		if (_severity != GL_DEBUG_SEVERITY_NOTIFICATION)
		{
			SYN_CORE_FATAL_ERROR(_msg);
			return;
		}
		SYN_CORE_TRACE(_msg);
	}


	//-----------------------------------------------------------------------------------
	void Renderer::initOpenGL()
	{
		// debug
		#ifdef DEBUG_OPENGL_API
			glDebugMessageCallback(openGLLogMessage, nullptr);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		#endif

		// rendering and culling
		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// depth testing
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		
		// set depth buffer to read-only
		//glDepthMask(GL_FALSE);
		
		// GL_LINE ans GL_POINT
		glLineWidth(1.0f);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, -1.0f);
		glEnable(GL_PROGRAM_POINT_SIZE);

		// textures
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//
		auto& caps = Renderer::getCapabilities();
		caps.vendor = (const char*)glGetString(GL_VENDOR);
		caps.renderer = (const char*)glGetString(GL_RENDERER);
		caps.version = (const char*)glGetString(GL_VERSION);

		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &caps.maxTextureUnits);
		glGetIntegerv(GL_MAX_SAMPLES, &caps.maxSamples);
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &caps.maxAnisotropy);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			SYN_CORE_ERROR("OpenGL error: ", error);
			glGetError();
		}

		// setup core normal debug shader
		#ifdef DEBUG_MESH_NORMAL_SHADER
			s_hasNormalShader = true;
		#endif

		#ifdef DEBUG_MESH_TANGENT_SHADER
			s_hasTangentShader = true;
		#endif

		#ifdef DEBUG_MESH_BITANGENT_SHADER
			s_hasBitangentShader = true;
		#endif

		// register function to receive viewport resize events
		EventHandler::register_callback(EventType::VIEWPORT_RESIZE, SYN_EVENT_STATIC_FNC(Renderer::onResizeEvent));

		// initialize and register debug shaders
		setupDebugShaders();
	}


	//-----------------------------------------------------------------------------------
	void Renderer::initImGui()
	{
		// find the viewport specified in the apps ImGui.ini (if present)
		std::ifstream fs("./imgui.ini", std::ios::in);
		if (!fs)
		{
			SYN_CORE_WARNING("no ImGui configuration file found.");
			return;
		}

		// read the preset viewport for the renderer in config file
		// under 
		// [Window][synapse-core::renderer]
		// Pos=0,22
		// Size=1528,1031
		// Collapsed=0
		// DockId=0x00000001,0
		//
		// Both Pos and Size are needed:
		// the viewport size = (size.x - pos.x, size.y - pos.y).
		//

		fs.seekg(0);
		std::string section = "[Window][" + s_imGuiRendererName + "]";
		std::string line;
		std::string pos, size;
		glm::ivec2 vpos, vsize;
		while (std::getline(fs, line))
		{
			// compare for correct section
			if (section.compare(0, section.size(), line) == 0)
			{
				// find Pos and Size and get values into vectors
				//
				std::getline(fs, pos);
				pos = pos.substr(pos.find('=')+1);
				vpos.x = std::stoi(pos.substr(0, pos.find(',')));
				vpos.y = std::stoi(pos.substr(pos.find(',')+1));

				//
				std::getline(fs, size);
				size = size.substr(size.find('=')+1);
				vsize.x = std::stoi(size.substr(0, size.find(',')));
				vsize.y = std::stoi(size.substr(size.find(',')+1));

				break;
			}
		}

		fs.seekg(0);
		section = "[Docking][Data]";
		std::string dockLine;
		glm::ivec2 dockPos(0);
		while (std::getline(fs, line))
		{
			if (section.compare(0, section.size(), line) == 0)
			{
				std::getline(fs, dockLine);
				dockLine = dockLine.substr(dockLine.find("Pos=")+4);
				dockPos.x = std::stoi(dockLine.substr(0, dockLine.find(',')));
				dockPos.y = std::stoi(dockLine.substr(dockLine.find(',')+1));
			}
		}

		// set all the things we found!
		s_viewport.x = vsize.x - vpos.x;
		s_viewport.y = vsize.y - vpos.y;
		s_imGuiViewportPos = vpos;

		s_imGuiDockPos = dockPos;

		SYN_CORE_TRACE("ImGui docker pos (", s_imGuiDockPos.x, ", ", s_imGuiDockPos.y, ")");
		SYN_CORE_TRACE("ImGui viewport [ ", s_viewport.x, ", ", s_viewport.y, " ]");
		SYN_CORE_TRACE("ImGui viewport pos (", s_imGuiViewportPos.x, ", ", s_imGuiViewportPos.y, ")");

		setImGuiWindowPosition(glm::ivec2(0, 0));

	}


	//-----------------------------------------------------------------------------------
	void Renderer::onResizeEvent(Event* _e)
	{
		ViewportResizeEvent* e = dynamic_cast<ViewportResizeEvent*>(_e);

		// store new viewport
		s_viewport = e->getViewport();
		// set main viewport
		setViewport(glm::ivec2(0, 0), s_viewport);

		//SYN_RENDER_1(s_viewport, {
		//	glViewport(0, 0, s_viewport.x, s_viewport.y);
		//});

	}


	//-----------------------------------------------------------------------------------
	void Renderer::beginScene(Ref<Camera> _camera_ptr)
	{
		if (_camera_ptr == nullptr)
		{
			s_viewProjectionMatrix = glm::mat4(1.0f);
			return;
		}

		s_camera = _camera_ptr;
		s_viewProjectionMatrix = _camera_ptr->getViewProjectionMatrix();

		// check for existing framebuffer for post-effects
		/*
		if (s_sceneFramebuffer != nullptr)
		{
			s_sceneFramebuffer->bind();
			Renderer::enableDepthTesting();
		}
		*/

	}


	//-----------------------------------------------------------------------------------
	void Renderer::endScene()
	{
		// if a post-effect frame buffer is used, render it!
		/*
		if (s_sceneFramebuffer != nullptr)
		{
			// unbind out custom framebuffer -- default framebuffer now active
			s_sceneFramebuffer->bindDefaultFramebuffer();
			
			// avoid textured screen rect being discarded by depth testing
			Renderer::disableDepthTesting();

		}
		*/
	}



	//-----------------------------------------------------------------------------------
	// API calls
	//-----------------------------------------------------------------------------------


	// buffers
	void Renderer::clearColorBuffer()
	{
		SYN_RENDER_0({
			glClear(GL_COLOR_BUFFER_BIT);
		});
	}

	void Renderer::clearDepthBuffer()
	{
		SYN_RENDER_0({
			glClear(GL_DEPTH_BUFFER_BIT);
		});
	}

	void Renderer::clear(uint32_t _bitfield)
	{
		SYN_RENDER_1(_bitfield, {
			glClear(_bitfield);
		});
	}

	void Renderer::setClearColor(float _r, float _g, float _b, float _a)
	{
		s_clearColor = glm::vec4(_r, _g, _b, _a);
		SYN_RENDER_4(_r, _g, _b, _a, {
			glClearColor(_r, _g, _b, _a);
		});
	}

	void Renderer::setClearColor(const glm::vec4& _color)
	{
		s_clearColor = _color;
		SYN_RENDER_1(_color, {
			glClearColor(_color.r, _color.g, _color.b, _color.a);
		});
	}

	//-----------------------------------------------------------------------------------
	// viewport
	void Renderer::setViewport(const glm::ivec2& _position, const glm::ivec2& _size)
	{
		SYN_RENDER_2(_position, _size, {
			glViewport(_position.x, _position.y, _size.x, _size.y);
		});
	}

	void Renderer::resetViewport()
	{
		SYN_RENDER_1(s_viewport, {
			glViewport(0, 0, s_viewport.x, s_viewport.y);
		});
	}



	//-----------------------------------------------------------------------------------
	// blending equation
	void Renderer::setBlendingEq(GLenum _src_factor, GLenum _dest_factor)
	{
		SYN_RENDER_2(_src_factor, _dest_factor, {
			glBlendFunc(_src_factor, _dest_factor);
		});
	}


	//-----------------------------------------------------------------------------------
	// states
	void Renderer::enableWireFrame()
	{
		SYN_RENDER_0({
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		});
	}

	void Renderer::disableWireFrame()
	{
		SYN_RENDER_0({
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		});
	}

	void Renderer::setWireFrame(bool _wireframe)
	{
		SYN_RENDER_1(_wireframe, {
			if (_wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				return;
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		})
	}

	void Renderer::enableDepthTesting()
	{
		SYN_RENDER_0({
			glEnable(GL_DEPTH_TEST);
		});
	}

	void Renderer::disableDepthTesting()
	{
		SYN_RENDER_0({
			glDisable(GL_DEPTH_TEST);
		})
	}

	void Renderer::setDepthTesting(bool _depth_test)
	{
		SYN_RENDER_1(_depth_test, {
			if (_depth_test)
			{
				glEnable(GL_DEPTH_TEST);
				return;
			}
			glDisable(GL_DEPTH_TEST);
		});
	}

	void Renderer::enableDepthMask()
	{
		SYN_RENDER_0({
			glDepthMask(GL_TRUE);
		});
	}

	void Renderer::disableDepthMask()
	{
		SYN_RENDER_0({
			glDepthMask(GL_FALSE);
		});
	}

	void Renderer::setDepthMask(bool _depth_mask)
	{
		SYN_RENDER_1(_depth_mask, {
			if (_depth_mask)
			{
				glDepthMask(GL_TRUE);
				return;
			}
			glDepthMask(GL_FALSE);
		});
	}

	void Renderer::enableCulling()
	{
		SYN_RENDER_0({
			glEnable(GL_CULL_FACE);
		});
	}

	void Renderer::disableCulling()
	{
		SYN_RENDER_0({
			glDisable(GL_CULL_FACE);
		});
	}

	void Renderer::setCulling(bool _cull)
	{
		SYN_RENDER_1(_cull, {
			if (_cull)
			{
				glEnable(GL_CULL_FACE);
				return;
			}
			glDisable(GL_CULL_FACE);
		});
	}

	void Renderer::enableBlending()
	{
		SYN_RENDER_0({
			glEnable(GL_BLEND);
		});
	}

	void Renderer::disableBlending()
	{
		SYN_RENDER_0({
			glDisable(GL_BLEND);
		});
	}

	void Renderer::setBlending(bool _blending)
	{
		SYN_RENDER_1(_blending, {
			if (_blending)
			{
				glEnable(GL_BLEND);
				return;
			}
			glDisable(GL_BLEND);
		});
	}


	void Renderer::enableGLenum(GLenum _gl_enum)
	{
		SYN_RENDER_1(_gl_enum, {
			glEnable(_gl_enum);
		});
	}
	
	void Renderer::disableGLenum(GLenum _gl_enum)
	{
		SYN_RENDER_1(_gl_enum, {
			glDisable(_gl_enum);
		});

	}
	
	void Renderer::setGLenum(GLenum _gl_enum, bool _b)
	{
		SYN_RENDER_2(_gl_enum, _b, {
			if (_b)
			{
				glEnable(_gl_enum);
				return;
			}
			glDisable(_gl_enum);
		});
	}

	//-----------------------------------------------------------------------------------
	// rendering
	//-----------------------------------------------------------------------------------

	//-----------------------------------------------------------------------------------
	// textures
	void Renderer::enableTexture2D(uint32_t _tex_id, uint32_t _tex_slot)
	{
		SYN_RENDER_2(_tex_id, _tex_slot, {
			glActiveTexture(GL_TEXTURE0 + _tex_slot);
			glBindTexture(GL_TEXTURE_2D, _tex_id);
		});
	}

	void Renderer::resetTexture2D(uint32_t _tex_slot)
	{
		SYN_RENDER_1(_tex_slot, {
			glActiveTexture(GL_TEXTURE0 + _tex_slot);
			glBindTexture(GL_TEXTURE_2D, 0);
		});
	}

	//-----------------------------------------------------------------------------------
	// rendering indexed vertex data
	void Renderer::drawIndexed(const Ref<VertexArray>& _vertex_array, bool _depth_test)
	{
		/* 
		Binds vertex array before issuing draw call. 
		*/
		SYN_RENDER_2(_vertex_array, _depth_test, {
			if (!_depth_test)
				glDisable(GL_DEPTH_TEST);
			glBindVertexArray(_vertex_array->getArrayID());
			glDrawElements(_vertex_array->getIndexBuffer()->getPrimitiveType(),
						   _vertex_array->getIndexCount(), 
						   GL_UNSIGNED_INT, 
						   nullptr);
			
			if (!_depth_test)
				glEnable(GL_DEPTH_TEST);
		});
	}

	void Renderer::drawIndexed(uint32_t _index_count, bool _depth_test, GLenum _primitive)
	{
		/* 
		Vertex array has to be bound before calling this. 
		*/
		SYN_RENDER_3(_index_count, _depth_test, _primitive, {
			if (!_depth_test)
				glDisable(GL_DEPTH_TEST);

			glDrawElements(_primitive, _index_count, GL_UNSIGNED_INT, nullptr); 

			if (!_depth_test)
				glEnable(GL_DEPTH_TEST);
		});

	}

	void Renderer::drawIndexedNoDepth(const Ref<VertexArray>& _vertex_array)
	{
		/*
		Binds vertex array before issuing draw call.
		*/
		SYN_RENDER_1(_vertex_array, {
			glBindVertexArray(_vertex_array->getArrayID());
			glDrawElements(_vertex_array->getIndexBuffer()->getPrimitiveType(), 
						   _vertex_array->getIndexCount(), 
						   GL_UNSIGNED_INT, 
						   nullptr);
		});

	}

	void Renderer::drawIndexedNoDepth(uint32_t _index_count, GLenum _primitive)
	{
		/* 
		Vertex array has to be bound before calling this. 
		*/
		SYN_RENDER_2(_index_count, _primitive, {
			glDrawElements(_primitive, _index_count, GL_UNSIGNED_INT, nullptr); 
		});
	}

	void Renderer::drawArrays(const Ref<VertexArray>& _vertex_array, 
							  uint32_t _index_count, 
							  uint32_t _first, 
							  bool _depth_test, 
							  GLenum _primitive)
	{
		/*
		Binds vertex array before issuing draw call.
		*/
		SYN_RENDER_5(_vertex_array, _index_count, _first, _depth_test, _primitive, {
			if (!_depth_test)
				glDisable(GL_DEPTH_TEST);
			
			glBindVertexArray(_vertex_array->getArrayID());
			glDrawArrays(_primitive, _first, _index_count);
			
			if (!_depth_test)
				glEnable(GL_DEPTH_TEST);
		});

	}

	void Renderer::drawArrays(uint32_t _index_count, uint32_t _first, bool _depth_test, GLenum _primitive)
	{
		/* 
		Vertex array has to be bound before calling this. 
		*/
		SYN_RENDER_4(_first, _index_count, _depth_test, _primitive, {
			if (!_depth_test)
				glDisable(GL_DEPTH_TEST);

			glDrawArrays(_primitive, _first, _index_count);

			if (!_depth_test)
				glEnable(GL_DEPTH_TEST);
		});
	}

	void Renderer::drawArraysNoDepth(const Ref<VertexArray>& _vertex_array, 
									 uint32_t _index_count, 
									 uint32_t _first, 
									 GLenum _primitive)
	{
		/*
		Binds vertex array before issuing draw call.
		*/
		SYN_RENDER_4(_vertex_array, _index_count, _first, _primitive, {
			glBindVertexArray(_vertex_array->getArrayID());
			glDrawArrays(_primitive, _first, _index_count);
		});
	}

	void Renderer::drawArraysNoDepth(uint32_t _index_count, uint32_t _first, GLenum _primitive)
	{
		/* 
		Vertex array has to be bound before calling this. 
		*/
		SYN_RENDER_3(_index_count, _first, _primitive, {
			glDrawArrays(_primitive, _first, _index_count);
		});

	}

	//-----------------------------------------------------------------------------------
	void Renderer::setLineWidth(float _width)
	{
		SYN_RENDER_1(_width, {
			glLineWidth(_width);
		});
	}


	//-----------------------------------------------------------------------------------
	void Renderer::debugNormals(const Ref<Mesh>& _mesh_ptr, const Ref<Camera>& _camera_ptr, float _length)
	{
		if (!s_hasNormalShader)
			return;

		glm::mat4 viewProjectionMatrix = _camera_ptr->getViewProjectionMatrix();
		glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(_mesh_ptr->getModelMatrix())));

		s_normalShader->enable();
		s_normalShader->setMatrix4fv("u_view_projection_matrix", viewProjectionMatrix);
		s_normalShader->setMatrix4fv("u_model_matrix", _mesh_ptr->getModelMatrix());
		s_normalShader->setMatrix3fv("u_normal_matrix", normalMatrix);
		s_normalShader->setUniform1f("u_length", _length);
		_mesh_ptr->getVertexArray()->bind();
		Syn::Renderer::drawIndexed(_mesh_ptr->getVertexArray()->getIndexCount(), true, GL_POINTS);
	}


	//-----------------------------------------------------------------------------------
	void Renderer::debugNormals(const Ref<VertexArray>& _vao, const Transform& _t, const Ref<Camera>& _camera_ptr, float _length)
	{
		if (!s_hasNormalShader)
			return;

		glm::mat4 viewProjectionMatrix = _camera_ptr->getViewProjectionMatrix();
		glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(_t.getModelMatrix())));

		s_normalShader->enable();
		s_normalShader->setMatrix4fv("u_view_projection_matrix", viewProjectionMatrix);
		s_normalShader->setMatrix4fv("u_model_matrix", _t.getModelMatrix());
		s_normalShader->setMatrix3fv("u_normal_matrix", normalMatrix);
		s_normalShader->setUniform1f("u_length", _length);
		_vao->bind();
		Syn::Renderer::drawIndexed(_vao->getIndexCount(), true, GL_POINTS);
	}


	//-----------------------------------------------------------------------------------
	void Renderer::debugTangents(const Ref<Mesh>& _mesh_ptr, const Ref<Camera>& _camera_ptr, float _length)
	{
		if (!s_hasTangentShader)
			return;

		glm::mat4 viewProjectionMatrix = _camera_ptr->getViewProjectionMatrix();
		glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(_mesh_ptr->getModelMatrix())));

		s_tangentShader->enable();
		s_tangentShader->setMatrix4fv("u_view_projection_matrix", viewProjectionMatrix);
		s_tangentShader->setMatrix4fv("u_model_matrix", _mesh_ptr->getModelMatrix());
		s_tangentShader->setMatrix3fv("u_normal_matrix", normalMatrix);
		s_tangentShader->setUniform1f("u_length", _length);
		_mesh_ptr->getVertexArray()->bind();
		Syn::Renderer::drawIndexed(_mesh_ptr->getVertexArray()->getIndexCount(), true, GL_POINTS);
	}


	//-----------------------------------------------------------------------------------
	void Renderer::debugBitangents(const Ref<Mesh>& _mesh_ptr, const Ref<Camera>& _camera_ptr, float _length)
	{
		if (!s_hasBitangentShader)
			return;

		glm::mat4 viewProjectionMatrix = _camera_ptr->getViewProjectionMatrix();
		glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(_mesh_ptr->getModelMatrix())));

		s_bitangentShader->enable();
		s_bitangentShader->setMatrix4fv("u_view_projection_matrix", viewProjectionMatrix);
		s_bitangentShader->setMatrix4fv("u_model_matrix", _mesh_ptr->getModelMatrix());
		s_bitangentShader->setMatrix3fv("u_normal_matrix", normalMatrix);
		s_bitangentShader->setUniform1f("u_length", _length);
		_mesh_ptr->getVertexArray()->bind();
		Syn::Renderer::drawIndexed(_mesh_ptr->getVertexArray()->getIndexCount(), true, GL_POINTS);
	}

	//-----------------------------------------------------------------------------------
	void Renderer::setupDebugShaders()
	{
		std::string srcNormal, srcTangent, srcBitangent;

		if (s_hasNormalShader)
		{
			srcNormal = R"(
				#type VERTEX_SHADER
				#version 330 core

				layout(location = 0) in vec3 a_position;
				layout(location = 1) in vec3 a_normal;
				out vec3 g_normal;

				void main()
				{
					g_normal = a_normal;
					gl_Position = vec4(a_position, 1.0f);
				}

				#type GEOMETRY_SHADER
				#version 330 core

				layout(points) in;
				layout(line_strip, max_vertices = 2) out;
				in vec3 g_normal[];
				out vec3 f_color;

				uniform mat4 u_view_projection_matrix = mat4(1.0f);
				uniform mat4 u_model_matrix = mat4(1.0f);
				uniform mat3 u_normal_matrix = mat3(1.0f);
				uniform float u_length = 1.0f;

				void main()
				{
					vec3 normal = g_normal[0];
					f_color = abs(vec4(mat4(u_normal_matrix) * vec4(g_normal[0], 0.0)).xyz);

					vec4 v0 = gl_in[0].gl_Position;
					gl_Position = u_view_projection_matrix * u_model_matrix * v0;
					EmitVertex();

					vec4 v1 = v0 + vec4(normal * u_length, 0.0f);
					gl_Position = u_view_projection_matrix * u_model_matrix * v1;
					EmitVertex();

					EndPrimitive();
				}

				#type FRAGMENT_SHADER
				#version 330 core

				layout(location = 0) out vec4 out_color;
				in vec3 f_color;

				void main()
				{
					out_color = vec4(f_color, 1.0f);
				}
			)";

			FileIOHandler::write_buffer_to_file("./static_debug_normal_shader.glsl", srcNormal);
			s_normalShader = MakeRef<Shader>("./static_debug_normal_shader.glsl");
			ShaderLibrary::add(s_normalShader);
		}
		
		if (s_hasTangentShader)
		{
			srcTangent = R"(
				#type VERTEX_SHADER
				#version 330 core

				layout(location = 0) in vec3 a_position;
				layout(location = 2) in vec3 a_tangent;
				out vec3 g_tangent;

				void main()
				{
					g_tangent = a_tangent;
					gl_Position = vec4(a_position, 1.0f);
				}

				#type GEOMETRY_SHADER
				#version 330 core

				layout(points) in;
				layout(line_strip, max_vertices = 2) out;
				in vec3 g_tangent[];
				out vec3 f_color;

				uniform mat4 u_view_projection_matrix = mat4(1.0f);
				uniform mat4 u_model_matrix = mat4(1.0f);
				uniform mat3 u_normal_matrix = mat3(1.0f);
				uniform float u_length = 1.0f;

				void main()
				{
					vec3 tangent = g_tangent[0];
					f_color = abs(vec4(mat4(u_normal_matrix) * vec4(g_tangent[0], 0.0)).xyz);

					vec4 v0 = gl_in[0].gl_Position;
					gl_Position = u_view_projection_matrix * u_model_matrix * v0;
					EmitVertex();

					vec4 v1 = v0 + vec4(tangent * u_length, 0.0f);
					gl_Position = u_view_projection_matrix * u_model_matrix * v1;
					EmitVertex();

					EndPrimitive();
				}

				#type FRAGMENT_SHADER
				#version 330 core

				layout(location = 0) out vec4 out_color;
				in vec3 f_color;

				void main()
				{
					out_color = vec4(f_color, 1.0f);
				}
			)";

			FileIOHandler::write_buffer_to_file("./static_debug_tangent_shader.glsl", srcTangent);
			s_tangentShader = MakeRef<Shader>("./static_debug_tangent_shader.glsl");
			ShaderLibrary::add(s_tangentShader);
		}

		if (s_hasBitangentShader)
		{
			srcBitangent = R"(
				#type VERTEX_SHADER
				#version 330 core

				layout(location = 0) in vec3 a_position;
				layout(location = 3) in vec3 a_bitangent;
				out vec3 g_bitangent;

				void main()
				{
					g_bitangent = a_bitangent;
					gl_Position = vec4(a_position, 1.0f);
				}

				#type GEOMETRY_SHADER
				#version 330 core

				layout(points) in;
				layout(line_strip, max_vertices = 2) out;
				in vec3 g_bitangent[];
				out vec3 f_color;

				uniform mat4 u_view_projection_matrix = mat4(1.0f);
				uniform mat4 u_model_matrix = mat4(1.0f);
				uniform mat3 u_normal_matrix = mat3(1.0f);
				uniform float u_length = 1.0f;

				void main()
				{
					vec3 bitangent = g_bitangent[0];
					f_color = abs(vec4(mat4(u_normal_matrix) * vec4(g_bitangent[0], 0.0)).xyz);

					vec4 v0 = gl_in[0].gl_Position;
					gl_Position = u_view_projection_matrix * u_model_matrix * v0;
					EmitVertex();

					vec4 v1 = v0 + vec4(bitangent * u_length, 0.0f);
					gl_Position = u_view_projection_matrix * u_model_matrix * v1;
					EmitVertex();

					EndPrimitive();
				}

				#type FRAGMENT_SHADER
				#version 330 core

				layout(location = 0) out vec4 out_color;
				in vec3 f_color;

				void main()
				{
					out_color = vec4(f_color, 1.0f);
				}
			)";

			FileIOHandler::write_buffer_to_file("./static_debug_bitangent_shader.glsl", srcBitangent);
			s_bitangentShader = MakeRef<Shader>("./static_debug_bitangent_shader.glsl");
			ShaderLibrary::add(s_bitangentShader);
		}

		Renderer::get().executeRenderCommands();
	}


}


