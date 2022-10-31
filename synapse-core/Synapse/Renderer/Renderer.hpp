#pragma once


#include <memory>

#include "Synapse/Renderer/RenderCommandQueue.hpp"
#include "Synapse/Renderer/Camera/Camera.hpp"
#include "Synapse/Renderer/Buffers/VertexArray.hpp"
#include "Synapse/Renderer/Transform.hpp"
#include "Synapse/Renderer/Buffers/Framebuffer.hpp"
#include "Synapse/Renderer/Shader/Shader.hpp"
#include "Synapse/Renderer/Material/Texture2D.hpp"
#include "Synapse/Event/EventTypes.hpp"


namespace Syn 
{

	struct RendererAPICapabilities
	{
		std::string vendor;
		std::string renderer;
		std::string version;

		int maxTextureUnits = 0;
		int maxSamples = 0;
		float maxAnisotropy = 0.0f;
	};


	class Mesh;
	class Renderer
	{
	public:
		typedef void(*RenderCommandFn)(void*);

		static void create() { s_instance = new Renderer; }
		static void release() { delete s_instance; }

		static void initOpenGL();
		static void initImGui();
		static void onResizeEvent(Event* _e);
		static RendererAPICapabilities& getCapabilities()
		{
			static RendererAPICapabilities caps;
			return caps;
		}

		static void beginScene(Ref<Camera> _camera_ptr=nullptr);
		static void endScene();

		// 3D mesh debug functions
		static void debugNormals(const Ref<Mesh>& _mesh_ptr, const Ref<Camera>& _camera_ptr, float _length=1.0f);
		static void debugNormals(const Ref<VertexArray>& _vao, const Transform& _t, const Ref<Camera>& _camera_ptr, float _length=1.0f);
		// debugNormalsRaw uses Renderer::drawArrays() for mesh rendering, over Renderer::drawIndexed()
		static void debugNormalsRaw(const Ref<Mesh>& _mesh_ptr, const Ref<Camera>& _camera_ptr, float _length=1.0f);
		static void debugTangents(const Ref<Mesh>& _mesh_ptr, const Ref<Camera>& _camera_ptr, float _length=1.0f);
		static void debugBitangents(const Ref<Mesh>& _mesh_ptr, const Ref<Camera>& _camera_ptr, float _length=1.0f);

		// accessors
		static Renderer& get() { return *s_instance; }
		static glm::mat4& getViewProjectionMatrix() { return s_viewProjectionMatrix; }
		static inline glm::ivec2& getViewport() { return s_viewport; }
		static inline glm::vec2 getViewportF() { return glm::vec2(s_viewport.x, s_viewport.y); }
		static inline glm::ivec2& getViewportPos() { return s_imGuiViewportPos; }
		static inline glm::vec2 getViewportPosF() { return glm::vec2(s_imGuiViewportPos.x, s_imGuiViewportPos.y); }
		static inline glm::ivec2& getImGuiDockingPosition() { return s_imGuiDockPos; }
		static inline glm::vec2 getImGuiDockingPositionF() { return glm::vec2(s_imGuiDockPos.x, s_imGuiDockPos.y); }
		static inline glm::ivec2& getImGuiWindowPosition() { return s_imGuiWinPos; }
		static inline glm::vec2 getImGuiWindowPositionF() { return glm::vec2(s_imGuiWinPos.x, s_imGuiWinPos.y); }
		static inline glm::ivec2& getImGuiViewPortOffset() { return s_imGuiViewportOffset; }
		static inline glm::vec2 getImGuiViewPortOffsetF() { return glm::vec2(s_imGuiViewportOffset.x, s_imGuiViewportOffset.y); }
		static inline float getAspectRatio() { return static_cast<float>(s_viewport.x) / static_cast<float>(s_viewport.y); }
		static const std::string& getImGuiRenderTargetName() { return s_imGuiRendererName; }
		static void setImGuiRenderTargetName(const std::string& _name) { s_imGuiRendererName = _name; }
		static glm::vec4& getClearColor() { return s_clearColor; }
		/* The docking poisition is absolute in px, location on desktop (for this version of Ubuntu [46, 71]),
		 * but mouse position, for instance, is relative to the window. To get the limits of the viewport in 
		 * relative coordinates, use getViewportLimits() or getViewportLimitsF().
		 */
		static void setImGuiWindowPosition(const glm::ivec2& _pos)
		{ 
			s_imGuiWinPos = _pos;
			s_imGuiViewportOffset = s_imGuiDockPos - s_imGuiWinPos + s_imGuiViewportPos;
			if (s_reportImGuiUpdate)
			{
				SYN_CORE_TRACE("ImGui window pos (", s_imGuiWinPos.x, ", ", s_imGuiWinPos.y, ")");
				SYN_CORE_TRACE("ImGui viewport offset (", s_imGuiViewportOffset.x, ", ", s_imGuiViewportOffset.y, ")");
			}
		}
		/* The limits of the viewport as a glm::ivec4, where x and y
		 * are the top and left limits, and z and w are the right 
		 * and bottom limits, respectively.
		 */
		static inline glm::ivec4 getViewportLimits()
		{
			glm::ivec2 vp_lim_lo = glm::ivec2(0, s_imGuiViewportOffset.y);
			glm::ivec2 vp_lim_hi = glm::ivec2(s_viewport.x, s_viewport.y) + vp_lim_lo;
			return glm::ivec4(vp_lim_lo.x, vp_lim_lo.y, vp_lim_hi.x, vp_lim_hi.y);
		}
		/* The limits of the viewport as a glm::vec4, where x and y
		 * are the top and left limits, and z and w are the right 
		 * and bottom limits, respectively.
		 */
		static inline glm::vec4 getViewportLimitsF()
		{
			glm::ivec4 lim = getViewportLimits();
			return glm::vec4(lim.x, lim.y, lim.z, lim.w);
		}
		//
		static void disableImGuiUpdateReport() { s_reportImGuiUpdate = false; }
		static void enableImGuiUpdateReport() { s_reportImGuiUpdate = true; }

		
		// API calls
		//

		// TODO: proper threading
		static void* submitRenderCommand(RenderCommandFn _fnc, unsigned int _size) { return s_instance->m_commandQueue.allocate(_fnc, _size); }
		static void executeRenderCommands() { s_instance->m_commandQueue.execute(); }

		// buffers
		static void clearColorBuffer();
		static void clearDepthBuffer();
		static void clear(uint32_t _bitfield);
		static void setClearColor(float _r, float _g, float _b, float _a);
		static void setClearColor(const glm::vec4& _color);

		// viewport
		static void setViewport(const glm::ivec2& _position, const glm::ivec2& _size);
		static void resetViewport();

		// blending
		static void setBlendingEq(GLenum _src_factor, GLenum _dest_factor);

		// states
		static void enableWireFrame();
		static void disableWireFrame();
		static void setWireFrame(bool _wireframe);

		static void enableDepthTesting();
		static void disableDepthTesting();
		static void setDepthTesting(bool _depth_test);

		static void enableDepthMask();
		static void disableDepthMask();
		static void setDepthMask(bool _depth_mask);

		static void enableCulling();
		static void disableCulling();
		static void setCulling(bool _cull);
	//-----------------------------------------------------------------------------------

		static void enableBlending();
		static void disableBlending();
		static void setBlending(bool _blending);

		static void enableGLenum(GLenum _gl_enum);
		static void disableGLenum(GLenum _gl_enum);
		static void setGLenum(GLenum _gl_enum, bool _b);

		// rendering
		/* activate and bind texture */
		static void enableTexture2D(uint32_t _tex_id, uint32_t _tex_slot=0);
		/* reset texture to 0 */
		static void resetTexture2D(uint32_t _tex_slot=0);

		// rendering of vertex arrays
		//
		static void drawIndexed(const Ref<VertexArray>& _vertex_array, bool _depth_test=true);
		static void drawIndexed(uint32_t _index_count, 
								bool _depth_test=true, 
								GLenum _primitive=GL_TRIANGLES);

		static void drawIndexedNoDepth(const Ref<VertexArray>& _vertex_array);
		static void drawIndexedNoDepth(uint32_t _index_count, GLenum _primitive=GL_TRIANGLES);

		static void drawArrays(const Ref<VertexArray>& _vertex_array, 
							   uint32_t _index_count, 
							   uint32_t _first=0, 
							   bool _depth_test=false, 
							   GLenum _primitive=GL_TRIANGLES);
		static void drawArrays(uint32_t _index_count, 
							   uint32_t _first=0, 
							   bool _depth_test=true, 
							   GLenum _primitive=GL_TRIANGLES);

		static void drawArraysNoDepth(const Ref<VertexArray>& _vertex_array, 
									  uint32_t _index_count, 
									  uint32_t _first=0, 
									  GLenum _primitive=GL_TRIANGLES);
		static void drawArraysNoDepth(uint32_t _index_count, 
									  uint32_t _first=0, 
									  GLenum _primitive=GL_TRIANGLES);

		//
		static void setLineWidth(float _width);

	private:
		static void setupDebugShaders();


	private:
		static Ref<Camera> s_camera;
		static glm::mat4 s_viewProjectionMatrix;

		static Renderer* s_instance;
		static glm::ivec2 s_viewport;

		static glm::ivec2 s_imGuiViewportPos;
		static glm::ivec2 s_imGuiDockPos;
		static glm::ivec2 s_imGuiWinPos;
		static glm::ivec2 s_imGuiViewportOffset;
		static std::string s_imGuiRendererName;
		static bool s_reportImGuiUpdate;

		RenderCommandQueue m_commandQueue;

		static glm::vec4 s_clearColor;

		static Ref<Shader> s_normalShader;
		static Ref<Shader> s_tangentShader;
		static Ref<Shader> s_bitangentShader;
		static bool s_hasNormalShader;
		static bool s_hasTangentShader;
		static bool s_hasBitangentShader;
	};


}



#define SYN_RENDER_PASTE2(a, b) a ## b
#define SYN_RENDER_PASTE(a, b) SYN_RENDER_PASTE2(a, b)
#define SYN_RENDER_UNIQUE(x) SYN_RENDER_PASTE(x, __LINE__)

#define SYN_RENDER_0(code)\
	struct SYN_RENDER_UNIQUE(RenderCommand)\
	{\
		static void execute(void*)\
		{\
			code\
		}\
	};\
	{\
		auto mem = ::Syn::Renderer::submitRenderCommand(SYN_RENDER_UNIQUE(RenderCommand)::execute, sizeof(SYN_RENDER_UNIQUE(RenderCommand)));\
		new (mem) SYN_RENDER_UNIQUE(RenderCommand)();\
	}\

#define SYN_RENDER_1(arg0, code)\
	struct SYN_RENDER_UNIQUE(RenderCommand)\
	{\
		SYN_RENDER_UNIQUE(RenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0)\
		: arg0(arg0) {}\
		\
		static void execute(void* argBuffer)\
		{\
			auto& arg0 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg0;\
			code\
		}\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
	};\
	{\
		auto mem = ::Syn::Renderer::submitRenderCommand(SYN_RENDER_UNIQUE(RenderCommand)::execute, sizeof(SYN_RENDER_UNIQUE(RenderCommand)));\
		new (mem) SYN_RENDER_UNIQUE(RenderCommand)(arg0);\
	}

#define SYN_RENDER_2(arg0, arg1, code)\
	struct SYN_RENDER_UNIQUE(RenderCommand)\
	{\
		SYN_RENDER_UNIQUE(RenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0,\
									 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1)\
		: arg0(arg0), arg1(arg1) {}\
		\
		static void execute(void* argBuffer)\
		{\
			auto& arg0 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg0;\
			auto& arg1 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg1;\
			code\
		}\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1;\
	};\
	{\
		auto mem = ::Syn::Renderer::submitRenderCommand(SYN_RENDER_UNIQUE(RenderCommand)::execute, sizeof(SYN_RENDER_UNIQUE(RenderCommand)));\
		new (mem) SYN_RENDER_UNIQUE(RenderCommand)(arg0, arg1);\
	}

#define SYN_RENDER_3(arg0, arg1, arg2, code)\
	struct SYN_RENDER_UNIQUE(RenderCommand)\
	{\
		SYN_RENDER_UNIQUE(RenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0,\
									 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1,\
									 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2)\
		: arg0(arg0), arg1(arg1), arg2(arg2) {}\
		\
		static void execute(void* argBuffer)\
		{\
			auto& arg0 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg0;\
			auto& arg1 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg1;\
			auto& arg2 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg2;\
			code\
		}\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2;\
	};\
	{\
		auto mem = ::Syn::Renderer::submitRenderCommand(SYN_RENDER_UNIQUE(RenderCommand)::execute, sizeof(SYN_RENDER_UNIQUE(RenderCommand)));\
		new (mem) SYN_RENDER_UNIQUE(RenderCommand)(arg0, arg1, arg2);\
	}

#define SYN_RENDER_4(arg0, arg1, arg2, arg3, code)\
	struct SYN_RENDER_UNIQUE(RenderCommand)\
	{\
		SYN_RENDER_UNIQUE(RenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0,\
									 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1,\
									 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2,\
									 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg3)>::type>::type arg3)\
		: arg0(arg0), arg1(arg1), arg2(arg2), arg3(arg3) {}\
		\
		static void execute(void* argBuffer)\
		{\
			auto& arg0 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg0;\
			auto& arg1 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg1;\
			auto& arg2 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg2;\
			auto& arg3 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg3;\
			code\
		}\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg3)>::type>::type arg3;\
	};\
	{\
		auto mem = ::Syn::Renderer::submitRenderCommand(SYN_RENDER_UNIQUE(RenderCommand)::execute, sizeof(SYN_RENDER_UNIQUE(RenderCommand)));\
		new (mem) SYN_RENDER_UNIQUE(RenderCommand)(arg0, arg1, arg2, arg3);\
	}

#define SYN_RENDER_5(arg0, arg1, arg2, arg3, arg4, code)\
	struct SYN_RENDER_UNIQUE(RenderCommand)\
	{\
		SYN_RENDER_UNIQUE(RenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0,\
										 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1,\
										 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2,\
										 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg3)>::type>::type arg3,\
										 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg4)>::type>::type arg4)\
		: arg0(arg0), arg1(arg1), arg2(arg2), arg3(arg3), arg4(arg4) {}\
		\
		static void execute(void* argBuffer)\
		{\
			auto& arg0 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg0;\
			auto& arg1 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg1;\
			auto& arg2 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg2;\
			auto& arg3 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg3;\
			auto& arg4 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg4;\
			code\
		}\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg3)>::type>::type arg3;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg4)>::type>::type arg4;\
	};\
	{\
		auto mem = ::Syn::Renderer::submitRenderCommand(SYN_RENDER_UNIQUE(RenderCommand)::execute, sizeof(SYN_RENDER_UNIQUE(RenderCommand)));\
		new (mem) SYN_RENDER_UNIQUE(RenderCommand)(arg0, arg1, arg2, arg3, arg4);\
	}

#define SYN_RENDER_6(arg0, arg1, arg2, arg3, arg4, arg5, code)\
	struct SYN_RENDER_UNIQUE(RenderCommand)\
	{\
		SYN_RENDER_UNIQUE(RenderCommand)(typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0,\
										 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1,\
										 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2,\
										 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg3)>::type>::type arg3,\
										 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg4)>::type>::type arg4,\
										 typename ::std::remove_const<typename ::std::remove_reference<decltype(arg4)>::type>::type arg5)\
		: arg0(arg0), arg1(arg1), arg2(arg2), arg3(arg3), arg4(arg4) {}\
		\
		static void execute(void* argBuffer)\
		{\
			auto& arg0 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg0;\
			auto& arg1 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg1;\
			auto& arg2 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg2;\
			auto& arg3 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg3;\
			auto& arg4 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg4;\
			auto& arg4 = ((SYN_RENDER_UNIQUE(RenderCommand)*)argBuffer)->arg5;\
			code\
		}\
		\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg0)>::type>::type arg0;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg1)>::type>::type arg1;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg2)>::type>::type arg2;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg3)>::type>::type arg3;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg4)>::type>::type arg4;\
		typename ::std::remove_const<typename ::std::remove_reference<decltype(arg4)>::type>::type arg5;\
	};\
	{\
		auto mem = ::Syn::Renderer::submitRenderCommand(SYN_RENDER_UNIQUE(RenderCommand)::execute, sizeof(SYN_RENDER_UNIQUE(RenderCommand)));\
		new (mem) SYN_RENDER_UNIQUE(RenderCommand)(arg0, arg1, arg2, arg3, arg4, arg5);\
	}


#define SYN_RENDER_S0(code) auto self = this;\
	SYN_RENDER_1(self, code)

#define SYN_RENDER_S1(arg0, code) auto self = this;\
	SYN_RENDER_2(self, arg0, code)

#define SYN_RENDER_S2(arg0, arg1, code) auto self = this;\
	SYN_RENDER_3(self, arg0, arg1, code)

#define SYN_RENDER_S3(arg0, arg1, arg2, code) auto self = this;\
	SYN_RENDER_4(self, arg0, arg1, arg2, code)

#define SYN_RENDER_S4(arg0, arg1, arg2, arg3, code) auto self = this;\
	SYN_RENDER_5(self, arg0, arg1, arg2, arg3, code)

#define SYN_RENDER_S5(arg0, arg1, arg2, arg3, arg4, code) auto self = this;\
	SYN_RENDER_6(self, arg0, arg1, arg2, arg3, arg4, code)



