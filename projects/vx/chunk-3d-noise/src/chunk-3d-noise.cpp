
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>

#include <Synapse.hpp>
#include <Synapse/SynapseMain.hpp>
#include <SynapseAddons.hpp>

#include "VxChunkManager.hpp"


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
	
	Ref<OrbitCamera> m_orbitCamera = nullptr;
	bool m_bCameraMode = false;	// if true, starts in camera move mode, contrasted to edit mode

	// chunk stuff
	Ref<VxNoise> m_noise = nullptr;
	Ref<Shader> m_shader = nullptr;
	Ref<VxChunkManager> m_world = nullptr;

	glm::vec3 m_lightPos[3];

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
	m_orbitCamera = API::newOrbitCamera(45.0f, SCREEN_WIDTH_F, SCREEN_HEIGHT_F, 0.1f, 1000.0f);
	m_orbitCamera->setRadius(350.0f);
	m_orbitCamera->setZoomSpeed(10.0f);
	m_orbitCamera->setXAngle(110.0f);
	m_orbitCamera->setYAngle(67.0f);

	EventHandler::push_event(new WindowToggleFrozenCursorEvent(m_bCameraMode));
	EventHandler::push_event(new WindowToggleCursorEvent(!m_bCameraMode));


	// =============== Engine objects ===============

	ShaderLibrary::load("../../assets/shaders/vxEngine/vxDiffuseLightShader.glsl");
	ShaderLibrary::load("../../assets/shaders/vxEngine/vxDebugShader.glsl");
	

	m_shader = ShaderLibrary::get("vxDiffuseLightShader");
	m_world = MakeRef<VxChunkManager>();
	m_world->updateChunks(m_orbitCamera->getPosition());

	m_lightPos[0] = glm::vec3(  70.0f, 100.0f, -100.0f);
	m_lightPos[1] = glm::vec3(-100.0f, 100.0f,  100.0f);
	m_lightPos[2] = glm::vec3(   8.0f, 100.0f,    8.0f);
	MeshCreator::createDebugCube(m_lightPos[0], 1.0f, "light0");
	MeshCreator::createDebugCube(m_lightPos[1], 1.0f, "light1");
	MeshCreator::createDebugCube(m_lightPos[2], 1.0f, "light2");
	MeshCreator::setDebugRenderColor(glm::vec3(1.0f));
	m_shader->enable();
	m_shader->setUniform3fv("u_light_position_0", m_lightPos[0]);
	m_shader->setUniform3fv("u_light_position_1", m_lightPos[1]);
	m_shader->setUniform3fv("u_light_position_2", m_lightPos[2]);
	//m_shader->setUniform3fv("u_mesh_color", glm::vec3(0.055, 0.51, 0.158));
	m_shader->setUniform3fv("u_mesh_color", glm::vec3(0.055, 0.51, 0.158));
	
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
	m_orbitCamera->setUpdateMode(m_bCameraMode);
	m_orbitCamera->onUpdate(_dt);

	// bind presenting framebuffer
	m_renderBuffer->bind();

	// clear the screen
	Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// -- BEGINNING OF SCENE -- //

	// toggle wireframe
	if (m_wireframeMode) Renderer::enableWireFrame();


	{
		Timer t("", false);
		m_world->updateChunks(m_orbitCamera->getPosition());
		m_shader->enable();
		m_shader->setMatrix4fv("u_view_projection_matrix", m_orbitCamera->getViewProjectionMatrix());
		m_shader->setUniform3fv("u_camera", m_orbitCamera->getPosition());
		m_shader->setUniform3fv("u_mesh_color", glm::vec3(0.055, 0.51, 0.158));
		m_world->render(m_shader);
		renderTime = t.getDeltaTimeMs();
	}

	// toggle wireframe (back)
	if (m_wireframeMode) Renderer::disableWireFrame();

	// show the camera (for now just the persepctive camera)
	MeshCreator::setDebugRenderColor(glm::vec3(1.0f));
	MeshCreator::setDebugRenderColor(glm::vec3(1.0f, 1.0f, 0.0f));
	MeshCreator::renderDebugMesh("light0", m_orbitCamera, false);
	MeshCreator::renderDebugMesh("light1", m_orbitCamera, false);
	MeshCreator::renderDebugMesh("light2", m_orbitCamera, false);

	//if (m_world->renderChunkBoundaries())
	//	MeshCreator::renderDebugMesh("chunk_boundaries", m_camera);

	// -- END OF SCENE -- //



	// Text rendering 
	// TODO: all text rendering should go into an overlay layer.
	int i = 1;
	m_font->beginRenderBlock();
	m_font->addString(2.0f, fontHeight * i++, "fps=%.0f  VSYNC=%s", TimeStep::getFPS(), Application::get().getWindow().isVSYNCenabled() ? "ON" : "OFF");
	//glm::vec3 camPos = m_camera->getPosition();
	//m_font->addString(2.0f, fontHeight * i++, 
	//				  "camera [ %.1f  %.1f  %.1f ], x %.1f  y %.1f", 
	//				  camPos.x, camPos.y, camPos.z, 
	//				  m_camera->getXAngle(), m_camera->getYAngle());
	glm::vec3 camPos = m_orbitCamera->getPosition();
	m_font->addString(2.0f, fontHeight * i++, 
					  "camera [ %.1f  %.1f  %.1f ], phi %.1f  theta %.1f  r %.1f", 
					  camPos.x, camPos.y, camPos.z, 
					  m_orbitCamera->getYAngle(), m_orbitCamera->getXAngle(), m_orbitCamera->getRadius());
	glm::vec2 mpos = InputManager::get_mouse_position();
	glm::vec2 mdelta = m_orbitCamera->getMouseDelta();
	m_font->addString(2.0f, fontHeight * i++, "mouse pos [ %.0f  %.0f ],  camera delta [ %.0f  %.0f ]", mpos.x, mpos.y, mdelta.x, mdelta.y);
	m_font->addString(2.0f, fontHeight * i++, "chunk count: %d", m_world->getChunkCount());
	m_font->addString(2.0f, fontHeight * i++, "vertex count: %d,  face count: %d", m_world->getVertexCount(), m_world->getFaceCount());
	m_font->addString(2.0f, fontHeight * i++, "render: %.2f ms   mesh: %.2f ms", renderTime, m_world->getDebugMeshingTime());
	AABB wdim = m_world->debugGetAABB();
	m_font->addString(2.0f, fontHeight * i++, "world AABB: min = [ %.0f  %.0f  %.0f ], max = [ %.0f  %.0f  %.0f ]",
					  wdim.min.x, wdim.min.y, wdim.min.z, wdim.max.x, wdim.max.y, wdim.max.z);
	i++;
	m_font->addString(2.0f, fontHeight * i++, "--> destroy random voxels from chunks with '0'.");
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
			break;

		case SYN_KEY_Z:
			vsync = !vsync;
			Application::get().getWindow().setVSYNC(vsync);
			break;

		case SYN_KEY_V:
			m_renderBuffer->saveAsPNG("..");
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
	AABB lim = m_world->debugGetAABB();
	static glm::ivec3 min;
	static glm::ivec3 max;
	static int x0, y0, z0;
	static int radius;

	if (InputManager::is_key_pressed(SYN_KEY_0))
	{
		radius = Random::rand_i_r(5, 25);
		x0 = Random::rand_i_r(lim.min.x-10, lim.max.x+10);
		y0 = Random::rand_i_r(lim.min.y-10, lim.max.y+10);
		z0 = Random::rand_i_r(lim.min.z-10, lim.max.z+10);
		min = { x0, y0, z0 };
		max = { x0+radius, y0+radius, z0+radius };
		m_world->debugDestroyVoxels(min, max);
	}
}

