
#include <imgui/imgui.h>
#include <Synapse.hpp>
#include <Synapse/SynapseMain.hpp>

#include <map>

#include "imgui/imgui_internal.h"

#include "Histogram.hpp"


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

	void popup_test();

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

	Ref<Histogram<float>> m_histogramFloat = nullptr;
	Ref<Histogram<int>> m_histogramInt = nullptr;
	
	std::string m_message = "";

	bool open_popup = true;

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
	m_map = { { 0, "aaa" }, { 2, "bbb" }, { 4, "ccc" }, { 6, "ddd" }, { 8, "eee" }, { 10, "fff" }, { 12, "ggg" }, { 14, "hhh" } };

	// histogram
	glm::vec2 canvas_sz = glm::vec2(480.0f, 320.0f);
	std::default_random_engine gen(1); // seed 1 for reproducibility
	std::normal_distribution<float> dist(10.0, 2.0);
	std::vector<float> data;
	int n = 5000;
	for (int i = 0; i < n; i++)
		data.push_back(dist(gen));
	m_histogramFloat = MakeRef<Histogram<float>>(data, 0, canvas_sz);
	m_histogramFloat->title("float_histogram");
	m_histogramFloat->interactiveMode(true);

	// Int histogram
	std::map<int, size_t> bins = { { 30, 10 }, { 31, 19 }, { 32, 77 }, { 33, 450 }, { 34, 161 }, { 35, 59 }, { 36, 31 }, { 37, 23 }, { 38, 10 } };
	m_histogramInt = MakeRef<Histogram<int>>(canvas_sz);
	m_histogramInt->title("int_histogram");
	m_histogramInt->__debug_set_bins(bins);
	m_histogramInt->interactiveMode(true);

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
	
	static float fontHeight = m_font->getFontHeight() + 1.0f;

	// handle input
	handleInput(_dt);


	// update camera
	if (m_usePerspectiveCamera)
	{
		m_perspectiveCamera->setUpdateMode(m_usePerspectiveCamera);
		if (!open_popup)
			m_orbitCamera->onUpdate(_dt);
		m_perspectiveCamera->onUpdate(_dt);
		m_activeCamera = m_perspectiveCamera;
	}
	else
	{
		m_orbitCamera->setUpdateMode(m_usePerspectiveCamera);
		if (!open_popup)
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


	if (m_histogramInt != nullptr)
		m_histogramInt->render();
	if (m_histogramFloat != nullptr)
		m_histogramFloat->render();

}
//---------------------------------------------------------------------------------------
void layer::popup_test()
{
	static ImGuiIO& io = ImGui::GetIO();
	static ImVec2 size = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
	ImGui::SetNextWindowPos(size, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(500.0f, 850.0f), ImGuiCond_Appearing);

	ImGui::OpenPopup("modal_popup");
	if (ImGui::BeginPopupModal("modal_popup", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		if (m_histogramInt)
			ImGui::Image((void*)m_histogramInt->getCanvas(), m_histogramInt->getCanvasSz(), { 0, 1 }, { 1, 0 });
		if (m_histogramFloat)
			ImGui::Image((void*)m_histogramFloat->getCanvas(), m_histogramFloat->getCanvasSz(), { 0, 1 }, { 1, 0 });
		
		if (m_histogramInt)
		{
			int imin = m_histogramInt->minBin();
			int imax = m_histogramInt->maxBin();
			ImGui::Text("Low "); ImGui::SameLine(); 
			ImGui::SliderInt("##lo_int", &m_histogramInt->intervalLo(), imin, imax); ImGui::SameLine();
			if (ImGui::Button("-##lo_int")) m_histogramInt->intervalLo()--; ImGui::SameLine();
			if (ImGui::Button("+##lo_int")) m_histogramInt->intervalLo()++;
			ImGui::Text("High"); ImGui::SameLine(); 
			ImGui::SliderInt("##hi_int", &m_histogramInt->intervalHi(), imin, imax); ImGui::SameLine();
			if (ImGui::Button("-##hi_int")) m_histogramInt->intervalHi()--; ImGui::SameLine();
			if (ImGui::Button("+##hi_int")) m_histogramInt->intervalHi()++;
			m_histogramInt->updateInterval();
			
			ImGui::Separator();
		}

		if (m_histogramFloat)
		{
			float fmin = m_histogramFloat->minBin();
			float fmax = m_histogramFloat->maxBin();
			ImGui::Text("Low "); ImGui::SameLine(); 
			ImGui::SliderFloat("##lo_float", &m_histogramFloat->intervalLo(), fmin, fmax); ImGui::SameLine();
			if (ImGui::Button("-##lo_float")) m_histogramFloat->intervalLo()--; ImGui::SameLine();
			if (ImGui::Button("+##lo_float")) m_histogramFloat->intervalLo()++;
			ImGui::Text("High"); ImGui::SameLine(); 
			ImGui::SliderFloat("##hi_float", &m_histogramFloat->intervalHi(), fmin, fmax); ImGui::SameLine();
			if (ImGui::Button("-##hi_float")) m_histogramFloat->intervalHi()--; ImGui::SameLine();
			if (ImGui::Button("+##hi_float")) m_histogramFloat->intervalHi()++;
			m_histogramFloat->updateInterval();
		}
		
		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
			open_popup = false;
		}
		ImGui::SameLine();

		if (ImGui::Button("TEST INT CHANGE DATA"))
		{
			std::vector<int> data;
			for (int i = 0; i < 4000; i++)
				data.push_back(Random::rand_i_r(15, 40));

			if (m_histogramInt != nullptr)
				m_histogramInt->data(data, 0);

		}
		ImGui::SameLine();
		if (ImGui::Button("TEST FLOAT CHANGE DATA"))
		{
			std::vector<float> data;
			for (int i = 0; i < 4000; i++)
				data.push_back(Random::rand_f_r(15, 40));

			if (m_histogramFloat != nullptr)
				m_histogramFloat->data(data, 0);

		}


		ImGui::EndPopup();

	}
	ImGui::PopStyleVar();

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
		
		case SYN_KEY_SPACE:
			open_popup = !open_popup;
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
	ImGuiID dockspace_id = 0;	//
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		dockspace_id = ImGui::GetID("dockspace");
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

	// TEST ImGui::BeginPopupModal()
	//
	ImGui::Separator();
	ImGui::Text("open_popup : %s", open_popup ? "true" : "false");
	if (ImGui::Button("Open popup"))
		open_popup = true;

	if (open_popup)
	{	
		popup_test();
	}
	//
	
	ImGui::End();
	ImGui::PopStyleVar();

	//-----------------------------------------------------------------------------------
	// set the 'rest' of the window as the viewport
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("synapse-core::renderer");
	static ImVec2 oldSize = { 0, 0 };
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

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

