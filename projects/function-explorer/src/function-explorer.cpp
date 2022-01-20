
#include <Synapse.hpp>
#include <SynapseAddons.hpp>
#include <Synapse/SynapseMain.hpp>
#include "Function3D.hpp"


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
	Ref<Framebuffer> m_renderBuffer = nullptr;
	
	Ref<PerspectiveCamera> m_perspectiveCamera = nullptr;
	Ref<OrbitCamera> m_orbitCamera = nullptr;
	Ref<Camera> m_activeCamera = nullptr;
	bool m_usePerspectiveCamera = false;	// if true, starts in perspective camera mode, else orbital mode

	Ref<Shader> m_shader = nullptr;
	
	//PyFunction3D* m_pyFunction = nullptr;
	Function3D* m_function = nullptr;
	Linspace<float> m_x;
	Linspace<float> m_y;
	char m_expr[512];
	std::string m_exprStr;
	bool m_debugNormals = false;

	float m_materialRoughness = 0.08;
	float m_materialMetallic = 0.96f;
	glm::vec3 m_materialAlbedo = { 0.467f, 0.024f, 0.024f };
	float m_materialAO = 1.0f;
	// flags
	bool m_wireframeMode = true;
	bool m_toggleCulling = false;

};
class functionExplorer : public Application
{
public:
	functionExplorer() { this->pushLayer(new layer); }
};
Application* CreateSynapseApplication() { return new functionExplorer(); }


//---------------------------------------------------------------------------------------
void layer::onAttach()
{
	SYN_PROFILE_FUNCTION();

	// register event callback

	EventHandler::register_callback(EventType::INPUT_KEY, SYN_EVENT_MEMBER_FNC(layer::onKeyDownEvent));
	EventHandler::register_callback(EventType::INPUT_MOUSE_BUTTON, SYN_EVENT_MEMBER_FNC(layer::onMouseButtonEvent));


	// load shaders
	ShaderLibrary::load("../assets/shaders/debugShader.glsl");


	// load font
	m_font = MakeRef<Font>("../assets/ttf/ubuntu.mono.ttf", 14.0f);
	m_font->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));


	// load camera
	m_orbitCamera = API::newOrbitCamera(45.0f, SCREEN_WIDTH_F, SCREEN_HEIGHT_F, 0.1f, 1000.0f);
	m_orbitCamera->setPosition({ 0.0f, 0.0f, 0.0f });
	m_orbitCamera->setYAngle(60.0f);
	m_orbitCamera->setXAngle(20.0f);
	m_orbitCamera->setRadius(1.65f);
	m_orbitCamera->setOrbitSpeed(0.6f);
	m_orbitCamera->setZoomSpeed(0.1f);
	m_orbitCamera->onUpdate(0.0f);
	EventHandler::push_event(new WindowToggleFrozenCursorEvent(m_usePerspectiveCamera));
	EventHandler::push_event(new WindowToggleCursorEvent(!m_usePerspectiveCamera));

	m_perspectiveCamera = API::newPerspectiveCamera(45.0f, SCREEN_WIDTH_F, SCREEN_HEIGHT_F, 0.1f, 400.0f);
	m_perspectiveCamera->setPosition({ 0.0f, 0.0f, 0.0f });
	m_perspectiveCamera->setXAngle(75.0f);
	m_perspectiveCamera->setYAngle(20.0f);
	m_perspectiveCamera->setMoveSpeed(1.0f);
	m_perspectiveCamera->onUpdate(0.0f);


	// =============== function-explorer tests =============== //

	ShaderLibrary::load("../assets/shaders/function-explorer/PBRShader.glsl");
	ShaderLibrary::load("../assets/shaders/function-explorer/testShader.glsl");
	m_shader = ShaderLibrary::get("PBRShader");
	//m_shader = ShaderLibrary::get("testShader");

	m_x = Linspace<float>(-3.0, 3.0, 101);
	m_y = Linspace<float>(-3.0, 3.0, 101);
	m_function = new Function3D(m_x, m_y);
	m_function->evaluateExpression("exp(x)*sin(y)-x*x");
	Renderer::setCulling(m_toggleCulling);

	m_shader->enable();
	// material
	m_shader->setUniform3fv("u_albedo", m_materialAlbedo);
	m_shader->setUniform1f("u_metallic", m_materialMetallic);
	m_shader->setUniform1f("u_roughness", m_materialRoughness);
	m_shader->setUniform1f("u_ao", m_materialAO);
	// lights
	glm::vec3 l0 = {  0.0f,  1.5f,  0.0f };
	glm::vec3 l1 = {  2.0f,  0.5f,  0.0f };
	glm::vec3 l2 = { -1.0f,  0.5f,  1.7f };
	glm::vec3 l3 = { -1.0f,  0.5f, -1.7f };
	m_shader->setUniform3fv("u_point_light_position_0", l0);
	m_shader->setUniform3fv("u_point_light_position_1", l1);
	m_shader->setUniform3fv("u_point_light_position_2", l2);
	m_shader->setUniform3fv("u_point_light_position_3", l3);
	MeshCreator::createDebugCube(l0, 0.03f, "light0");
	MeshCreator::createDebugCube(l1, 0.03f, "light1");
	MeshCreator::createDebugCube(l2, 0.03f, "light2");
	MeshCreator::createDebugCube(l3, 0.03f, "light2");
	MeshCreator::setDebugRenderColor({ 1.0f, 1.0f, 0.0f });

	// ======================================================= //


	// framebuffer
	m_renderBuffer = API::newFramebuffer(ColorFormat::RGBA16F, glm::ivec2(0), 1, true, true, "render_buffer");


	// misc setup
	Renderer::setClearColor(0.0f, 0.0f, 0.0f, 1.0f);


	// execute pending rendering commands
	Renderer::get().executeRenderCommands();

	//EventHandler::push_event(new WindowCloseEvent());
}

//---------------------------------------------------------------------------------------
void layer::onUpdate(float _dt)
{
	SYN_PROFILE_FUNCTION();
	
	static float fontHeight = (float)m_font->getFontHeight() + 1.0f;


	// handle input
	handleInput(_dt);


	// update camera
	if (m_usePerspectiveCamera)
	{
		m_perspectiveCamera->setUpdateMode(m_usePerspectiveCamera);
		m_perspectiveCamera->onUpdate(_dt);
		m_activeCamera = m_perspectiveCamera;
	}
	else
	{
		m_orbitCamera->setUpdateMode(m_usePerspectiveCamera);
		m_orbitCamera->onUpdate(_dt);
		m_activeCamera = m_orbitCamera;
	}

	// bind presenting framebuffer
	m_renderBuffer->bind();

	// toggle wireframe
	if (m_wireframeMode) Renderer::enableWireFrame();

	// clear the screen
	Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// -- BEGINNING OF SCENE -- //
	m_shader->enable();
	m_shader->setMatrix4fv("u_view_projection_matrix", m_activeCamera->getViewProjectionMatrix());
	m_shader->setUniform3fv("u_camera_pos", m_activeCamera->getPosition());
	m_shader->setUniform3fv("u_albedo", m_materialAlbedo);
	m_shader->setUniform1f("u_metallic", m_materialMetallic);
	m_shader->setUniform1f("u_roughness", m_materialRoughness);
	m_shader->setUniform1f("u_ao", m_materialAO);

	m_function->render(m_shader);

	if (m_debugNormals)
	{
		Renderer::debugNormals(m_function->getMesh(), m_activeCamera, 0.05f);
		MeshCreator::renderDebugMeshes(m_activeCamera, false);
	}

	// -- END OF SCENE -- //


	// toggle wireframe (back)
	if (m_wireframeMode) Renderer::disableWireFrame();

	// Text rendering 
	// TODO: all text rendering should go into an overlay layer.
	int i = 1;
	m_font->beginRenderBlock();
	m_font->addString(2.0f, fontHeight * i++, "fps=%.0f  VSYNC=%s", TimeStep::getFPS(), Application::get().getWindow().isVSYNCenabled() ? "ON" : "OFF");
	glm::vec3 camPos = m_orbitCamera->getPosition();
	m_font->addString(2.0f, fontHeight * i++, 
					  "camera 1 [ %.1f  %.1f  %.1f ], phi %.1f  theta %.1f  r %.1f", 
					  camPos.x, camPos.y, camPos.z, 
					  m_orbitCamera->getYAngle(), m_orbitCamera->getXAngle(), m_orbitCamera->getRadius());
	camPos = m_perspectiveCamera->getPosition();
	m_font->addString(2.0f, fontHeight * i++, 
					  "camera 2 [ %.1f  %.1f  %.1f ], X %.1f  Y %.1f", 
					  camPos.x, camPos.y, camPos.z, 
					  m_perspectiveCamera->getXAngle(), m_perspectiveCamera->getYAngle());
	m_font->addString(2.0f, fontHeight * i++, "camera: perspective %s  orbit %s", 
					  m_usePerspectiveCamera ? "TRUE" : "FALSE", m_usePerspectiveCamera ? "FALSE" : "TRUE");
	i++;
	m_font->addString(2.0f, fontHeight * i++, "Function: f(x,y) = %s", m_function->getExpr().c_str());
	m_font->endRenderBlock();


	// ...and we're done! hand-off to ImGui to render the texture (scene) in the viewport pane.
	m_renderBuffer->bindDefaultFramebuffer();

}


//---------------------------------------------------------------------------------------
void layer::onKeyDownEvent(Event* _e)
{
	SYN_PROFILE_FUNCTION();

	KeyDownEvent* e = dynamic_cast<KeyDownEvent*>(_e);

	static bool vsync = true;

	static glm::vec2 vp_offset = Renderer::getImGuiViewPortOffsetF();

	if (e->getAction() == GLFW_PRESS)
	{
		switch (e->getKey())
		{
		case SYN_KEY_R:
			ShaderLibrary::reload(m_shader);
			break;

		case SYN_KEY_Z:
			vsync = !vsync;
			Application::get().getWindow().setVSYNC(vsync);
			break;

		case SYN_KEY_N:
			m_debugNormals = !m_debugNormals;
			break;

		case SYN_KEY_X:
			printf("vp offset = %.0f %.0f\n\n", vp_offset.x, vp_offset.y);
			break;

		case SYN_KEY_ENTER:
			//m_function->evaluateExpression("sin(x)*cos(y)");
			m_exprStr = std::string(m_expr);
			SYN_CORE_TRACE("expression: ", m_exprStr.c_str());
			m_function->evaluateExpression(m_exprStr);
			break;

		case SYN_KEY_B:
			break;

		case SYN_KEY_V:
			m_renderBuffer->saveAsPNG();
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
			m_usePerspectiveCamera = !m_usePerspectiveCamera;
			if (m_usePerspectiveCamera)
				m_perspectiveCamera->resetMousePosition();
			EventHandler::push_event(new WindowToggleFrozenCursorEvent(m_usePerspectiveCamera));
			EventHandler::push_event(new WindowToggleCursorEvent(!m_usePerspectiveCamera));
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
	ImGui::Begin("synapse-core::Function3D interface");
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));

	ImGui::InputText("expression", m_expr, 512);
	ImGui::Text("press <ENTER> to update");

	ImGui::PopStyleVar();
	ImGui::End();
	//-----------------------------------------------------------------------------------
	ImGui::Begin("synapse-core::Function3D material");
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));

	ImGui::SliderFloat("Metallic", &m_materialMetallic, 0.01f, 1.0f, "%.2f");
	ImGui::SliderFloat("Roughness", &m_materialRoughness, 0.01f, 1.0f, "%.2f");
	ImGui::SliderFloat("Ambient Occlusion", &m_materialAO, 0.01f, 2.0f, "%.2f");

	bool hdr = 1, drag_and_drop = 1, alpha_half_preview = 1, alpha_preview = 1, options_menu = 1;
	ImGuiColorEditFlags misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | 
									 (drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) | 
									 (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | 
									 (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);

	ImGui::Text("Albedo");
	ImGui::ColorEdit3("RGB", (float*)&m_materialAlbedo, misc_flags);	

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
	ImGui::Image((void*)m_renderBuffer->getColorAttachmentIDn(0), viewportSize, { 0, 1 }, { 1, 0 });

	ImGui::End();
	ImGui::PopStyleVar();


	// end root
	ImGui::End();

}


//---------------------------------------------------------------------------------------
void layer::handleInput(float _dt)
{
}

