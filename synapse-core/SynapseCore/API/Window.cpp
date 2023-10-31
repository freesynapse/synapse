
#include "pch.hpp"

#include "Window.hpp"

#include "../Debug/Log.hpp"
#include "../Debug/Error.hpp"
#include "../Debug/Profiler.hpp"

#include "../Core.hpp"

#include "../Event/EventHandler.hpp"
#include "../Event/WindowEvents.hpp"
#include "../Event/InputEvents.hpp"

#include "../Input/InputManager.hpp"

#include "../Renderer/Renderer.hpp"

#include "../Utils/Timer/Timer.hpp"


namespace Syn 
{
	//-----------------------------------------------------------------------------------
	Window::Window(const char* _name, const int& _width, const int& _height, bool _fixed_size) :
		m_width(_width), m_height(_height), m_title(_name)
	{
		if (init(_fixed_size) == RETURN_FAILURE)
		{
			SYN_CORE_ERROR("Window initialization failed. Terminating.");
			glfwTerminate();
		}
		else
			SYN_CORE_TRACE("Window initialization complete.");

	}

	//-----------------------------------------------------------------------------------
	Window::~Window()
	{
		SYN_CORE_TRACE("terminating GLFW.");
		glfwTerminate();
	}

	//-----------------------------------------------------------------------------------
	int Window::init(bool _fixed_size)
	{

		// init GLFW
		//
		if (!glfwInit())
		{
			SYN_CORE_ERROR("glfwInit() failed.");
			return RETURN_FAILURE;
		}
		
		// desktop resolution (for positioning and size)
		#ifdef _WIN64
			RECT desktop;
			const HWND hDesktop = GetDesktopWindow();
			GetWindowRect(hDesktop, &desktop);
			int x_offset = desktop.right;
		#else
			const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			int x_offset = mode->width;
			/*
			SYN_CORE_TRACE("Primary monitor statistics:\n\t\t\tdimensions:\t", mode->width, "x", mode->height,
						    "\n\t\t\trefreshRate:\t", mode->refreshRate, 
							"\n\t\t\tchannels:\t\t", mode->redBits, "x", mode->greenBits, "x", mode->blueBits);
			*/
		#endif

		if (!_fixed_size)
		{
			m_width = mode->width;
			m_height = mode->height;
		}

		// borderless window
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		// create window
		m_window = glfwCreateWindow(m_width, m_height, m_title, NULL, NULL);
		if (m_window)
		{
			SYN_CORE_TRACE("GLFW window (", m_width, "x", m_height, ") created.");
		}
		else
		{
			SYN_CORE_ERROR("GLFW window could not be created.");
			return RETURN_FAILURE;
		}
		
		// set window position upper right

		// get glfw window frame size (i.e. including title bar etc.)
		int x0, x1, y0, y1;
		glfwGetWindowFrameSize(m_window, &x0, &y0, &x1, &y1);

		// set the position accordingly
		glfwSetWindowPos(m_window, x_offset - m_width - x1, y0);

		// pointer storage
		glfwMakeContextCurrent(m_window);
		glfwSwapInterval(m_vsync);
		glfwSetWindowUserPointer(m_window, this);
		m_primaryMonitor = glfwGetPrimaryMonitor();
		m_vidMode = glfwGetVideoMode(m_primaryMonitor);

		// maximise window
		glfwMaximizeWindow(m_window);
		//resize_callback(m_window, m_width, m_height);
		//glfwSetWindowMonitor(m_window, m_primaryMonitor, 0, 0, m_vidMode->width, m_vidMode->height, m_vidMode->refreshRate);


		// window callbacks
		//
		glfwSetFramebufferSizeCallback(m_window, resize_callback);
		// callbacks for keyboard and mouse input reside in the InputManager static class
		glfwSetKeyCallback(m_window, InputManager::key_callback);
		glfwSetMouseButtonCallback(m_window, InputManager::mouse_button_callback);
		glfwSetCursorPosCallback(m_window, InputManager::cursor_position_callback);
		glfwSetScrollCallback(m_window, InputManager::mouse_scroll_callback);


		// init glew
		//
		#ifdef _WIN64
		if (glewInit() != GLEW_OK)
		{
			SYN_CORE_ERROR("failed to initialize GLEW.");
			return RETURN_FAILURE;
		}
		else
		{
			SYN_CORE_TRACE("GLEW initialized.");
			SYN_CORE_TRACE("OpenGL vendor: ", glGetString(GL_VENDOR));
			SYN_CORE_TRACE("OpenGL renderer: ", glGetString(GL_RENDERER));
			SYN_CORE_TRACE("OpenGL version: ", glGetString(GL_VERSION));
		}
		#else
			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			{
				SYN_CORE_ERROR("[glad] could not load OpenGL.");
				return RETURN_FAILURE;
			}
			else
			{
				SYN_CORE_TRACE("GLAD initialized.");
				SYN_CORE_TRACE("OpenGL vendor: ", glGetString(GL_VENDOR));
				SYN_CORE_TRACE("OpenGL renderer: ", glGetString(GL_RENDERER));
				SYN_CORE_TRACE("OpenGL version: ", glGetString(GL_VERSION));
			}
		#endif


		// register callback for synapse events
		//
		//std::function<void(Event*)> eventCallback = std::bind(&Window::onEvent, this, std::placeholders::_1);
		EventHandler::register_callback(EventType::WINDOW_CLOSE, SYN_EVENT_MEMBER_FNC(Window::onEvent));
		EventHandler::register_callback(EventType::WINDOW_TOGGLE_FULLSCREEN, SYN_EVENT_MEMBER_FNC(Window::onEvent));
		EventHandler::register_callback(EventType::WINDOW_TOGGLE_CURSOR, SYN_EVENT_MEMBER_FNC(Window::onEvent));
		EventHandler::register_callback(EventType::WINDOW_TOGGLE_FROZEN_CURSOR, SYN_EVENT_MEMBER_FNC(Window::onEvent));
		EventHandler::register_callback(EventType::INPUT_KEY, SYN_EVENT_MEMBER_FNC(Window::onEvent));


		// set cursor to middle of screen
		glfwSetCursorPos(m_window, SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5);


		//
		return RETURN_SUCCESS;

	}


	//-----------------------------------------------------------------------------------
	void Window::onUpdate()
	{
		SYN_PROFILE_FUNCTION();

		// handle GLFW events
		glfwPollEvents();
		
		//
		if (m_frozenCursor)
			centerCursor();


		// swap buffers
		glfwSwapBuffers(m_window);

	}


	//-----------------------------------------------------------------------------------
	void Window::centerCursor()
	{
		glm::vec2 half_vp = Renderer::getViewportF() * 0.5f;
		glfwSetCursorPos(m_window, floor(half_vp.x), floor(half_vp.y));
	}


	//-----------------------------------------------------------------------------------
	void Window::onEvent(Event* _event)
	{
		switch (_event->getEventType())
		{
			case EventType::WINDOW_CLOSE:
			{
				m_toCloseWindow = true;
				break;
			}

			case EventType::WINDOW_TOGGLE_FULLSCREEN:
			{
				setFullscreen(m_isFullscreen);
				break;
			}

			case EventType::INPUT_KEY:
			{
				if (dynamic_cast<KeyDownEvent*>(_event)->getKey() == SYN_KEY_ESCAPE)
					m_toCloseWindow = true;
				break;
			}

			case EventType::WINDOW_TOGGLE_CURSOR:
			{
				m_visibleCursor = dynamic_cast<Syn::WindowToggleCursorEvent*>(_event)->getState();
				if (m_visibleCursor)
					glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				else
					// glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
					glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				break;
			}

			case EventType::WINDOW_TOGGLE_FROZEN_CURSOR:
			{
				m_frozenCursor = dynamic_cast<Syn::WindowToggleFrozenCursorEvent*>(_event)->getState();
				break;
			}

			default:
				break;

		}

	}


	//-----------------------------------------------------------------------------------
	void Window::setFullscreen(const bool& _fullscreen)
	{
		// fullscreen here refers to maximized, borderless window
		m_isFullscreen = _fullscreen;

		if (m_isFullscreen)
		{
			glfwSetWindowMonitor(m_window, m_primaryMonitor, 0, 0, m_vidMode->width, m_vidMode->height, m_vidMode->refreshRate);
		}
		else
		{
			glfwSetWindowMonitor(m_window, m_primaryMonitor, 0, 0, m_width, m_height, m_vidMode->refreshRate);
		}

		m_isFullscreen = !m_isFullscreen;

	}


	//-----------------------------------------------------------------------------------
	void Window::resize_callback(GLFWwindow* _window, int _width, int _height)
	{
		glViewport(0, 0, _width, _height);
		EventHandler::push_event(new WindowResizeEvent((uint32_t)_width, (uint32_t)_height));
	}


}