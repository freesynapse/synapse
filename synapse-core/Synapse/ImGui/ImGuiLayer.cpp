
#include "pch.hpp"

#include "Synapse/ImGui/ImGuiLayer.hpp"

#include "Synapse/Application.hpp"

#include "External/imgui/imgui.h"
//#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "External/imgui/examples/imgui_impl_opengl3.h"
#include "External/imgui/examples/imgui_impl_glfw.h"


namespace Syn {


	void ImGuiLayer::onAttach() 
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;		// Disable ImGui interfering with glfwSetInputMode()
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		ImFont* pFont = io.Fonts->AddFontFromFileTTF("../assets/ttf/ubuntu.mono.ttf", 16.0f);
		#ifdef _MSV_VER
			io.FontDefault = io.Fonts->Fonts.back();
		#else
			io.FontDefault = pFont;
		#endif
		
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

		Application& app = Application::get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.getWindow().getNativeWindow());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330");

	}


	//-----------------------------------------------------------------------------------
	void ImGuiLayer::onDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}


	//-----------------------------------------------------------------------------------
	void ImGuiLayer::begin()
	{
		// start new ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}


	//-----------------------------------------------------------------------------------
	void ImGuiLayer::end()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::get();
		io.DisplaySize = ImVec2((float)app.getWindow().getWidth(), (float)app.getWindow().getHeight());

		// render
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}


	//-----------------------------------------------------------------------------------
	void ImGuiLayer::onUpdate(float _dt)
	{

	}


	//-----------------------------------------------------------------------------------
	void ImGuiLayer::onEvent(Event* _event)
	{

	}


	//-----------------------------------------------------------------------------------
	void ImGuiLayer::onImGuiRender()
	{

	}


}