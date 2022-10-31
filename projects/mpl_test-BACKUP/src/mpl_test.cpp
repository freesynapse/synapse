
#include <imgui/imgui.h>
#include <Synapse.hpp>
#include <Synapse/SynapseMain.hpp>

#include <map>
#include <chrono>

#include "imgui/imgui_internal.h"

#include "Figure/Figure.h"


using namespace Syn;
using namespace Syn::mplc;

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

	Ref<Shader> m_shader = nullptr;

	Ref<Histogram> m_histogramFloat = nullptr;
	Ref<Histogram> m_histogramInt = nullptr;
	Ref<LinePlot> m_linePlot = nullptr;
	Ref<ScatterPlot> m_scatterPlot = nullptr;
	
	// flags
	bool m_wireframeMode = true;
	bool m_toggleCulling = false;

	bool open_popup = true;

};
class imgui_188_test : public Application
{
public:
	imgui_188_test() { this->pushLayer(new layer); }
};
Application* CreateSynapseApplication() { return new imgui_188_test(); }

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

	// HISTOGRAM
	glm::vec2 canvas_sz = glm::vec2(480.0f, 320.0f);

	std::default_random_engine gen(1); // seed 1 for reproducibility
	std::normal_distribution<float> dist(10.0, 2.0);
	int n = 5000;

	std::vector<int> idata;
	for (int i = 0; i < n; i++)
		idata.push_back(static_cast<int>(dist(gen)));
	m_histogramInt = MakeRef<Histogram>(idata, 0, canvas_sz);
	m_histogramInt->title("int_histogram");
	m_histogramInt->interactiveMode(true);

	std::default_random_engine gen2(std::chrono::system_clock::now().time_since_epoch().count());
	std::normal_distribution<float> dist2(0, 5.0);
	std::vector<float> data;
	for (int i = 0; i < n; i++)
		data.push_back(dist2(gen2));
	//m_histogramFloat = MakeRef<Histogram>(data, 0, canvas_sz);
	//m_histogramFloat->title("float_histogram");
	//m_histogramFloat->interactiveMode(true);
	
	// LINE PLOT
	//data = { -10.0f, 0.0f, 0.0f, 10.0f };
	//m_linePlot->data(data);
	//std::vector<float> X = { 1, 0, 2, 1, 3, 7 };
	//std::vector<float> Y = { 1, 0, 3, 4, 5, 7 };
	//m_linePlot->data(X, Y);
	m_linePlot = newLinePlot(canvas_sz);
	//Nästa steg är att testa att testa ->data(std::vector<std::vector<float>>&)-funktionen, redan implemnterat alfa-mask i shadern för line plots
	std::vector<std::vector<float>> data2 = {
		{ 0, 1, 2, 3, 4, 5, 6 },
		{ 6, 5, 4, 3, 2 },
		{ 10, 12, 7, 5, 10 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 20, 1, 20, 10, 20, 1, 20, 10, 20, 1, 20 },
	};
	m_linePlot->data(data2);

	// SCATTER PLOT
	data.clear();
	for (float i = 0.0f; i < 10.0f; i+=1.0f)
		data.push_back(i);
	m_scatterPlot = newScatterPlot(data, canvas_sz);


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


	// bind presenting framebuffer
	m_renderBuffer->bind();


	// toggle wireframe
	if (m_wireframeMode) Renderer::enableWireFrame();

	// clear the screen
	Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// -- BEGINNING OF SCENE -- //
	// -- END OF SCENE -- //


	// toggle wireframe (back)
	if (m_wireframeMode) Renderer::disableWireFrame();

	
	// Text rendering 
	// TODO: all text rendering should go into an overlay layer.


	// ...and we're done! hand-off to ImGui to render the texture (scene) in the viewport pane.
	m_renderBuffer->bindDefaultFramebuffer();


	if (m_histogramInt != nullptr)
		m_histogramInt->render();
	if (m_histogramFloat != nullptr)
		m_histogramFloat->render();
	if (m_linePlot != nullptr)
	{
		//static int n = 1000;
		//std::normal_distribution<float> dist(10.0f, 2.0f);
		//std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
		//std::vector<float> data;
		//for (int i = 0; i < n; i++)
		//	data.push_back(dist(gen));
		//m_linePlot->data(data);

		m_linePlot->render();
	}
	if (m_scatterPlot != nullptr)
		m_scatterPlot->render();

}
//---------------------------------------------------------------------------------------
void layer::popup_test()
{
	static ImGuiIO& io = ImGui::GetIO();
	static ImVec2 size = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
	static int n = 1000;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
	ImGui::SetNextWindowPos(size, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(500.0f, 850.0f), ImGuiCond_Appearing);

	ImGui::OpenPopup("modal_popup");
	if (ImGui::BeginPopupModal("modal_popup", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		//if (m_histogramInt)
		//	ImGui::Image((void*)m_histogramInt->figurePtrID(), m_histogramInt->figureSz(), { 0, 1 }, { 1, 0 });
		//
		//if (m_histogramFloat)
		//	ImGui::Image((void*)m_histogramFloat->figurePtrID(), m_histogramFloat->figureSz(), { 0, 1 }, { 1, 0 });
		
		if (m_linePlot)
			ImGui::Image((void*)m_linePlot->figurePtrID(), m_linePlot->figureSz(), { 0, 1 }, { 1, 0 });

		if (m_scatterPlot)
			ImGui::Image((void*)m_scatterPlot->figurePtrID(), m_scatterPlot->figureSz(), { 0, 1 }, { 1, 0 });
		//if (m_histogramInt)
		//{
		//	int imin = static_cast<int>(m_histogramInt->minBin());
		//	int imax = static_cast<int>(m_histogramInt->maxBin());
		//	int lo = static_cast<int>(m_histogramInt->fillLimX0());
		//	int hi = static_cast<int>(m_histogramInt->fillLimX1());
		//	ImGui::Text("Low "); ImGui::SameLine(); 
		//	ImGui::SliderInt("##lo_int", &lo, imin, imax); ImGui::SameLine();
		//	if (ImGui::Button("-##lo_int")) lo--; ImGui::SameLine();
		//	if (ImGui::Button("+##lo_int")) lo++;			
		//	ImGui::Text("High"); ImGui::SameLine(); 
		//	ImGui::SliderInt("##hi_int", &hi, imin, imax); ImGui::SameLine();
		//	if (ImGui::Button("-##hi_int")) hi--; ImGui::SameLine();
		//	if (ImGui::Button("+##hi_int")) hi++;
		//	m_histogramInt->fillBetweenX(static_cast<float>(lo), static_cast<float>(hi));
		//	
		//	ImGui::Separator();
		//}

		/*
		if (m_histogramFloat)
		{
			float fmin = m_histogramFloat->minBin();
			float fmax = m_histogramFloat->maxBin();
			ImGui::Text("Low "); ImGui::SameLine(); 
			ImGui::SliderFloat("##lo_float", &m_histogramFloat->fillLimX0(), fmin, fmax); ImGui::SameLine();
			if (ImGui::Button("-##lo_float")) m_histogramFloat->fillLimX0()--; ImGui::SameLine();
			if (ImGui::Button("+##lo_float")) m_histogramFloat->fillLimX0()++;
			ImGui::Text("High"); ImGui::SameLine(); 
			ImGui::SliderFloat("##hi_float", &m_histogramFloat->fillLimX1(), fmin, fmax); ImGui::SameLine();
			if (ImGui::Button("-##hi_float")) m_histogramFloat->fillLimX1()--; ImGui::SameLine();
			if (ImGui::Button("+##hi_float")) m_histogramFloat->fillLimX1()++;
			m_histogramFloat->fillBetweenX();
		}
		*/

		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
			open_popup = false;
		}
		ImGui::SameLine();

		if (ImGui::Button("histogramInt") && m_histogramInt)
		{
			std::normal_distribution<float> dist(10.0f, 2.0f);
			std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
			std::vector<int> data;
			for (int i = 0; i < n; i++)
				data.push_back(static_cast<int>(dist(gen)));
			
			m_histogramInt->data(data, 0);
		}
		/*
		ImGui::SameLine();
		if (ImGui::Button("histogramFloat") && m_histogramFloat)
		{
			std::normal_distribution<float> dist(10.0f, 2.0f);
			std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
			std::vector<float> data;
			for (int i = 0; i < n; i++)
				data.push_back(dist(gen));
			m_histogramFloat->data(data, 0);
		}
		*/
		ImGui::SameLine();
		if (ImGui::Button("linePlot") && m_linePlot)
		{
			std::normal_distribution<float> dist(10.0f, 2.0f);
			std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
			std::vector<float> data;
			for (int i = 0; i < n; i++)
				data.push_back(dist(gen));
			int i = Random::rand_i_r(0, n);
			data[i] = Random::rand_f_r(-100.0f, 100.0f);
			m_linePlot->data(data);
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
			//m_usePerspectiveCamera = !m_usePerspectiveCamera;
			//if (m_usePerspectiveCamera)
			//	m_perspectiveCamera->resetMousePosition();
			//EventHandler::push_event(new WindowToggleFrozenCursorEvent(m_usePerspectiveCamera));
			//EventHandler::push_event(new WindowToggleCursorEvent(!m_usePerspectiveCamera));
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
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("settings");

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

