
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>

#include <Synapse.hpp>
#include <Synapse/SynapseMain.hpp>

using namespace Syn;

class layer : public Layer
{
public:
	layer() : Layer("layer") {}
	virtual ~layer() {}

	virtual void onAttach() override;
	virtual void onUpdate(float _dt) override;
	virtual void onImGuiRender() override;
	void onKeyDownEvent(Event* _e);
	void onMouseButtonEvent(Event* _e);
	void handleInput(float _dt);


public:
	// standard objects
	Ref<Font> m_font = nullptr;
	Ref<Framebuffer> m_fboRender = nullptr;
	
	Ref<OrthographicCamera> m_camera = nullptr;
	bool m_bCameraMode = false;	// if true, starts in camera move mode, contrasted to edit mode
	Ref<Shader> m_shader2D = nullptr;

	struct vertex 
	{ 
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec4 color;
		vertex() {}
		vertex(glm::vec3 _p, glm::vec2 _uv, glm::vec4 _c) :
			position(_p), uv(_uv), color(_c) {}
	};

	Ref<VertexArray> m_vao;
	uint32_t m_quadOffset = 0;
	uint32_t m_maxQuads = 4000;

	// flags
	bool m_wireframeMode = false;
	bool m_toggleCulling = true;

};
class renderer2D_sandbox : public Application
{
public:
	renderer2D_sandbox() { this->pushLayer(new layer); }
};
Application* CreateSynapseApplication() { return new renderer2D_sandbox(); }


//---------------------------------------------------------------------------------------
void layer::onAttach()
{
	SYN_PROFILE_FUNCTION();

	// register event callback

	EventHandler::register_callback(EventType::INPUT_KEY, SYN_EVENT_MEMBER_FNC(layer::onKeyDownEvent));
	EventHandler::register_callback(EventType::INPUT_MOUSE_BUTTON, SYN_EVENT_MEMBER_FNC(layer::onMouseButtonEvent));


	// load shaders

	ShaderLibrary::load("../assets/shaders/debugShader.glsl");
	ShaderLibrary::load("../assets/shaders/simple2DShader.glsl");
	m_shader2D = ShaderLibrary::get("simple2DShader");


	// load font

	m_font = MakeRef<Font>("../assets/ttf/ubuntu.mono.ttf", 16.0f);

	m_font->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));


	// =============== Renderer2D tests ===============
	
	// load camera
	// initialization with aspect ratio and zoom level (default = 1.0f)
	m_camera = API::newOrthographicCamera(Renderer::getAspectRatio());
	m_camera->setZoomLevel(10.0f);
	// set camera to edit mode
	EventHandler::push_event(new WindowToggleFrozenCursorEvent(m_bCameraMode));
	EventHandler::push_event(new WindowToggleCursorEvent(!m_bCameraMode));

	ShaderLibrary::load("../assets/shaders/2D-sandbox/testVBO.glsl");
	m_shader2D = ShaderLibrary::get("testVBO");

	Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_DYNAMIC_DRAW);
	vbo->setBufferLayout({
		{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
		{ VERTEX_ATTRIB_LOCATION_UV, ShaderDataType::Float2, "a_uv" },
		{ VERTEX_ATTRIB_LOCATION_COLOR, ShaderDataType::Float4, "a_color" }
	});
	vbo->setData(nullptr, m_maxQuads * sizeof(vertex) * 4);

	uint32_t indices[m_maxQuads * 6];
	int offset = 0;
	for (int i = 0; i < m_maxQuads * 6; i+=6)
	{
		indices[i+0] = 0 + offset;
		indices[i+1] = 1 + offset;
		indices[i+2] = 2 + offset;
		
		indices[i+3] = 2 + offset;
		indices[i+4] = 3 + offset;
		indices[i+5] = 0 + offset;

		offset += 4;
	}


	Ref<IndexBuffer> ibo = API::newIndexBuffer(GL_TRIANGLES, GL_DYNAMIC_DRAW);
	ibo->setData((void*)indices, sizeof(indices) / sizeof(uint32_t));

	m_vao = API::newVertexArray(vbo, ibo);


	// =============== Renderer2D tests ===============


	// framebuffer
	// the final, rendered scene framebuffer, for hand-off to ImGui for rendering
	m_fboRender = API::newFramebuffer(ColorFormat::RGBA16F, glm::ivec2(0), 1, true, true, "render_buffer");


	// misc setup

	Renderer::setClearColor(0.0f, 0.0f, 0.0f, 0.0f);


	// execute pending rendering commands

	Renderer::get().executeRenderCommands();

}

//---------------------------------------------------------------------------------------
void layer::onUpdate(float _dt)
{
	SYN_PROFILE_FUNCTION();
	
	static float fontHeight = (float)m_font->getFontHeight() + 1.0f;


	// handle input
	handleInput(_dt);


	// update camera
	m_camera->setUpdateMode(m_bCameraMode);
	m_camera->onUpdate(_dt);

	// bind presenting framebuffer
	m_fboRender->bind();

	// toggle wireframe
	if (m_wireframeMode) Renderer::enableWireFrame();

	// clear the screen
	Renderer::setClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// -- BEGINNING OF SCENE -- //
	#ifdef DEBUG_RENDERER_2D
		Renderer2D::resetStatistics();
	#endif

	m_shader2D->enable();
	m_shader2D->setMatrix4fv("u_view_projection_matrix", m_camera->getViewProjectionMatrix());
	m_vao->bind();
	Renderer::drawIndexed(m_maxQuads * 6, true, GL_TRIANGLES);


	// -- END OF SCENE -- //


	// toggle wireframe (back)
	if (m_wireframeMode) Renderer::disableWireFrame();

	// Text rendering 
	// TODO: all text rendering should go into an overlay layer.
	m_font->beginRenderBlock();
	int i = 1;
	m_font->addString(2.0f, fontHeight * i++, "fps=%.0f  VSYNC=%s", TimeStep::getFPS(), Application::get().getWindow().isVSYNCenabled() ? "ON" : "OFF");
	glm::vec3 camPos = m_camera->getPosition();
	float camTheta = m_camera->getTheta();
	m_font->addString(2.0f, fontHeight * i++, 
					  "camera [ %.1f  %.1f ]  theta: %.1f   zoom: %.1f", 
					  camPos.x, camPos.y, 
					  m_camera->getTheta(), m_camera->getZoomLevel());
	m_font->addString(2.0f, fontHeight * i++, "sprites: %d", m_quadOffset);
	#ifdef DEBUG_RENDERER_2D
		m_font->addString(2.0f, fontHeight * 3, "draw calls: %d", Renderer2D::getStatistics().drawCalls);
		m_font->addString(2.0f, fontHeight * 4, "sprites:    %d", Renderer2D::getStatistics().spriteCount);
		m_font->addString(2.0f, fontHeight * 5, "vertices:   %d", Renderer2D::getStatistics().getVertexCount());
		m_font->addString(2.0f, fontHeight * 6, "indices:    %d", Renderer2D::getStatistics().getIndexCount());
	#endif
	m_font->endRenderBlock();


	// ...and we're done! hand-off to ImGui to render the texture (scene) in the viewport pane.
	m_fboRender->bindDefaultFramebuffer();

	//EventHandler::push_event(new WindowCloseEvent());
}


//---------------------------------------------------------------------------------------
void layer::onKeyDownEvent(Event* _e)
{
	SYN_PROFILE_FUNCTION();

	KeyDownEvent* e = dynamic_cast<KeyDownEvent*>(_e);

	static bool vsync = true;

	if (e->getAction() == GLFW_PRESS)
	{
		switch (e->getKey())
		{
		case SYN_KEY_R:
			//ShaderLibrary::reload("textureShader");
			break;

		case SYN_KEY_Z:
			vsync = !vsync;
			Application::get().getWindow().setVSYNC(vsync);
			break;

		case SYN_KEY_X:
			break;

		case SYN_KEY_C:
			break;

		case SYN_KEY_B:
			break;

		case SYN_KEY_V:
			m_fboRender->saveAsPNG();
			break;

		case SYN_KEY_ESCAPE:
			EventHandler::push_event(new WindowCloseEvent());
			break;

		case SYN_KEY_F4:
			m_wireframeMode = !m_wireframeMode;
			break;

		case SYN_KEY_F5:
			m_toggleCulling = !m_toggleCulling;
			Renderer::setCulling(m_toggleCulling);
			break;

		case SYN_KEY_F6:
			break;

		case SYN_KEY_F7:
			break;

		case SYN_KEY_F8:
			break;

		case SYN_KEY_TAB:
			m_bCameraMode = !m_bCameraMode;
			EventHandler::push_event(new WindowToggleFrozenCursorEvent(m_bCameraMode));
			EventHandler::push_event(new WindowToggleCursorEvent(!m_bCameraMode));
			// if entering camera mode again, adjust cursor position
			//if (m_fpsCameraMode)
			//{
			//	glm::vec2 half_vp = Renderer::getViewportF() * 0.5f;
			//	Application::get().getWindow().setCursorPos(glm::vec2(floor(half_vp.x), floor(half_vp.y)));
			//	m_cameraPerspective->resetMousePosition();
			//}
			break;
		}
	}

}


//---------------------------------------------------------------------------------------
void layer::onMouseButtonEvent(Event* _e)
{
}


//---------------------------------------------------------------------------------------
void layer::onImGuiRender()
{
	SYN_PROFILE_FUNCTION();

	static bool p_open = true;

	static bool opt_fullscreen_persistant = true;
	static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
	bool opt_fullscreen = opt_fullscreen_persistant;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (opt_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	window_flags |= ImGuiWindowFlags_NoTitleBar;


	//-----------------------------------------------------------------------------------
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("synapse-core", &p_open, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// Dockspace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("dockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);
	}

	//-----------------------------------------------------------------------------------

#ifdef DEBUG_IMGUI_LOG
	ImGui::Begin("synapse-core::log");
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGuiTextBuffer buffer = Log::getImGuiBuffer();
	const char* bufBegin = buffer.begin();
	const char* bufEnd = buffer.end();

	// only process lines within the visible area
	ImGuiListClipper clipper;
	ImVector<int> lineOffsets = Log::getImGuiLineOffset();
	clipper.Begin(lineOffsets.Size);
	while (clipper.Step())
	{
		for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
		{
			const char* lineStart = bufBegin + lineOffsets[line_no];
			const char* lineEnd = (line_no + 1 < lineOffsets.Size) ? (bufBegin + lineOffsets[line_no + 1] - 1) : bufEnd;
			ImGui::TextUnformatted(lineStart, lineEnd);
		}
	}

	// scroll to end
	ImGui::SetScrollHereY(1.0f);


	//ImGui::TextUnformatted(bufBegin, bufEnd);
	ImGui::PopStyleVar();
	ImGui::End();
#endif

	//-----------------------------------------------------------------------------------

	// set the 'rest' of the window as the viewport
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("synapse-core::renderer");
	static ImVec2 oldSize;
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	if (viewportSize.x != oldSize.x && viewportSize.y != oldSize.y)
	{
		// dispatch a viewport resize event -- registered classes will receive this.
		EventHandler::push_event(new ViewportResizeEvent(glm::vec2(viewportSize.x, viewportSize.y)));
		SYN_CORE_TRACE("viewport [ ", viewportSize.x, ", ", viewportSize.y, " ]");
		oldSize = viewportSize;
	}

	// direct ImGui to the framebuffer texture
	ImGui::Image((void*)m_fboRender->getColorAttachmentIDn(0), viewportSize, { 0, 1 }, { 1, 0 });

	ImGui::End();
	ImGui::PopStyleVar();


	// end root
	ImGui::End();

}


//---------------------------------------------------------------------------------------
void layer::handleInput(float _dt)
{
	if (InputManager::is_button_pressed(SYN_MOUSE_BUTTON_1))
	{
		glm::vec2 m = InputManager::get_mouse_position();
		glm::vec2 vp = Renderer::getViewportF();
		glm::vec2 cpos = m_camera->getPosition();
		auto bounds = m_camera->getBounds();
		static glm::vec2 vpPos = Renderer::getViewportPos();
		m -= Renderer::getImGuiViewportOffset();

		m.x = (m.x / vp.x) * bounds.getWidth() - bounds.getWidth() * 0.5f;
		m.y = bounds.getHeight() * 0.5f - (m.y / vp.y) * bounds.getHeight();

		glm::vec3 clickPos = { m.x + cpos.x, m.y + cpos.y, 0.0f };
		int nNewQuads = 20;
		vertex vertices[nNewQuads * 4];
		for (int i = 0; i < nNewQuads; i++)
		{
			glm::vec3 r = glm::vec3(Random::rfloat()-0.5f, Random::rfloat()-0.5f, Random::rfloat()-0.5f) * 4.0f;

			vertices[i*4+0] = { glm::vec3(-0.5f, -0.5f,  0.5f) + clickPos + r, glm::vec2(0.0f, 0.0f), glm::vec4(0.83f, 0.79f, 0.18f, 1.0f) };
			vertices[i*4+1] = { glm::vec3( 0.5f, -0.5f,  0.5f) + clickPos + r, glm::vec2(1.0f, 0.0f), glm::vec4(0.83f, 0.79f, 0.18f, 1.0f) };
			vertices[i*4+2] = { glm::vec3( 0.5f,  0.5f,  0.5f) + clickPos + r, glm::vec2(1.0f, 1.0f), glm::vec4(0.83f, 0.79f, 0.18f, 1.0f) };
			vertices[i*4+3] = { glm::vec3(-0.5f,  0.5f,  0.5f) + clickPos + r, glm::vec2(0.0f, 1.0f), glm::vec4(0.83f, 0.79f, 0.18f, 1.0f) };
		}

		Ref<VertexBuffer> vbo = m_vao->getVertexBuffer();
		vbo->bind();
		vbo->addSubData((void*)vertices, sizeof(vertices), m_quadOffset * (sizeof(vertex) * 4));
		vbo->unbind();
		Renderer::get().executeRenderCommands();
		m_quadOffset = (m_quadOffset + nNewQuads) % m_maxQuads;

		
	}	
}

