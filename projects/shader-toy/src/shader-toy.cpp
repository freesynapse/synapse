
#include <Synapse.hpp>
#include <Synapse/SynapseMain.hpp>


class layer : public Syn::Layer
{
public:
	layer() : Layer("layer") {}
	virtual ~layer() {}

	virtual void onAttach() override;
	virtual void onUpdate(float _dt) override;
	virtual void onImGuiRender() override;
	void onKeyDownEvent(Syn::Event* _e);
	void onMouseButtonEvent(Syn::Event* _e);
	void handleInput(float _dt);


public:
	// standard objects
	Syn::Ref<Syn::Font> m_font = nullptr;
	Syn::Ref<Syn::Framebuffer> m_fboRender = nullptr;
	
	Syn::Ref<Syn::PerspectiveCamera> m_camera = nullptr;
	bool m_bCameraMode = true;	// if true, starts in camera move mode, contrasted to edit mode

	Syn::Ref<Syn::MeshShape> m_quad = nullptr;
	Syn::Ref<Syn::Shader> m_shader = nullptr;
	glm::vec2 m_resolution;
	glm::vec2 m_invResolution;
	int m_gridSize;

	// flags
	bool m_wireframeMode = false;
	bool m_toggleCulling = true;

};
class bloom_filter : public Syn::Application
{
public:
	bloom_filter() { this->pushLayer(new layer); }
};
Syn::Application* CreateSynapseApplication() { return new bloom_filter(); }


//---------------------------------------------------------------------------------------
void layer::onAttach()
{
	SYN_PROFILE_FUNCTION();

	// register event callback
	Syn::EventHandler::register_callback(Syn::EventType::INPUT_KEY, SYN_EVENT_MEMBER_FNC(layer::onKeyDownEvent));
	Syn::EventHandler::register_callback(Syn::EventType::INPUT_MOUSE_BUTTON, SYN_EVENT_MEMBER_FNC(layer::onMouseButtonEvent));


	// default shaders
	Syn::ShaderLibrary::load("../assets/shaders/debugShader.glsl");


	// load font
	m_font = Syn::MakeRef<Syn::Font>("../assets/ttf/ubuntu.mono.ttf", 16.0f);
	m_font->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));


	// camera
	m_camera = Syn::MakeRef<Syn::PerspectiveCamera>(glm::perspectiveFov(glm::radians(45.0f), SCREEN_WIDTH_F, SCREEN_HEIGHT_F, 0.1f, 1000.0f));
	m_camera->setPosition(glm::vec3(0.0f, 4.0f, 1.0f));
	m_camera->setXAngle(0.0f);
	m_camera->setYAngle(0.0f);
	Syn::EventHandler::push_event(new Syn::WindowToggleFrozenCursorEvent(m_bCameraMode));
	Syn::EventHandler::push_event(new Syn::WindowToggleCursorEvent(!m_bCameraMode));


	// toy!
	m_quad = Syn::MeshCreator::createShapeViewportQuad();
	Syn::ShaderLibrary::load("../assets/shaders/shader-toy/starShader.glsl");
	Syn::ShaderLibrary::load("../assets/shaders/shader-toy/gridShader.glsl");
	Syn::ShaderLibrary::load("../assets/shaders/shader-toy/starfieldShader.glsl");
	m_shader = Syn::ShaderLibrary::get("starfieldShader");
	m_resolution = Syn::Renderer::getViewportF();
	m_invResolution = 1.0f / m_resolution;
	m_gridSize = 21;


	// the default rendered scene framebuffer, for hand-off to ImGui for rendering
	m_fboRender = Syn::API::newFramebuffer(Syn::ColorFormat::RGBA16F, glm::ivec2(0), 1, true, true, "render_buffer");


	// misc setup
	Syn::Renderer::setClearColor(0.0f, 0.0f, 0.0f, 1.0f);


	// execute pending rendering commands
	Syn::Renderer::get().executeRenderCommands();

}

//---------------------------------------------------------------------------------------
void layer::onUpdate(float _dt)
{
	static double time = 0.0;


	SYN_PROFILE_FUNCTION();
	
	static float fontHeight = (float)m_font->getFontHeight() + 1.0f;


	// handle input
	handleInput(_dt);


	// update camera
	m_camera->setUpdateMode(m_bCameraMode);
	m_camera->onUpdate(_dt);

	// toggle wireframe
	if (m_wireframeMode) Syn::Renderer::enableWireFrame();
	

	// -- BEGINNING OF SCENE -- //

	m_fboRender->bind();
	Syn::Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render quad
	m_shader->enable();
	m_shader->setUniform1f("time", time);
	m_shader->setUniform2fv("resolution", m_resolution);
	m_shader->setUniform2fv("inv_resolution", m_invResolution);
	m_quad->renderNDC();
	
	time += (double)_dt;
	
	// -- END OF SCENE -- //


	// toggle wireframe (back)
	if (m_wireframeMode) Syn::Renderer::disableWireFrame();

	// Text rendering 
	// TODO: all text rendering should go into an overlay layer.
	m_font->beginRenderBlock();
	m_font->addString(2.0f, 0, "fps=%.0f  VSYNC=%s", Syn::TimeStep::getFPS(), Syn::Application::get().getWindow().isVSYNCenabled() ? "ON" : "OFF");
	// ...and we're done! hand-off to ImGui to render the texture (scene) in the viewport pane.
	m_fboRender->bindDefaultFramebuffer();


	//Syn::EventHandler::push_event(new Syn::WindowCloseEvent());

}


//---------------------------------------------------------------------------------------
void layer::onKeyDownEvent(Syn::Event* _e)
{
	SYN_PROFILE_FUNCTION();

	Syn::KeyDownEvent* e = dynamic_cast<Syn::KeyDownEvent*>(_e);

	static bool vsync = true;

	if (e->getAction() == GLFW_PRESS)
	{
		switch (e->getKey())
		{
		case SYN_KEY_R:
			m_shader->reload();
			break;

		case SYN_KEY_Z:
			vsync = !vsync;
			Syn::Application::get().getWindow().setVSYNC(vsync);
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
			Syn::EventHandler::push_event(new Syn::WindowCloseEvent());
			break;

		case SYN_KEY_F4:
			m_wireframeMode = !m_wireframeMode;
			break;

		case SYN_KEY_F5:
			m_toggleCulling = !m_toggleCulling;
			Syn::Renderer::setCulling(m_toggleCulling);
			break;

		case SYN_KEY_F6:
			break;

		case SYN_KEY_F7:
			break;

		case SYN_KEY_F8:
			break;

		case SYN_KEY_TAB:
			m_bCameraMode = !m_bCameraMode;
			Syn::EventHandler::push_event(new Syn::WindowToggleFrozenCursorEvent(m_bCameraMode));
			Syn::EventHandler::push_event(new Syn::WindowToggleCursorEvent(!m_bCameraMode));
			// if entering camera mode again, adjust cursor position
			//if (m_fpsCameraMode)
			//{
			//	glm::vec2 half_vp = Syn::Renderer::getViewportF() * 0.5f;
			//	Syn::Application::get().getWindow().setCursorPos(glm::vec2(floor(half_vp.x), floor(half_vp.y)));
			//	m_cameraPerspective->resetMousePosition();
			//}
			break;
		}
	}

}


//---------------------------------------------------------------------------------------
void layer::onMouseButtonEvent(Syn::Event* _e)
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
	ImGuiTextBuffer buffer = Syn::Log::getImGuiBuffer();
	const char* bufBegin = buffer.begin();
	const char* bufEnd = buffer.end();

	// only process lines within the visible area
	ImGuiListClipper clipper;
	ImVector<int> lineOffsets = Syn::Log::getImGuiLineOffset();
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
	ImGui::Begin(Syn::Renderer::getImGuiRenderTargetName().c_str());
	static ImVec2 oldSize;
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	if (viewportSize.x != oldSize.x && viewportSize.y != oldSize.y)
	{
		// dispatch a viewport resize event -- registered classes will receive this, including:
		//  - camera
		//	- font renderer
		//	- 3D renderer
		//	- framebuffer instances
		Syn::EventHandler::push_event(new Syn::ViewportResizeEvent(glm::vec2(viewportSize.x, viewportSize.y)));
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
}

