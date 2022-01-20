
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>

#include <Synapse.hpp>
#include <Synapse/SynapseMain.hpp>

#include "noise_2.hpp"
#include "height_map.hpp"


using namespace Syn;

class layer : public Layer
{
public:
	layer() : Layer("layer") {}
	virtual ~layer() 
	{
		if (m_noise)
			delete[] m_noise;
	}

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
	
	Ref<PerspectiveCamera> m_camera = nullptr;
	bool m_bCameraMode = true;	// if true, starts in camera move mode, contrasted to edit mode

	Ref<Shader> m_textureShader = nullptr;
	Ref<Mesh> m_quad = nullptr;

	cnoise m_noiseGenerator;
	int m_textureDim = 256;
	float* m_noise = nullptr;

	Ref<Shader> m_mapShader = nullptr;
	Ref<HeightMap> m_map = nullptr;
	bool m_debugNormals = false;
	bool m_debugTangents = false;
	bool m_debugBitangents = false;
	glm::vec3 m_lightPos = { 0.0f, 0.0f, 0.0f };
	float m_lightTheta = 0.0f;
	float m_lightRadius = 0.0f;
	glm::vec3 m_lightCenter = { 0.0f, 0.0f, 0.0f };
	float m_lightElevation = 0.0f;

	Ref<Texture2DNoise> m_noiseTexture = nullptr;

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
void layer::generateNoise()
{
	float max = -1000.0f, min = 1000.0f;
	{
		Timer timer("noise gen");

		for (int x = 0; x < m_textureDim; x++)
		{
			for (int y = 0; y < m_textureDim; y++)
			{
				int index = y * m_textureDim + x;
				//float n = m_noiseGenerator.fbm_3(glm::vec3(x, y, 0));
				glm::vec4 n = m_noiseGenerator.fbm_3_d(glm::vec3(x, y, 0));
				//m_noise[index] = n;
				m_noise[index] = n.x;

				//max = std::max(n, max);
				//min = std::min(n, min);
			}
		}
		//float r_inv = 1.0f / (max - min);
		//for (int i = 0; i < m_textureDim * m_textureDim; i++)
		//	m_noise[i] = (m_noise[i] - min) * r_inv;
	}
}
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
	m_font = MakeRef<Font>("../../assets/ttf/ubuntu.mono.ttf", 14.0f);
	m_font->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));


	// load camera
	int dim = 32;
	m_camera = API::newPerspectiveCamera(45.0f, SCREEN_WIDTH_F, SCREEN_HEIGHT_F, 0.1f, 1000.0f);
	m_camera->setPosition({ 0.0f, 45.0f, (float(dim/2)) });
	m_camera->setXAngle(90.0f);
	m_camera->setYAngle(40.0f);
	//m_camera->setPosition({ 0.0f, 0.0f, 8.0f });
	//m_camera->setXAngle(0.0f);
	//m_camera->setYAngle(0.0f);
	EventHandler::push_event(new WindowToggleFrozenCursorEvent(m_bCameraMode));
	EventHandler::push_event(new WindowToggleCursorEvent(!m_bCameraMode));
	
	// =============== object init ===============

	ShaderLibrary::load("../../assets/shaders/vxEngine/noiseTextureShader.glsl");
	ShaderLibrary::load("../../assets/shaders/vxEngine/heightmapShader.glsl");

	m_textureShader = ShaderLibrary::get("noiseTextureShader");
	m_quad = MeshCreator::createShapeQuad(glm::vec3(0), 5.0f, MESH_ATTRIB_POSITION | MESH_ATTRIB_UV);

	// set initial noise texture
	m_textureDim = dim;
	m_noiseTexture = MakeRef<Texture2DNoise>(m_textureDim);

	m_noise = new float[m_textureDim * m_textureDim];

	// generate noise
	generateNoise();
	m_noiseTexture->setFromNoiseData(m_noise);


	// heightmap (for debugging partial derivatives of noise)
	m_map = MakeRef<HeightMap>(dim, 2*dim);
	m_map->setupVertexData();

	m_mapShader = ShaderLibrary::get("heightmapShader");
	AABB aabb = m_map->getAABB();
	m_lightTheta = 0.0f;
	m_lightRadius = aabb.max.x + 5.0f;
	m_lightElevation = aabb.max.y + 10.0f;
	m_lightCenter = glm::vec3((aabb.max.x - aabb.min.x) / 2.0f,
							  0.0f,
							  (aabb.max.z - aabb.min.z) / 2.0f);

	m_lightPos = glm::vec3(m_lightCenter.x + m_lightRadius * cos(DEG_TO_RAD(m_lightTheta)), 
						   m_lightElevation, 
						   m_lightCenter.z + m_lightRadius * sin(DEG_TO_RAD(m_lightTheta)));
	
	MeshCreator::createDebugCube(glm::vec3(0.0f), 1.0f, "light");
	MeshCreator::getMeshDebugPtr("light")->setPosition(m_lightPos);
	MeshCreator::setDebugRenderColor(glm::vec3(1.0f));
	
	m_camera->setPosition({ 0.0f, aabb.max.y + 10.0f, (float(dim/2)) });
	

	// static exploration of noise
	




	// =============== object init ===============


	// framebuffer
	m_renderBuffer = API::newFramebuffer(ColorFormat::RGBA16F, glm::ivec2(0), 1, true, true, "render_buffer");


	// misc setup
	Renderer::setClearColor(0.0f, 0.0f, 0.0f, 0.0f);

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
	m_renderBuffer->bind();

	// toggle wireframe
	if (m_wireframeMode) Renderer::enableWireFrame();

	// clear the screen
	Renderer::setClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// -- BEGINNING OF SCENE -- //

	//m_textureShader->enable();
	//m_textureShader->setMatrix4fv("u_view_projection_matrix", m_camera->getViewProjectionMatrix());
	//m_noiseTexture->bind(0);
	//m_textureShader->setUniform1i("u_texture", 0);
	//m_quad->render(m_textureShader);

	m_lightPos = glm::vec3(m_lightCenter.x + m_lightRadius * cos(DEG_TO_RAD(m_lightTheta)), 
						   m_lightElevation, 
						   m_lightCenter.z + m_lightRadius * sin(DEG_TO_RAD(m_lightTheta)));
	MeshCreator::getMeshDebugPtr("light")->setPosition(m_lightPos);

	m_mapShader->enable();
	m_mapShader->setMatrix4fv("u_view_projection_matrix", m_camera->getViewProjectionMatrix());
	m_mapShader->setUniform3fv("u_camera", m_camera->getPosition());
	m_mapShader->setUniform3fv("u_diffuse_light_pos", m_lightPos);
	m_map->render(m_mapShader);

	MeshCreator::renderDebugMesh("light", m_camera, false);

	if (m_debugNormals)	 	Renderer::debugNormals(m_map, m_camera, 2.0f);
	if (m_debugTangents) 	Renderer::debugTangents(m_map, m_camera, 2.0f);
	if (m_debugBitangents)	Renderer::debugBitangents(m_map, m_camera, 2.0f);

	// -- END OF SCENE -- //


	// toggle wireframe (back)
	if (m_wireframeMode) Renderer::disableWireFrame();

	// Text rendering 
	// TODO: all text rendering should go into an overlay layer.
	int i = 1;
	m_font->beginRenderBlock();
	m_font->addString(2.0f, fontHeight * i++, "fps=%.0f  VSYNC=%s", TimeStep::getFPS(), Application::get().getWindow().isVSYNCenabled() ? "ON" : "OFF");
	glm::vec3 camPos = m_camera->getPosition();
	m_font->addString(2.0f, fontHeight * i++, 
					  "camera [ %.1f  %.1f  %.1f ], x %.1f  y %.1f", 
					  camPos.x, camPos.y, camPos.z, 
					  m_camera->getXAngle(), m_camera->getYAngle());
	m_font->addString(2.0f, fontHeight * i++, "light theta = %.1f", DEG_TO_RAD(m_lightTheta));
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

		case SYN_KEY_N:
			m_debugNormals = !m_debugNormals;
			break;
		case SYN_KEY_B:
			m_debugBitangents = !m_debugBitangents;
			break;
		case SYN_KEY_T:
			m_debugTangents = !m_debugTangents;
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
	
	ImGui::Begin("synapse-core::noise_2");
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

	static int o = 5;
	ImGui::SliderInt("octaves", &o, 1, 9);
	m_noiseGenerator.set_octaves(o);

	static float f = 0.01f;
	ImGui::SliderFloat("frequency", &f, 0.001f, 0.5f);
	m_noiseGenerator.set_frequency(f);

	static float df = 2.0f;
	ImGui::SliderFloat("delta frequency", &df, 1.5f, 2.5f);
	m_noiseGenerator.set_delta_freq(df);

	if (ImGui::Button("Reload"))
	{
		generateNoise();
		m_noiseTexture->setFromNoiseData(m_noise);
	}

	ImGui::PopStyleVar();
	ImGui::End();
	
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
	if (InputManager::is_key_pressed(SYN_KEY_E))
	{
		m_lightTheta += 1.0f;
		if (m_lightTheta >= 360.0f)	
			m_lightTheta = 0.0f;
	}
	else if (InputManager::is_key_pressed(SYN_KEY_Q))
	{
		m_lightTheta -= 1.0f;
		if (m_lightTheta <= 0.0f)	
			m_lightTheta = 360.0f;		
	}
}

