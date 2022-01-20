
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>

#include <Synapse.hpp>
#include <Synapse/SynapseMain.hpp>
#include <SynapseAddons.hpp>



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

	void generateNoise();

public:
	// standard objects
	Ref<Font> m_font = nullptr;
	Ref<Framebuffer> m_renderBuffer = nullptr;
	
	Ref<OrthographicCamera> m_camera = nullptr;
	bool m_bCameraMode = true;	// if true, starts in camera move mode, contrasted to edit mode

	// chunk stuff
	Ref<Shader> m_shader = nullptr;
	Ref<MeshShape> m_quad = nullptr;
	Ref<MeshShape> m_quad1 = nullptr;
	Ref<Texture2DNoise> m_texture = nullptr;
	Ref<Texture2DNoise> m_texture1 = nullptr;
	uint32_t m_tileSz = 0;

	// flags
	bool m_wireframeMode = false;
	bool m_toggleCulling = true;

};
class appInstance : public Application
{
public:
	appInstance() { this->pushLayer(new layer); }
};
Application* CreateSynapseApplication() { return new appInstance(); }


//---------------------------------------------------------------------------------------
void layer::onAttach()
{
	SYN_PROFILE_FUNCTION();

	// register event callback

	EventHandler::register_callback(EventType::INPUT_KEY, SYN_EVENT_MEMBER_FNC(layer::onKeyDownEvent));
	EventHandler::register_callback(EventType::INPUT_MOUSE_BUTTON, SYN_EVENT_MEMBER_FNC(layer::onMouseButtonEvent));


	// load shaders
	ShaderLibrary::load("../../assets/shaders/debugShader.glsl");


	// load font
	m_font = MakeRef<Font>("../../assets/ttf/ubuntu.mono.ttf", 14.0f);
	m_font->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));


	// load camera
	m_camera = API::newOrthographicCamera(Renderer::getAspectRatio());
	m_camera->setPosition({ 0.0f, 0.0f, 0.0f });
	m_camera->setRotation(0.0f);
	m_camera->setZoomLevel(4.0f);
	EventHandler::push_event(new WindowToggleFrozenCursorEvent(m_bCameraMode));
	EventHandler::push_event(new WindowToggleCursorEvent(!m_bCameraMode));
	

	// =============== Engine objects ===============

	ShaderLibrary::load("../../assets/shaders/diffuseLightShader.glsl");
	ShaderLibrary::load("../../assets/shaders/vxEngine/noiseTileShader.glsl");
	
	m_shader = ShaderLibrary::get("noiseTileShader");
	m_quad = MeshCreator::createShapeQuad(glm::vec3(0.0f), 1.0f, 
										  MESH_ATTRIB_POSITION | MESH_ATTRIB_UV);
	m_quad1 = MeshCreator::createShapeQuad(glm::vec3(2.0f, 0.0f, 0.0f), 1.0f, 
										  MESH_ATTRIB_POSITION | MESH_ATTRIB_UV);

	m_tileSz = VX_CHUNK_SIZE_XZ;
	m_texture = MakeRef<Texture2DNoise>(m_tileSz);
	m_texture1 = MakeRef<Texture2DNoise>(m_tileSz);
	float n[m_tileSz*m_tileSz];
	float n1[m_tileSz*m_tileSz];
	/* World scale 
	 * 128 seems reasonable for a chunk size of 16. To increase the world size (before repeat at
	 * 256 units), the scale is changed from 1/128 to 1/65536, corresponding to a 2^9 increase
	 * (2^7 * 2^9 = 2^16). The frequency needs to be increased by the same magnitude. For value 
	 * noise, the base frequency is 5.0, thus the new base frequency is set to 5 * 2^9 = 2560.
	 * 
	 * New plan, let's make it as big as possible. 2^23 for the denominator in step and 5 * 2^16
	 * as the frequency. This means that the world should go on for 2^23 * 2^8 = 2^31 units before
	 * repeating.
	 */
	float step = 1.0f / (float)(1<<23);						// = 2^7 * 2^16 = 2^23 
	VxNoise::set_frequency_value(5.0f * (float)(1<<16));	// = 5.0 * 2^16
	for (int y = 0; y < m_tileSz; y++)
	{
		for (int x = 0; x < m_tileSz; x++)
		{
			n[y*m_tileSz+x] = VxNoise::value2_fbm_r(x*step, y*step);
			n1[y*m_tileSz+x] = VxNoise::value2_fbm_r((x+m_tileSz)*step, y*step);
		}
	}
	m_texture->setFromNoiseData(n);
	m_texture1->setFromNoiseData(n1);

	// =============== Engine objects ===============


	// framebuffer
	m_renderBuffer = API::newFramebuffer(ColorFormat::RGBA16F, glm::ivec2(0), 1, true, true, "render_buffer");


	// misc setup
	Renderer::setClearColor(0.0f, 0.0f, 0.0f, 1.0f);

}

//---------------------------------------------------------------------------------------
void layer::onUpdate(float _dt)
{
	SYN_PROFILE_FUNCTION();
	
	static float fontHeight = (float)m_font->getFontHeight() + 1.0f;
	static float renderTime = 0.0f;


	// handle input
	handleInput(_dt);


	// update camera
	m_camera->setUpdateMode(m_bCameraMode);
	m_camera->onUpdate(_dt);

	// bind presenting framebuffer
	m_renderBuffer->bind();

	// clear the screen
	Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// -- BEGINNING OF SCENE -- //

	// toggle wireframe
	if (m_wireframeMode) Renderer::enableWireFrame();

	
	m_shader->enable();
	m_shader->setMatrix4fv("u_view_projection_matrix", m_camera->getViewProjectionMatrix());
	m_texture->bind(0);
	m_shader->setUniform1i("u_sampler", 0);
	m_quad->render(m_shader);
	m_texture1->bind(0);
	m_shader->setUniform1i("u_sampler", 0);
	m_quad1->render(m_shader);


	// toggle wireframe (back)
	if (m_wireframeMode) Renderer::disableWireFrame();

	//if (m_world->renderChunkBoundaries())
	//	MeshCreator::renderDebugMesh("chunk_boundaries", m_camera);

	// -- END OF SCENE -- //



	// Text rendering 
	// TODO: all text rendering should go into an overlay layer.
	int i = 1;
	m_font->beginRenderBlock();
	m_font->addString(2.0f, fontHeight * i++, "fps=%.0f  VSYNC=%s", TimeStep::getFPS(), Application::get().getWindow().isVSYNCenabled() ? "ON" : "OFF");
	glm::vec3 camPos = m_camera->getPosition();
	m_font->addString(2.0f, fontHeight * i++, 
					  "camera [ %.1f  %.1f  %.1f ], theta %.1f  zoom %.1f", 
					  camPos.x, camPos.y, camPos.z, m_camera->getTheta(), m_camera->getZoomLevel());
	m_font->endRenderBlock();


	// ...and we're done! hand-off to ImGui to render the texture (scene) in the viewport pane.
	m_renderBuffer->bindDefaultFramebuffer();

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
			ShaderLibrary::reload("testGridShader");
			break;

		case SYN_KEY_Z:
			vsync = !vsync;
			Application::get().getWindow().setVSYNC(vsync);
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

