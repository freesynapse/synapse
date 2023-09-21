
#include "../../../pch.hpp"

#include "OrbitCamera.hpp"
#include "../../Input/InputManager.hpp"
#include "../Renderer.hpp"
#include "../../Event/EventHandler.hpp"
#include "../../Event/RendererEvents.hpp"


namespace Syn {


	OrbitCamera::OrbitCamera(float _fov_deg, 
							 float _screen_w, 
							 float _screen_h, 
							 float _z_near, 
							 float _z_far)
	{
		m_fov = _fov_deg;
		m_aspectRatio = _screen_w / _screen_h;
		m_zNear = _z_near;
		m_zFar = _z_far;

		// default for orbit cameras (differs from default of parent PerspectiveCamera)
		m_zoomSpeed = 1.0f;

		m_projectionMatrix = glm::perspectiveFov(glm::radians(m_fov), 
												 _screen_w,
												 _screen_h,
												 m_zNear, 
												 m_zFar);

		m_prevMousePosition = { 0.0f, 0.0f };

		// register for viewport resize events
		EventHandler::register_callback(EventType::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(OrbitCamera::onEvent));
		EventHandler::register_callback(EventType::WINDOW_TOGGLE_FROZEN_CURSOR, SYN_EVENT_MEMBER_FNC(OrbitCamera::onEvent));
		EventHandler::register_callback(EventType::INPUT_MOUSE_SCROLL, SYN_EVENT_MEMBER_FNC(OrbitCamera::onEvent));
	}


	//-----------------------------------------------------------------------------------
	void OrbitCamera::onUpdate(float _dt)
	{
		// get input to adjust position and look-at
		handleInput(_dt);

		// calculate the up vector
		m_forward = glm::normalize(m_position - m_target);
		// calculate temporary up vector
		if (fabs(m_forward.x) < FLT_EPSILON && fabs(m_forward.z) < FLT_EPSILON)
		{
			if (m_forward.y > 0)	// looking along pos y-axis
				m_up = { 0, 0, -1 };
			else					// looking along neg y-axis
				m_up = { 0, 0, 1 };
		}
		else
			m_up = { 0, 1, 0 };
		
		// calculate the right vector
		m_right = glm::normalize(glm::cross(m_forward, m_up));
		// recalculate the up vector
		m_up = glm::normalize(glm::cross(m_right, m_forward));

		// update camera position based on rotation angles
		m_position.x = m_radius * sinf(deg_to_rad(m_yAngle)) * cosf(deg_to_rad(m_xAngle));
		m_position.y = m_radius * cosf(deg_to_rad(m_yAngle));
		m_position.z = m_radius * sinf(deg_to_rad(m_yAngle)) * sinf(deg_to_rad(m_xAngle));

		// set view matrix and update VP matrix
		m_viewMatrix = glm::lookAt(m_position, m_target, m_up);
		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

	}
	//-----------------------------------------------------------------------------------
	void OrbitCamera::onEvent(Event* _e)
	{
		// handle change of viewport -- for now the only event handled
		
		ViewportResizeEvent* resizeEvent;// = dynamic_cast<ViewportResizeEvent*>(_e);
		WindowToggleFrozenCursorEvent* frozenCursorEvent;

		switch (_e->getEventType())
		{
			case EventType::VIEWPORT_RESIZE:
			{
				resizeEvent = dynamic_cast<ViewportResizeEvent*>(_e);
				m_aspectRatio = static_cast<float>(resizeEvent->getViewportX()) / static_cast<float>(resizeEvent->getViewportY());
				updateProjectionMatrix();
				m_reset = true;
				onUpdate(0.0f);
				break;
			}

			case EventType::WINDOW_TOGGLE_FROZEN_CURSOR:
			{
				updateViewMatrix();
				break;
			}
				
			case EventType::INPUT_MOUSE_SCROLL:
			{
				m_radius -= dynamic_cast<MouseScrolledEvent*>(_e)->getYOffset() * m_zoomSpeed;
				m_radius = clamp(m_radius, 0.05f, m_zFar+100.0f);
				break;
			}

			default:
				break;
		}

	}
	//-----------------------------------------------------------------------------------
	void OrbitCamera::handleInput(float _dt)
	{
		// break if input is disabled (i.e. in engine edit mode).
		//if (!m_updateCamera)
		//	return;
		
		// update look-at angles
		glm::vec2 mousePosition = InputManager::get_mouse_position();
		
		// check to see if inside viewport
		static glm::vec4 vp = Renderer::getViewportLimitsF();

		if (mousePosition.x < vp.x || mousePosition.x > vp.z ||
			mousePosition.y < vp.y || mousePosition.y > vp.w)
		{
			return;
		}

		if (m_reset)
		{
			m_prevMousePosition = mousePosition;
			m_reset = false;
		}

		if ((InputManager::is_button_pressed(SYN_MOUSE_BUTTON_1)) &&	// requires mouse pressed
			(mousePosition.x != m_prevMousePosition.x || mousePosition.y != m_prevMousePosition.y))
		{
			static glm::vec2 half_vp = Renderer::getViewportF() * 0.5f;

			m_delta = m_prevMousePosition - mousePosition;
			m_delta *= m_orbitSpeed;
			//m_delta *= (m_orbitSpeed * _dt);
			m_yAngle += m_delta.y;
			m_xAngle -= m_delta.x;
			// clamp angles
			if (m_xAngle >= 360.0f)	m_xAngle -= 360.0f;
			if (m_xAngle < 0.0f)	m_xAngle += 360.0f;

			m_yAngle = clamp(m_yAngle, 0.1f, 179.9f);
			
			// update mouse position
		}
	
		m_prevMousePosition = mousePosition;

	}


}