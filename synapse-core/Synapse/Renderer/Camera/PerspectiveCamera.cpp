
#include "pch.hpp"

#include "Synapse/Renderer/Camera/PerspectiveCamera.hpp"
#include "Synapse/Input/InputManager.hpp"
#include "Synapse/Renderer/Renderer.hpp"
#include "Synapse/Event/EventHandler.hpp"
#include "Synapse/Event/RendererEvents.hpp"


namespace Syn {

	PerspectiveCamera::PerspectiveCamera(float _fov_deg, 
										 float _screen_w, 
										 float _screen_h, 
										 float _z_near, 
										 float _z_far)
	{
		m_xAngle = 0.0f;
		m_yAngle = 0.0f;

		m_fov = _fov_deg;
		m_aspectRatio = _screen_w / _screen_h;
		m_zNear = _z_near;
		m_zFar = _z_far;

		m_prevMousePosition = { 0.0f, 0.0f };

		m_projectionMatrix = glm::perspectiveFov(glm::radians(m_fov), 
												 _screen_w,
												 _screen_h,
												 m_zNear, 
												 m_zFar);

		registerEvents();
	}
	//-----------------------------------------------------------------------------------
	void PerspectiveCamera::registerEvents()
	{
		// register for viewport resize events
		EventHandler::register_callback(EventType::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(PerspectiveCamera::onEvent));
		EventHandler::register_callback(EventType::WINDOW_TOGGLE_FROZEN_CURSOR, SYN_EVENT_MEMBER_FNC(PerspectiveCamera::onEvent));
	}
	//-----------------------------------------------------------------------------------
	void PerspectiveCamera::onUpdate(float _dt)
	{
		// get input to adjust position and look-at
		handleInput(_dt);

		// calculate pitch and yaw
		glm::mat4 matPitch = glm::rotate(glm::mat4(1.0f), glm::radians(m_yAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 matYaw = glm::rotate(glm::mat4(1.0f), glm::radians(m_xAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		// ordered multiplication
		glm::mat4 matRotation = matPitch * matYaw;
		// add in tranlation
		glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f), -m_position);
		// update view and VP matrices
		m_viewMatrix = matRotation * matTranslate;
		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
		// update directional vectors
		m_lookAt = glm::vec3(m_viewMatrix[0][2], m_viewMatrix[1][2], m_viewMatrix[2][2]);
		m_right = glm::vec3(m_viewMatrix[0][0], m_viewMatrix[1][0], m_viewMatrix[2][0]);
		m_up = glm::normalize(glm::cross(m_lookAt, m_right));
		// update forward move vector
		m_forward = glm::cross(m_up, m_right);
	}
	//-----------------------------------------------------------------------------------
	void PerspectiveCamera::onEvent(Event* _e)
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
				static glm::vec3 currentPos;
				static float currentXAngle, currentYAngle;
				frozenCursorEvent = dynamic_cast<WindowToggleFrozenCursorEvent*>(_e);
				
				// toggle == false -> going into editor mode
				if (frozenCursorEvent->getState() == true)
				{
					//currentPos = m_position;
					//currentXAngle = m_xAngle;
					//currentXAngle = m_yAngle;
					updateViewMatrix();
				}
				// toggle == true -> going into camera mode
				else
				{
					//m_position = currentPos;
					//m_xAngle = currentXAngle;
					//m_yAngle = currentYAngle;
					updateViewMatrix();
				}
				
				//SYN_CORE_TRACE("frozen cursor toggle = ", toggle ? "TRUE" : "FALSE");
				// TODO: add code to prevent camera from moving when in editor mode
				//
				//updateProjectionMatrix();

				break;
			}

			default:
				break;
		}

	}
	//-----------------------------------------------------------------------------------
	void PerspectiveCamera::handleInput(float _dt)
	{
		// break if input is disabled (i.e. in engine edit mode).
		if (!m_updateCamera)
			return;
		
		// update position
		if (InputManager::is_key_pressed(SYN_KEY_D))
			m_position += m_right * m_moveSpeed * _dt;
		else if (InputManager::is_key_pressed(SYN_KEY_A))
			m_position -= m_right * m_moveSpeed * _dt;
		if (InputManager::is_key_pressed(SYN_KEY_SPACE))
			m_position += glm::vec3(0.0f, 1.0f, 0.0f) * m_moveSpeed * _dt;
		else if (InputManager::is_key_pressed(SYN_KEY_LEFT_SHIFT))
			m_position -= glm::vec3(0.0f, 1.0f, 0.0f) * m_moveSpeed * _dt;
		if (InputManager::is_key_pressed(SYN_KEY_W))
			m_position += m_forward * m_moveSpeed * _dt;
		else if (InputManager::is_key_pressed(SYN_KEY_S))
			m_position -= m_forward * m_moveSpeed * _dt;

		// update look-at angles
		glm::vec2 mousePosition = InputManager::get_mouse_position();
		if (m_reset)
		{
			m_prevMousePosition = mousePosition;
			m_reset = false;
		}
					
		if (mousePosition.x != m_prevMousePosition.x || mousePosition.y != m_prevMousePosition.y)
		{
			static glm::vec2 half_vp = Renderer::getViewportF() * 0.5f;
			m_delta = glm::vec2(floor(half_vp.x), floor(half_vp.y)) - mousePosition;
			//delta *= (m_lookatSpeed * _dt);
			m_delta *= m_lookatSpeed;
			m_xAngle -= m_delta.x;// *m_aspectRatio;
			m_yAngle -= m_delta.y;
			// clamp angles
			if (m_xAngle >= 360.0f)
				m_xAngle -= 360.0f;
			if (m_xAngle < 0.0f)
				m_xAngle += 360.0f;
			m_yAngle = std::max(-90.0f, std::min(m_yAngle, 90.0f));
			// update mouse position
			m_prevMousePosition = mousePosition;
		}

	}


}