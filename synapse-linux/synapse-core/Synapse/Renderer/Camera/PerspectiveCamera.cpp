
#include "pch.hpp"

#include "Synapse/Renderer/Camera/PerspectiveCamera.hpp"
#include "Synapse/Input/InputManager.hpp"
#include "Synapse/Renderer/Renderer.hpp"
#include "Synapse/Event/EventHandler.hpp"
#include "Synapse/Event/RendererEvents.hpp"


namespace Syn {


	PerspectiveCamera::PerspectiveCamera(const glm::mat4& _projection_matrix)
	{
		m_projectionMatrix = _projection_matrix;
		m_xAngle = 0.0f;
		m_yAngle = 0.0f;
		m_prevMousePosition = glm::vec2(0.0f);
		m_position = glm::vec3(0.0f, 20.0f, 130.0f);

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
		m_forward = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), m_right);
	}


	//-----------------------------------------------------------------------------------
	void PerspectiveCamera::onEvent(Event* _e)
	{
		// handle change of viewport -- for now the only event handled
		
		ViewportResizeEvent* resizeEvent;// = dynamic_cast<ViewportResizeEvent*>(_e);

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
				updateProjectionMatrix();
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
			m_position += m_right * CAMERA_MOVESPEED * _dt;
		else if (InputManager::is_key_pressed(SYN_KEY_A))
			m_position -= m_right * CAMERA_MOVESPEED * _dt;
		if (InputManager::is_key_pressed(SYN_KEY_SPACE))
			m_position += glm::vec3(0.0f, 1.0f, 0.0f) * CAMERA_MOVESPEED * _dt;
		else if (InputManager::is_key_pressed(SYN_KEY_LEFT_SHIFT))
			m_position -= glm::vec3(0.0f, 1.0f, 0.0f) * CAMERA_MOVESPEED * _dt;
		if (InputManager::is_key_pressed(SYN_KEY_W))
			m_position += m_forward * CAMERA_MOVESPEED * _dt;
		else if (InputManager::is_key_pressed(SYN_KEY_S))
			m_position -= m_forward * CAMERA_MOVESPEED * _dt;

		// update look-at angles
		glm::vec2 mousePosition = InputManager::get_mouse_position();
		if (m_reset)
		{
			m_prevMousePosition = mousePosition;
			m_reset = false;
		}
					
		if (mousePosition.x != m_prevMousePosition.x || mousePosition.y != m_prevMousePosition.y)
		{
			glm::vec2 half_vp = Renderer::getViewportF() * 0.5f;
			glm::vec2 delta = glm::vec2(floor(half_vp.x), floor(half_vp.y)) - mousePosition;
			m_delta = delta;
			delta *= CAMERA_LOOKAT_SPEED * _dt;
			m_xAngle -= delta.x;// *m_aspectRatio;
			m_yAngle -= delta.y;
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