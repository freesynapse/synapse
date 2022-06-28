
#include <imgui/imgui.h>
#include <Synapse.hpp>
#include <Synapse/SynapseMain.hpp>

#include <map>

#include "imgui/imgui_internal.h"

using namespace Syn;
#undef DEBUG_IMGUI_LOG

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

	std::map<int, std::string> m_map;

	Ref<PerspectiveCamera> m_perspectiveCamera = nullptr;
	Ref<OrbitCamera> m_orbitCamera = nullptr;
	Ref<Camera> m_activeCamera = nullptr;
	bool m_usePerspectiveCamera = false;	// if true, starts in perspective camera mode, else orbital mode

	Ref<Shader> m_shader = nullptr;
	
	std::string m_message = "";

	// flags
	bool m_wireframeMode = true;
	bool m_toggleCulling = false;

};
class imgui_tests : public Application
{
public:
	imgui_tests() { this->pushLayer(new layer); }
};
Application* CreateSynapseApplication() { return new imgui_tests(); }

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
	m_font = MakeRef<Font>("../assets/ttf/JetBrains/JetBrainsMono-Medium.ttf", 14.0f);
	m_font->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));


	// load camera
	m_orbitCamera = API::newOrbitCamera(45.0f, SCREEN_WIDTH_F, SCREEN_HEIGHT_F, 0.1f, 1000.0f);
	m_orbitCamera->setPosition({ 0.0f, 0.0f, 0.0f });
	m_orbitCamera->setYAngle(60.0f);
	m_orbitCamera->setXAngle(20.0f);
	m_orbitCamera->setRadius(10.0f);
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

	// something to look at..
	MeshCreator::createDebugCube();
	m_map.insert(std::pair<int, std::string>(0, "aaa"));
	m_map.insert(std::pair<int, std::string>(2, "bbb"));
	m_map.insert(std::pair<int, std::string>(4, "ccc"));
	m_map.insert(std::pair<int, std::string>(6, "ddd"));
	m_map.insert(std::pair<int, std::string>(8, "eee"));
	m_map.insert(std::pair<int, std::string>(10, "fff"));
	m_map.insert(std::pair<int, std::string>(12, "ggg"));
	m_map.insert(std::pair<int, std::string>(14, "hhh"));

	// framebuffer
	m_renderBuffer = API::newFramebuffer(ColorFormat::RGBA16F, glm::ivec2(0), 1, true, true, "render_buffer");


	// misc setup
	Renderer::setClearColor(0.0f, 0.0f, 0.0f, 1.0f);


	// Test fps cap
	Application::get().setMaxFPS(30.0f);
	//

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

	MeshCreator::renderDebugMeshes(m_activeCamera);

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
			break;

		case SYN_KEY_X:
			if (InputManager::is_key_pressed(SYN_KEY_LEFT_CONTROL) || InputManager::is_key_pressed(SYN_KEY_RIGHT_CONTROL))
				EventHandler::push_event(new WindowCloseEvent);
			break;
			
		case SYN_KEY_W:
			if (InputManager::is_key_pressed(SYN_KEY_LEFT_CONTROL) || InputManager::is_key_pressed(SYN_KEY_RIGHT_CONTROL))
				EventHandler::push_event(new WindowCloseEvent);
			break;

		case SYN_KEY_ENTER:
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

	ImGui::GetCurrentContext()->NavWindowingToggleLayer = false;

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
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("settings");
	if (ImGui::BeginTable("split1", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
	{
		static bool selected[10] = {};
		for (int i = 0; i < 10; i++)
		{
			char label[32];
			sprintf(label, "Item %d", i);
			ImGui::TableNextColumn();
			ImGui::Selectable(label, &selected[i]); // FIXME-TABLE: Selection overlap
		}
		ImGui::EndTable();
	}

	// list test
	ImGui::Separator();

	static int current_idx = 0;
	// extract indices from map
	std::vector<int> map_indices;
	for ( auto const& it : m_map)
		map_indices.push_back(it.first);
	
	if (ImGui::BeginListBox("##listbox 2", ImVec2(-FLT_MIN, 32 * ImGui::GetTextLineHeightWithSpacing())))
	{
		for (size_t i = 0; i < map_indices.size(); i++)
		{
			auto entry = m_map[map_indices[i]];
			char buffer[128];
			sprintf(buffer, "%zu --- %s", i, entry.c_str());
			const bool is_selected = (current_idx == i);
			//if (ImGui::Selectable(m_map[map_indices[i]].c_str(), is_selected))
			if (ImGui::Selectable(buffer, is_selected))
				current_idx = (int)i;
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndListBox();
	}

	ImGui::Separator();
	ImGui::Text("fps : %f", TimeStep::getFPS());
	ImGui::Text("idle time : %f", TimeStep::getIdleTime());

	// debug key input
	ImGui::Separator();
	ImGui::Text("key 'X'  : %s", InputManager::is_key_pressed(SYN_KEY_X) ? "true" : "false");
	ImGui::Text("key 'W'  : %s", InputManager::is_key_pressed(SYN_KEY_W) ? "true" : "false");
	ImGui::Text("L <CTRL> : %s", InputManager::is_key_pressed(SYN_KEY_LEFT_CONTROL) ? "true" : "false");
	ImGui::Text("R <CTRL> : %s", InputManager::is_key_pressed(SYN_KEY_RIGHT_CONTROL) ? "true" : "false");

	ImGui::End();
	ImGui::PopStyleVar();
	//-----------------------------------------------------------------------------------
	// set the 'rest' of the window as the viewport
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("synapse-core::renderer");
	static ImVec2 oldSize = { 0, 0 };
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	//printf("  vp %.0f x %.0f\n", viewportSize.x, viewportSize.y);
	//printf("o vp %.0f x %.0f\n", oldSize.x, oldSize.y);

	if (viewportSize.x != oldSize.x || viewportSize.y != oldSize.y)
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

