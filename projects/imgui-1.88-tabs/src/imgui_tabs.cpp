
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
	
	std::string m_message = "";
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

	// bind presenting framebuffer
	m_renderBuffer->bind();

	// clear the screen
	Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Text rendering 
	int i = 1;
	m_font->beginRenderBlock();
	m_font->addString(2.0f, fontHeight * i++, "fps=%.0f  VSYNC=%s", 
						TimeStep::getFPS(), 
						Application::get().getWindow().isVSYNCenabled() ? "ON" : "OFF");
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

		case SYN_KEY_F6:
			break;

		case SYN_KEY_F7:
			break;

		case SYN_KEY_F8:
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

	ImGuiID main_dockspace_id = ImGui::GetID("dockspace");
	ImGui::DockSpace(main_dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);

	auto node = ImGui::DockBuilderGetNode(main_dockspace_id);
	if (node == nullptr) {
		ImGui::DockBuilderAddNode(main_dockspace_id);

		const auto size = ImVec2(520.0f, 600.0f);
		ImGui::DockBuilderSetNodeSize(main_dockspace_id, size);

		// Center the window on the screen
		const auto viewport = ImGui::GetMainViewport();
		const auto pos = ImVec2(
			std::max(0.0f, (viewport->WorkSize.x - size.x) / 2.0f),
			std::max(0.0f, (viewport->WorkSize.y - size.y) / 2.0f));
		ImGui::DockBuilderSetNodePos(main_dockspace_id, pos);

		// Split the dockspace into 2 nodes
		ImGuiID dockspaceA = 0;
		ImGuiID dockspaceB = 0;
		ImGui::DockBuilderSplitNode(main_dockspace_id, ImGuiDir_Down, 0.60f, &dockspaceB, &dockspaceA);

		// Dock our windows into the docking nodes we made above
		ImGui::DockBuilderDockWindow("top", dockspaceA);
		ImGui::DockBuilderDockWindow("bot_tab0", dockspaceB);
		ImGui::DockBuilderDockWindow("bot_tab1", dockspaceB);

		ImGui::DockBuilderFinish(main_dockspace_id);
	}
	// __debug : setup of 'tabs' which actually are dockspace nodes
	/*
	const auto vp = ImGui::GetMainViewport();
	auto node = ImGui::DockBuilderGetNode(dockspace_id);
	SYN_CORE_ASSERT(node != nullptr);

	ImGuiID win_render = 0;
	ImGuiID win_settings = 0;

	ImVec2 sz_settings = ImVec2(395, 1031);
	
	ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.8f, &win_render, &win_settings);
	ImGui::DockBuilderDockWindow("synapse-core::renderer", win_render);
	ImGui::DockBuilderDockWindow("settings", win_settings);

	ImGui::DockBuilderFinish(dockspace_id);
	*/
	// end __debug

	//-----------------------------------------------------------------------------------
	/*
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("synapse-core", &p_open, window_flags);	// root
	ImGui::PopStyleVar();

	*/
	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	//-----------------------------------------------------------------------------------
	static bool top = true;
	static bool bot_tab0 = true;
	static bool bot_tab1 = true;

	// top 
	if (top)
	{
		ImGui::Begin("top", &top);

		ImGui::Text("fps : %f", TimeStep::getFPS());
		ImGui::Text("idle time : %f", TimeStep::getIdleTime());

		ImGui::End();
	}
	// bot tab 0
	if (bot_tab0)
	{
		ImGui::Begin("bot_tab0", &bot_tab0);
		ImGui::Text("tab 0");
		ImGui::End();
	}
	// bot tab 1
	if (bot_tab1)
	{
		ImGui::Begin("bot_tab1", &bot_tab1);
		ImGui::Text("tab 1");
		ImGui::End();
	}

	//-----------------------------------------------------------------------------------
	// set the 'rest' of the window as the viewport
	/*
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
	*/

	// end root
	/*
	ImGui::End();
	*/
}
//---------------------------------------------------------------------------------------
void layer::handleInput(float _dt)
{
}

