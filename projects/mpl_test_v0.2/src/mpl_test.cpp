
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


//
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

	Ref<Figure> m_figure = nullptr;
	std::string m_scatterID = "";
	std::string m_lineplotID = "";
	std::string m_histID = "";

	std::vector<std::vector<float>> m_data_X;
	std::vector<std::vector<float>> m_data_Y;
	int n = 500;
	
	// flags
	bool m_wireframeMode = true;
	bool m_toggleCulling = false;

	bool open_popup = true;
	bool add_scatter_data = false;
	bool add_lineplot_data = false;

};
class mpl_test_v02 : public Application
{
public:
	mpl_test_v02() { this->pushLayer(new layer); }
};
Application* CreateSynapseApplication() { return new mpl_test_v02(); }

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

	//-----------------------------------------------------------------------------------
	// FIGURE TESTS
	//-----------------------------------------------------------------------------------
	//
	glm::vec2 fig_sz = glm::vec2(480.0f, 320.0f);
	m_figure = MakeRef<Figure>(fig_sz);

	// SCATTER TEST ---------------------------------------------------------------------
	//
	std::default_random_engine gen(1); // seed 1 for reproducibility
	std::normal_distribution<float> dist(0.0, 5.0);

	std::vector<float> X(n), Y(n);
	for (int i = 0; i < n; i++)
	{
		X[i] = dist(gen);
		Y[i] = dist(gen);
	}
	//X = std::vector<float>({ 1, 0, 2 });
	//Y = std::vector<float>({ 2, 2, 2 });
	scatter_params_t scatter_params;
	scatter_params.marker_size = 4.0f;
	scatter_params.marker = FigureMarker::Square;
	//m_scatterID = m_figure->scatter(X, Y, "SCATTER", scatter_params);
	
	// LINEPLOT TEST --------------------------------------------------------------------
	//
	std::vector<float> y;
	for (int i = 0; i < 20; i++)
		y.push_back(dist(gen));

	for (int n_ = 0; n_ < 3; n_++)
	{
		std::vector<float> y;
		std::vector<float> x;
		for (int i = 0; i < n; i++)
		{
			y.push_back(dist(gen) + n_ * 100 - 100);
			x.push_back(static_cast<float>(i+n_*n));
		}
		m_data_Y.push_back(y);
		m_data_X.push_back(x);
	}
	lineplot_params_t lineplot_params;
	//lineplot_params.marker = FigureMarker::Square;
	//lineplot_params.marker_size = 2.0f;
	//m_lineplotID = m_figure->lineplot(m_data_X, m_data_Y, "TEST_LINE", lineplot_params);
	//m_lineplotID = m_figure->lineplot(m_data_Y, "TEST_LINE", lineplot_params);
	//m_lineplotID = m_figure->lineplot(y, "TEST_LINE", lineplot_params);
	
	// HISTOGRAM TEST -------------------------------------------------------------------
	//
	histogram_params_t hist_params;
	m_histID = m_figure->histogram(Y, "TEST_HIST", 10);



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
	
	static auto& renderer = Renderer::get();
	static float fontHeight = m_font->getFontHeight() + 1.0f;

	// handle input
	handleInput(_dt);


	// bind presenting framebuffer
	m_renderBuffer->bind();


	// toggle wireframe
	if (m_wireframeMode) renderer.enableWireFrame();

	// clear the screen
	renderer.setClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
	renderer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// -- BEGINNING OF SCENE -- //
	// -- END OF SCENE -- //


	// toggle wireframe (back)
	if (m_wireframeMode) renderer.disableWireFrame();

	
	// Text rendering 
	// TODO: all text rendering should go into an overlay layer.


	// ...and we're done! hand-off to ImGui to render the texture (scene) in the viewport pane.
	m_renderBuffer->bindDefaultFramebuffer();

	if (add_scatter_data)
	{
		std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
		float mu = 0.0f;
		float sigma = 10.0f;
		float offset_x = Random::rand_f_r(-100.0f, 100.0f);
		float offset_y = Random::rand_f_r(-100.0f, 100.0f);
		std::normal_distribution<float> dist(mu, sigma);
		std::vector<float> X(n), Y(n);
		for (int i = 0; i < n; i++)
		{
			X[i] = dist(gen) + offset_x;
			Y[i] = dist(gen) + offset_y;
		}
		scatter_params_t params;
		params.marker_size = 4.0f;
		params.marker = FigureMarker::Square;
		params.marker_color = glm::vec4(Random::rand3_f_r(), 0.5f);
		params.marker = FigureMarker(Random::rand_i_r(0, 7));
		std::string ID(Random::rand_str(16));
		m_figure->scatter(X, Y, ID, params);
		
		add_scatter_data = false;
	}

	if (add_lineplot_data)
	{
		std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
		float mu = 0.0f;
		float sigma = 10.0f;
		std::normal_distribution<float> dist(mu, sigma);
		std::vector<float> y;
		for (int i = 0; i < n; i++)
			y.push_back(dist(gen));
		m_figure->canvas("TEST_LINE")->data(y);

		add_lineplot_data = false;
	}

	if (m_figure != nullptr)
	{
		//Timer timer("update and render");
		m_figure->render();
	}

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
		if (m_figure)
			ImGui::Image((void*)m_figure->framebufferTexturePtr(), m_figure->size(), { 0, 1 }, { 1, 0 });

		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
			open_popup = false;
		}
		ImGui::SameLine();
		/*
		if (ImGui::Button("Rand scatter"))
		{
			std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
			std::normal_distribution<float> dist(5.0, 2.0);
			std::vector<float> X(n), Y(n);
			for (int i = 0; i < n; i++)
			{
				X[i] = dist(gen);
				Y[i] = dist(gen);
			}
			m_figure->canvas("TEST_SCATTER")->data(X, Y);
		}
		
		if (ImGui::Button("Add scatter data"))
		{
			add_scatter_data = true;
		}
		if (ImGui::Button("Rand line"))
		{
			std::default_random_engine gen(std::chrono::system_clock::now().time_since_epoch().count());
			std::normal_distribution<float> dist(5.0, 2.0);
			std::vector<float> Y;
			for (int i = 0; i < n; i++)
				Y.push_back(dist(gen));
			m_figure->canvas("TEST_LINE")->data(Y);

		}
		if (ImGui::Button("Add line data"))
		{
			add_lineplot_data = true;
		}
		*/		

		ImGui::Text("");
		ImGui::Separator();
		ImGui::Text("data points : %zu", m_figure->dataSize());

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

