
#include <imgui/imgui.h>
#include <Synapse.hpp>
#include <Synapse/SynapseMain.hpp>
#include <Synapse/Utils/MathUtils.hpp>

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
	void onMouseScrollEvent(Event* _e);
	void onApplicationExit(Event* _e);

	void asyncInput();
	void zoom(float _x);
	void detectVisibleVertices();

	inline float worldToScreenX(float _wx) { return 2.0 * ((_wx - m_offset.x) * m_scale.x) - 1.0; }
	inline float screenToWorldX(float _sx) { return (((_sx + 1.0f) * 0.5f) / m_scale.x) + m_offset.x; }

public:
	// standard objects
	Ref<Font> m_font = nullptr;
	Ref<Framebuffer> m_renderBuffer = nullptr;
	
	Ref<Shader> m_shader = nullptr;
	glm::vec2* m_vertices = nullptr;
	int32_t data_dim = 0;
	int32_t m_vertexCount = 0;
	int32_t m_vertexOffset = 0;
	Ref<VertexArray> m_vao = nullptr;

	// zoom and pan
	float mx_ndc 	   = 0.0f;
	float mx_pre_zoom  = 0.0f;
	float mx_post_zoom = 0.0f;
	float delta_zoom   = 0.0f;
	glm::vec2 m_offset = { 0.0f, 0.0f };
	glm::vec2 m_scale  = { 1.0f, 1.0f };

	int32_t vertex_idx0 = 0;
	int32_t vertex_idx1 = 0;
	float vertex_idx0_ndc = 0.0f;
	float vertex_idx1_ndc = 0.0f;

	glm::vec2 prev_mpos;
	glm::vec2 curr_mpos;
	glm::vec2 vp;


};
class pan_and_zoom : public Application
{
public:
	pan_and_zoom() { this->pushLayer(new layer); }
};
Application* CreateSynapseApplication() { return new pan_and_zoom(); }

//---------------------------------------------------------------------------------------
void layer::onAttach()
{
	SYN_PROFILE_FUNCTION();

	// register event callback

	EventHandler::register_callback(EventType::INPUT_KEY, SYN_EVENT_MEMBER_FNC(layer::onKeyDownEvent));
	EventHandler::register_callback(EventType::INPUT_MOUSE_BUTTON, SYN_EVENT_MEMBER_FNC(layer::onMouseButtonEvent));
	EventHandler::register_callback(EventType::INPUT_MOUSE_SCROLL, SYN_EVENT_MEMBER_FNC(layer::onMouseScrollEvent));
	EventHandler::register_callback(EventType::APPLICATION_EXIT, SYN_EVENT_MEMBER_FNC(layer::onApplicationExit));
	EventHandler::push_event(new WindowToggleFullscreenEvent());
	// load font
	m_font = MakeRef<Font>("../assets/ttf/JetBrains/JetBrainsMono-Medium.ttf", 14.0f);
	m_font->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	// shader source
	std::string src = R"(
		#type VERTEX_SHADER
		#version 430 core
		layout(location=0) in vec2 a_position;
		
		uniform float u_offset_x = 0.0;
		uniform float u_scale_x = 1.0;
		
		float world_to_NDC(float _p)
		{
			float ndc = 2.0 * ((_p - u_offset_x) * u_scale_x) - 1.0;
			return ndc;
		}

		void main()
		{
			vec2 p = vec2(world_to_NDC(a_position.x), a_position.y);
			gl_Position = vec4(p, 1.0, 1.0);
		}
		#type FRAGMENT_SHADER
		#version 430 core
		out vec4 frag_color;
		void main()
		{
			frag_color = vec4(1.0);
		}
	)";
	FileIOHandler::write_buffer_to_file("./tmp_shader.glsl", src);
	ShaderLibrary::load("./tmp_shader.glsl");
	m_shader = ShaderLibrary::get("tmp_shader");
	// geometry
	data_dim = 300000;
	m_vertexCount = data_dim;
	m_vertices = new glm::vec2[m_vertexCount];
	float x_offset = 0.0f;
	float x_step = 1.0f / (float)data_dim;
	for (int i = 0; i < m_vertexCount; i++)
	{
		//m_vertices[i] = glm::vec2((float)i, 0.5f*sinf(256.0f*M_PI*x_offset));
		m_vertices[i] = glm::vec2(x_offset, 0.5f*sinf(256.0f*M_PI*x_offset));
		x_offset += x_step;
	}
	Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
	vbo->setBufferLayout({{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position"}});
	vbo->setData((void*)m_vertices, sizeof(glm::vec2) * m_vertexCount);
	m_vao = API::newVertexArray(vbo);
	// framebuffer
	m_renderBuffer = API::newFramebuffer(ColorFormat::RGBA16F, glm::ivec2(0), 1, true, true, "render_buffer");
	// misc setup
	Renderer::setClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// execute pending rendering commands
	Renderer::get().executeRenderCommands();
}
//---------------------------------------------------------------------------------------
void layer::onUpdate(float _dt)
{
	SYN_PROFILE_FUNCTION();
	static float fontHeight = (float)m_font->getFontHeight() + 1.0f;

	asyncInput();

	// bind presenting framebuffer
	m_renderBuffer->bind();

	// -- BEGINNING OF SCENE -- //
	detectVisibleVertices();
	
	Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_shader->enable();
	m_shader->setUniform1f("u_offset_x", m_offset.x);
	m_shader->setUniform1f("u_scale_x", m_scale.x);
	m_vao->bind();
	Renderer::drawArraysNoDepth(m_vertexCount, m_vertexOffset, GL_LINE_STRIP);
	//
	int i = 1;
	m_font->beginRenderBlock();
	m_font->addString(2.0f, fontHeight*i++, "offset: %f, %f", m_offset.x, m_offset.y);
	m_font->addString(2.0f, fontHeight*i++, "scale: %f, %f", m_scale.x, m_scale.y);
	m_font->addString(2.0f, fontHeight*i++, "mx_ndc: %f", mx_ndc);
	m_font->addString(2.0f, fontHeight*i++, "mx pre/post: %f, %f (delta %f)", mx_pre_zoom, mx_post_zoom, delta_zoom);
	m_font->addString(2.0f, fontHeight*i++, "vert_idx0: %d, vert_idx1: %d", vertex_idx0, vertex_idx1);
	m_font->addString(2.0f, fontHeight*i++, "vert_idx0_ndc: %f, vert_idx1_ndc: %f", vertex_idx0_ndc, vertex_idx1_ndc);
	m_font->addString(2.0f, fontHeight*i++, "vertex count %d, vertex offset %d", m_vertexCount, m_vertexOffset);
	m_font->endRenderBlock();
	// -- END OF SCENE -- //

	m_renderBuffer->bindDefaultFramebuffer();
}
//---------------------------------------------------------------------------------------
void layer::zoom(float _x)
{
	// find mouse x position in world space before zoom
	mx_pre_zoom = screenToWorldX(mx_ndc);
	//mx_pre_zoom = (((mx_ndc + 1.0f) / 2.0f) / m_scale.x) + m_offset.x;

		if (_x > 0.0f)
		m_scale.x *= 1.05f;
	else
		m_scale.x *= 0.95f;

	// find mouse x position after zoom
	mx_post_zoom = screenToWorldX(mx_ndc);
	//mx_post_zoom = (((mx_ndc + 1.0f) / 2.0f) / m_scale.x) + m_offset.x;
	delta_zoom = mx_pre_zoom - mx_post_zoom;
	m_offset.x += delta_zoom;
}
//---------------------------------------------------------------------------------------
void layer::detectVisibleVertices()
{
	// find world coordinates of extreme vertices at x -1 and 1
	double x_lo = -1.0;
	double x_hi =  1.0;
	x_lo = screenToWorldX(x_lo);
	x_hi = screenToWorldX(x_hi);
	// indices of vertices
	vertex_idx0 = max((int32_t)(x_lo * data_dim), 0);
	vertex_idx1 = min((int32_t)(x_hi * data_dim), data_dim);
	m_vertexOffset = vertex_idx0;
	m_vertexCount = min(vertex_idx1 - vertex_idx0, data_dim);

	// recreate (sanity check)
	vertex_idx0_ndc = worldToScreenX(m_vertices[vertex_idx0].x);
	vertex_idx1_ndc = worldToScreenX(m_vertices[vertex_idx1].x);
}
//---------------------------------------------------------------------------------------
void layer::asyncInput()
{
	static glm::vec2 dock_pos = Renderer::getImGuiDockingPositionF();
	curr_mpos = InputManager::get_mouse_position();
	glm::vec2 mpos_dock = curr_mpos - glm::vec2(0.0f, dock_pos.y);
	glm::vec2 vp = Renderer::getViewportF();

	// get mouse position as ndc
	mx_ndc = lmap(mpos_dock.x, 0.0f, vp.x, -1.0f, 1.0f);

	// get delta mouse movement for panning
	static float vp_x_inv = 1.0f / vp.x;
	glm::vec2 delta = curr_mpos - prev_mpos;
	if (InputManager::is_button_pressed(SYN_MOUSE_BUTTON_1) && delta.x != 0.0f && delta.y != 0.0f)
	{
		m_offset.x -= delta.x * vp_x_inv / m_scale.x;
		prev_mpos = curr_mpos;
	}
}
//---------------------------------------------------------------------------------------
void layer::onKeyDownEvent(Event* _e)
{
	SYN_PROFILE_FUNCTION();

	KeyDownEvent* e = dynamic_cast<KeyDownEvent*>(_e);
	static bool vsync = true;
	static glm::vec2 vp_offset = Renderer::getImGuiViewPortOffsetF();

	double x_lo = -1.0;
	double x_hi =  1.0;
	if (e->getAction() == GLFW_PRESS)
	{
		switch (e->getKey())
		{
		case SYN_KEY_Z:
			vsync = !vsync;
			Application::get().getWindow().setVSYNC(vsync);
			break;
		case SYN_KEY_ENTER:
			x_lo = screenToWorldX(x_lo);
			x_hi = screenToWorldX(x_hi);
			// indices of vertices
			vertex_idx0 = max((int32_t)(x_lo * data_dim), 0);
			vertex_idx1 = min((int32_t)(x_hi * data_dim), data_dim);
			m_vertexOffset = vertex_idx0;
			m_vertexCount = min(vertex_idx1 - vertex_idx0, data_dim);
			// recreate (sanity check)
			vertex_idx0_ndc = worldToScreenX(m_vertices[vertex_idx0].x);
			vertex_idx1_ndc = worldToScreenX(m_vertices[vertex_idx1].x);
			printf("x_lo = %f\nx_hi = %f\n", x_lo, x_hi);
			printf("lo_x = %f\nhi_x = %f\n", m_vertices[vertex_idx0].x, m_vertices[vertex_idx1].x);
			break;
		case SYN_KEY_ESCAPE:
			EventHandler::push_event(new WindowCloseEvent());
			break;
		case SYN_KEY_TAB:
			break;
		}
	}
}
//---------------------------------------------------------------------------------------
void layer::onMouseScrollEvent(Event* _e)
{
	MouseScrolledEvent* e = dynamic_cast<MouseScrolledEvent*>(_e);
	float z = e->getYOffset();
	zoom(z);
}
//---------------------------------------------------------------------------------------
void layer::onMouseButtonEvent(Event* _e)
{
	MouseButtonEvent* e = dynamic_cast<MouseButtonEvent*>(_e);
	switch (e->getButton())
	{
	case SYN_MOUSE_BUTTON_1:
		prev_mpos = InputManager::get_mouse_position();
		break;
	}
}
//---------------------------------------------------------------------------------------
void layer::onApplicationExit(Event* _e)
{
	if (m_vertices != nullptr)
		delete[] m_vertices;
}
//---------------------------------------------------------------------------------------
void layer::onImGuiRender()
{
	SYN_PROFILE_FUNCTION();

	static bool p_open = true;

	static bool opt_fullscreen_persistant = true;
	static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
	bool opt_fullscreen = opt_fullscreen_persistant;

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
