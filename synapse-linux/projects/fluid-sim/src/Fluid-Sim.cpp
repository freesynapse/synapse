
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>

#include <Synapse.hpp>
#include <Synapse/SynapseMain.hpp>


#include "Fluid.hpp"


//
class FluidLayer : public Syn::Layer
{
public:
	FluidLayer() : Layer("FluidLayer") {}
	virtual ~FluidLayer() {}

	virtual void onAttach() override;
	virtual void onUpdate(float _dt) override;
	virtual void onImGuiRender() override;
	void onKeyDownEvent(Syn::Event* _e);
	void onMouseButtonEvent(Syn::Event* _e);
	void handleInput(float _dt);


public:
	// standard objects
	Syn::Ref<Syn::Font> m_font = nullptr;
	Syn::Ref<Syn::Framebuffer> m_fboRender = nullptr;
	
	Syn::Ref<Syn::OrthographicCamera> m_camera = nullptr;
	bool m_bCameraMode = false;	// if true, starts in camera move mode, contrasted to edit mode
	
	Syn::Ref<Syn::Texture2D> m_texture = nullptr;
	Syn::Ref<Syn::Texture2D> m_whiteTexture = nullptr;
	Syn::Ref<Syn::Shader> m_shader2D = nullptr;

	Syn::Ref<Syn::Shader> m_screenShader = nullptr;
	Syn::Ref<Syn::MeshShape> m_screenQuad = nullptr;
	Syn::Ref<Syn::Texture2D> m_screenTexture = nullptr;

	Syn::Ref<Syn::Framebuffer> m_surface = nullptr;

	Syn::Ref<Syn::Fluid> m_fluid = nullptr;

	// flags
	bool m_wireframeMode = false;
	bool m_toggleCulling = true;

};
class FluidSim : public Syn::Application
{
public:
	FluidSim() { this->pushLayer(new FluidLayer); }
};
Syn::Application* CreateSynapseApplication() { return new FluidSim(); }


//---------------------------------------------------------------------------------------
void FluidLayer::onAttach()
{
	SYN_PROFILE_FUNCTION();

	// register event callback

	Syn::EventHandler::register_callback(Syn::EventType::INPUT_KEY, SYN_EVENT_MEMBER_FNC(FluidLayer::onKeyDownEvent));
	Syn::EventHandler::register_callback(Syn::EventType::INPUT_MOUSE_BUTTON, SYN_EVENT_MEMBER_FNC(FluidLayer::onMouseButtonEvent));


	// load shaders

	Syn::ShaderLibrary::load("../assets/shaders/debugShader.glsl");
	Syn::ShaderLibrary::load("../assets/shaders/debugPointShader.glsl");
	Syn::ShaderLibrary::load("../assets/shaders/simple2DShader.glsl");
	m_shader2D = Syn::ShaderLibrary::get("simple2DShader");


	// load font

	//m_font = Syn::MakeRef<Syn::Font>("../assets/ttf/ubuntu.mono.ttf", 16.0f);
	m_font = Syn::API::newFont("../assets/ttf/ubuntu.mono.ttf", 16.0f);
	//m_font = SYN_NEW_FONT_REF("../assets/ttf/ubuntu.mono.ttf", 18.0f, nullptr);
	m_font->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));


	// Fluid surfaces initialization
	//

	// shader
	Syn::ShaderLibrary::load("../assets/shaders/screenCoordShader.glsl");
	m_screenShader = Syn::ShaderLibrary::get("screenCoordShader");
	// texture
	m_texture = Syn::API::newTexture2D("../assets/textures/uv_grid.jpg");
	// load camera
	// initialization with aspect ratio and zoom level (default = 1.0f)
	m_camera = Syn::API::newOrthographicCamera(SCREEN_WIDTH_F / SCREEN_WIDTH_F, 1.0f);
	// set camera to edit mode
	Syn::EventHandler::push_event(new Syn::WindowToggleFrozenCursorEvent(m_bCameraMode));
	Syn::EventHandler::push_event(new Syn::WindowToggleCursorEvent(!m_bCameraMode));
	// test fbo setup
	m_surface = Syn::API::newFramebuffer(Syn::ColorFormat::RGBA8, glm::ivec2(256, 256), true);
	// whole screen quad, used to show rendering target framebuffer.
	m_screenQuad = Syn::MeshCreator::createShapeViewportQuad();
	
	Syn::Renderer2D::init();

	// initialize the fluid class
	const uint32_t w = 1536;	// ImGUI viewport width/height
	const uint32_t h = 1009;
	m_fluid = Syn::MakeRef<Syn::Fluid>(w, h);


	// framebuffer
	// the final, rendered scene framebuffer, for hand-off to ImGui for rendering
	m_fboRender = Syn::API::newFramebuffer(Syn::ColorFormat::RGBA16F);


	// execute pending rendering commands before rendering
	//
	Syn::Renderer::get().executeRenderCommands();

}

//---------------------------------------------------------------------------------------
void FluidLayer::onUpdate(float _dt)
{
	SYN_PROFILE_FUNCTION();
	
	static float fontHeight = (float)m_font->getFontHeight() + 1.0f;


	// handle input
	handleInput(_dt);

	// update camera
	//m_camera->setUpdateMode(m_bCameraMode);
	//m_camera->onUpdate(_dt);

	// bind presenting framebuffer
	//Renderer::enableGLenum(GL_BLEND);
	//m_screenShader->enable();
	m_fboRender->bind();

	// toggle wireframe
	if (m_wireframeMode) Syn::Renderer::enableWireFrame();

	// clear the screen
	m_fboRender->clear(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	// -- BEGINNING OF SCENE -- //
	Syn::Renderer2D::beginScene(m_camera);
	{
		// Fluid 
		//

		// update state -- compute the next time step
		m_fluid->step(_dt);

		m_fboRender->bind();
		// render Fluid
		float w = (float)m_fboRender->getWidth();
		float h = (float)m_fboRender->getHeight();
		m_fluid->render(_dt, w, h);
		

	}
	Syn::Renderer2D::endScene();
	// -- END OF SCENE -- //

	// toggle wireframe (back)
	if (m_wireframeMode) Syn::Renderer::disableWireFrame();

	// Text rendering 
	// TODO: all text rendering should go into an overlay layer.
	m_font->beginRenderBlock();
	m_font->addString(2.0f, fontHeight * 1, "fps=%.0f  VSYNC=%s", Syn::TimeStep::getFPS(), Syn::Application::get().getWindow().isVSYNCenabled() ? "ON" : "OFF");
	
	m_font->addString(2.0f, fontHeight * 2, "process memory:");
	m_font->addString(2.0f, fontHeight * 3, "    virtual=%d kB", Syn::ProcessInfo::getSizeVirtual_kB());
	m_font->addString(2.0f, fontHeight * 4, "    RSS=%d kB", Syn::ProcessInfo::getSizeRSS_kB());
	m_font->addString(2.0f, fontHeight * 5, "    shared=%d kB", Syn::ProcessInfo::getSizeShared_kB());

	
	//glm::vec3 camPos = m_camera->getPosition();
	//float camTheta = m_camera->getTheta();
	//m_font->addString(2.0f, fontHeight * 2, "camera [ %.1f  %.1f ], theta [ %.1f ]", camPos.x, camPos.y, camTheta);
	
	m_font->endRenderBlock();

	// ...and we're done! hand-off to ImGui to render the texture (scene) in the viewport pane.
	m_fboRender->bindDefaultFramebuffer();

	//Syn::EventHandler::push_event(new Syn::WindowCloseEvent());
}


//---------------------------------------------------------------------------------------
void FluidLayer::onKeyDownEvent(Syn::Event* _e)
{
	SYN_PROFILE_FUNCTION();

	Syn::KeyDownEvent* e = dynamic_cast<Syn::KeyDownEvent*>(_e);

	static bool vsync = true;

	if (e->getAction() == GLFW_PRESS)
	{
		switch (e->getKey())
		{
		case SYN_KEY_R:
			break;

		case SYN_KEY_Z:
			vsync = !vsync;
			Syn::Application::get().getWindow().setVSYNC(vsync);
			break;

		case SYN_KEY_X:
			break;

		case SYN_KEY_C:
			break;

		case SYN_KEY_B:
			break;

		case SYN_KEY_V:
			m_fboRender->saveAsPNG();
			break;

		case SYN_KEY_ESCAPE:
			Syn::EventHandler::push_event(new Syn::WindowCloseEvent());
			break;

		case SYN_KEY_F4:
			m_wireframeMode = !m_wireframeMode;
			break;

		case SYN_KEY_F5:
			m_toggleCulling = !m_toggleCulling;
			Syn::Renderer::setCulling(m_toggleCulling);
			break;

		case SYN_KEY_F6:
			break;

		case SYN_KEY_F7:
			break;

		case SYN_KEY_F8:
			break;

		case SYN_KEY_TAB:
			m_bCameraMode = !m_bCameraMode;
			Syn::EventHandler::push_event(new Syn::WindowToggleFrozenCursorEvent(m_bCameraMode));
			Syn::EventHandler::push_event(new Syn::WindowToggleCursorEvent(!m_bCameraMode));
			// if entering camera mode again, adjust cursor position
			//if (m_fpsCameraMode)
			//{
			//	glm::vec2 half_vp = Syn::Renderer::getViewportF() * 0.5f;
			//	Syn::Application::get().getWindow().setCursorPos(glm::vec2(floor(half_vp.x), floor(half_vp.y)));
			//	m_cameraPerspective->resetMousePosition();
			//}
			break;
		}
	}

}


//---------------------------------------------------------------------------------------
void FluidLayer::onMouseButtonEvent(Syn::Event* _e)
{
}


//---------------------------------------------------------------------------------------
void FluidLayer::onImGuiRender()
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
	ImGuiTextBuffer buffer = Syn::Log::getImGuiBuffer();
	const char* bufBegin = buffer.begin();
	const char* bufEnd = buffer.end();

	// only process lines within the visible area
	ImGuiListClipper clipper;
	ImVector<int> lineOffsets = Syn::Log::getImGuiLineOffset();
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
	ImGui::Begin("synapse-core::Renderer");
	static ImVec2 oldSize;
	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	if (viewportSize.x != oldSize.x && viewportSize.y != oldSize.y)
	{
		// dispatch a viewport resize event -- registered classes will receive this, including:
		//  - camera
		//	- font renderer
		//	- 3D renderer
		//	- framebuffer instances
		Syn::EventHandler::push_event(new Syn::ViewportResizeEvent(glm::vec2(viewportSize.x, viewportSize.y)));
		SYN_CORE_TRACE("viewport [ ", viewportSize.x, ", ", viewportSize.y, " ]");
		oldSize = viewportSize;
	}

	// direct ImGui to the framebuffer texture
	ImGui::Image((void*)m_fboRender->getColorAttachmentIDn(0), viewportSize, { 0, 1 }, { 1, 0 });

	ImGui::End();
	ImGui::PopStyleVar();


	// end root
	ImGui::End();

}


//---------------------------------------------------------------------------------------
void FluidLayer::handleInput(float _dt)
{
}

