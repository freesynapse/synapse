
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

class class_ex
{
public:
    class_ex() { func1(); }

private:
    void func1() { func2(); }
    void func2() { func3(); }
    void func3() { func4(); }
    void func4() { crash_func(); }
    void crash_func() { char * p = NULL; *p = 0; }
};

int crash_func()    { char * p = NULL; *p = 0; return 0; }
int func4()         { crash_func(); return 0;            }
int func3()         { func4(); return 0;                 }
int func2()         { func3(); return 0;                 }
int func1()         { func2(); return 0;                 }


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
	glm::vec2 fig_sz = glm::vec2(480.0f, 320.0f);
	m_figure = MakeRef<Figure>(fig_sz);

	std::default_random_engine gen(1); // seed 1 for reproducibility
	std::normal_distribution<float> dist(10.0, 2.0);
	int n = 5000;

	
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


	if (m_figure != nullptr)
		m_figure->render();

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
		if (m_figure)
			ImGui::Image((void*)m_figure->figurePtrID(), m_figure->figureSz(), { 0, 1 }, { 1, 0 });


		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
			open_popup = false;
		}
		ImGui::SameLine();

		if (ImGui::Button("CRASH_1"))
		{
			func1();			
		}
		ImGui::SameLine();

		if (ImGui::Button("CRASH_2"))
		{
			class_ex c;
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

