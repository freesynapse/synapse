
#include <Synapse.hpp>
#include <Synapse/SynapseMain.hpp>

#include "SandWorld.hpp"


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
	
	Ref<PerspectiveCamera> m_camera = nullptr;
	bool m_bCameraMode = true;	// if true, starts in camera move mode, contrasted to edit mode

	glm::vec2 m_resolution;
	glm::vec2 m_invResolution;
	SandWorld* m_world = nullptr;

	// flags
	bool m_wireframeMode = false;
	bool m_toggleCulling = true;

};
class sand : public Application
{
public:
	sand() { this->pushLayer(new layer); }
};
Application* CreateSynapseApplication() { return new sand(); }


//---------------------------------------------------------------------------------------
void layer::onAttach()
{
	SYN_PROFILE_FUNCTION();

	// register event callback
	EventHandler::register_callback(EventType::INPUT_KEY, SYN_EVENT_MEMBER_FNC(layer::onKeyDownEvent));
	EventHandler::register_callback(EventType::INPUT_MOUSE_BUTTON, SYN_EVENT_MEMBER_FNC(layer::onMouseButtonEvent));


	// default shaders
	ShaderLibrary::load("../assets/shaders/debugShader.glsl");


	// load font
	m_font = MakeRef<Font>("../assets/ttf/ubuntu.mono.ttf", 16.0f);
	m_font->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));


	// camera
	m_camera = API::newPerspectiveCamera(glm::perspectiveFov(glm::radians(45.0f), SCREEN_WIDTH_F, SCREEN_HEIGHT_F, 0.1f, 1000.0f)); 
    //MakeRef<PerspectiveCamera>(glm::perspectiveFov(glm::radians(45.0f), SCREEN_WIDTH_F, SCREEN_HEIGHT_F, 0.1f, 1000.0f));
	m_camera->setPosition(glm::vec3(-3.5f, 4.0f, 3.5f));
	m_camera->setXAngle(45.0f);
	m_camera->setYAngle(35.0f);
	

	// set camera mode
	EventHandler::push_event(new WindowToggleFrozenCursorEvent(m_bCameraMode));
	EventHandler::push_event(new WindowToggleCursorEvent(!m_bCameraMode));


	// BEGIN : SAND

	ShaderLibrary::load("../assets/shaders/Synapse-sand/sandShader.glsl");
	m_world = new SandWorld(0, 0, 2, ShaderLibrary::get("sandShader"));

    // END : SAND


	// the default rendered scene framebuffer, for hand-off to ImGui for rendering
	m_fboRender = API::newFramebuffer(ColorFormat::RGBA16F, glm::ivec2(0), 1, true, true, "render_buffer");


	// misc setup
	Renderer::setClearColor(0.0f, 0.0f, 0.0f, 1.0f);


	// execute pending rendering commands
	Renderer::get().executeRenderCommands();

}

//---------------------------------------------------------------------------------------
void layer::onUpdate(float _dt)
{
	SYN_PROFILE_FUNCTION();
	
    static double time = 0.0;
	static float fontHeight = (float)m_font->getFontHeight() + 1.0f;


	// handle input
	handleInput(_dt);


	// update camera
	m_camera->setUpdateMode(m_bCameraMode);
	m_camera->onUpdate(_dt);

	// bind default render buffer
    m_fboRender->bind();
	
	// toggle wireframe
	if (m_wireframeMode) Renderer::enableWireFrame();

    // clear buffer for rendering
	Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    
    // -- BEGINNING OF SCENE -- //
	//
    
    //MeshCreator::renderDebugMeshes(m_camera);
    m_world->onUpdate(_dt);
	m_world->onRender();    

	//
	// -- END OF SCENE -- //


	// toggle wireframe (back)
	if (m_wireframeMode) Renderer::disableWireFrame();

	// Text rendering 
	// TODO: all text rendering should go into an overlay layer.
	m_font->beginRenderBlock();
	int i = 0;
	m_font->addString(2.0f, fontHeight * ++i, "fps=%.0f  VSYNC=%s", TimeStep::getFPS(), \
                    	Application::get().getWindow().isVSYNCenabled() ? "ON" : "OFF");
	m_font->addString(2.0f, fontHeight * ++i, "delta_t=%.8f", TimeStep::getDeltaTime());
	m_font->addString(2.0f, fontHeight * ++i, "brush: %s", m_world->getSelectedBrushStr());
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
			//ShaderLibrary::reload("sandShader");
			m_world->setSelectedBrush(_ROCK);
			break;

		case SYN_KEY_W:
			m_world->setSelectedBrush(_WATER);
			break;

		case SYN_KEY_A:
			m_world->setSelectedBrush(_NONE);
			break;

		case SYN_KEY_S:
			m_world->setSelectedBrush(_SAND);
			break;

		case SYN_KEY_Z:
			vsync = !vsync;
			Application::get().getWindow().setVSYNC(vsync);
			break;

		case SYN_KEY_X:
			delete m_world;
			m_world = new SandWorld(0, 0, 2, ShaderLibrary::get("sandShader"));
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
			m_camera->setUpdateMode(m_bCameraMode);
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
	ImGui::Begin("synapse-core::controls");
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::PopStyleVar();
	ImGui::End();
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
	ImGui::Begin(Renderer::getImGuiRenderTargetName().c_str());
	static ImVec2 oldSize;
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	if (viewportSize.x != oldSize.x && viewportSize.y != oldSize.y)
	{
		// dispatch a viewport resize event -- registered classes will receive this, including:
		//  - camera
		//	- font renderer
		//	- 3D renderer
		//	- framebuffer instances
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
}

