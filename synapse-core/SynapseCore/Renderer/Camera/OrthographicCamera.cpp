
#include "../../../pch.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "OrthographicCamera.hpp"
#include "../../Input/InputManager.hpp"
#include "../../Debug/Profiler.hpp"
#include "../../Event/EventHandler.hpp"
#include "../../Event/RendererEvents.hpp"
#include "../../Event/InputEvents.hpp"


namespace Syn {


	OrthographicCamera::OrthographicCamera(float _aspect_ratio, float _zoom_level)
	{
		SYN_PROFILE_FUNCTION();

		m_aspectRatio = _aspect_ratio;
		m_zoomLevel = _zoom_level;

		m_zoomSpeed = 0.25f;
		m_moveSpeed = 6.0f;

		m_bounds = { -m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel };

		// uses aspect ratio and zoom level
		updateProjectionMatrix();

		m_viewMatrix = glm::mat4(1.0f);

		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

		// register event callbacks
		EventHandler::register_callback(EventType::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(OrthographicCamera::onEvent));
		EventHandler::register_callback(EventType::WINDOW_TOGGLE_FROZEN_CURSOR, SYN_EVENT_MEMBER_FNC(OrthographicCamera::onEvent));
		EventHandler::register_callback(EventType::INPUT_MOUSE_SCROLL, SYN_EVENT_MEMBER_FNC(OrthographicCamera::onEvent));

	}


	//-----------------------------------------------------------------------------------
	/*
	OrthographicCamera::OrthographicCamera(float _left, float _right, float _bottom, float _top)
	{
		SYN_PROFILE_FUNCTION();

		// matrix setup
		m_projectionMatrix = glm::ortho(_left, _right, _bottom, _top, -1.0f, 1.0f);
		m_viewMatrix = glm::mat4(1.0f);

		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

		// register event callbacks
		EventHandler::register_callback(EventType::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(OrthographicCamera::onEvent));
		EventHandler::register_callback(EventType::WINDOW_TOGGLE_FROZEN_CURSOR, SYN_EVENT_MEMBER_FNC(OrthographicCamera::onEvent));

	}
	*/


	//-----------------------------------------------------------------------------------
	const void OrthographicCamera::setProjectionMatrix(float _left, float _right, float _bottom, float _top)
	{
		SYN_PROFILE_FUNCTION();

		m_projectionMatrix = glm::ortho(_left, _right, _bottom, _top, -1.0f, 1.0f);
	}


	//-----------------------------------------------------------------------------------
	void OrthographicCamera::updateViewMatrix()
	{
		SYN_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position) * 
							  glm::rotate(glm::mat4(1.0f), glm::radians(m_theta), glm::vec3(0.0f, 0.0f, 1.0f));

		//m_viewMatrixInverted = transform;
		m_viewMatrix = glm::inverse(transform);

		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

	}


	//-----------------------------------------------------------------------------------
	void OrthographicCamera::updateProjectionMatrix()
	{
		SYN_PROFILE_FUNCTION();

		m_projectionMatrix = glm::ortho(-m_aspectRatio * m_zoomLevel, 
							  		     m_aspectRatio * m_zoomLevel,
									    -m_zoomLevel,
									     m_zoomLevel,
									    -1.0f, 1.0f);

		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;										
		
	}


	//-----------------------------------------------------------------------------------
	void OrthographicCamera::reset()
	{
		SYN_PROFILE_FUNCTION();

		m_position = glm::vec3(0.0f);
		m_theta = 0.0f;
		m_zoomLevel = 1.0f;

		updateProjectionMatrix();
		updateViewMatrix();
	}


	//-----------------------------------------------------------------------------------
	void OrthographicCamera::onUpdate(float _dt)
	{
		SYN_PROFILE_FUNCTION();

		// only update if in engine camera mode (i.e. not edit mode).
		// -- not true for orthographic cameras
		//if (!m_updateCamera)
		//	return;
			
		glm::vec3 prevPos = m_position;
		float prevTheta = m_theta;
		float adj_speed = m_moveSpeed * _dt * m_zoomLevel;

		// update position
		if (InputManager::is_key_pressed(SYN_KEY_D))
		{
			m_position.x += cos(glm::radians(m_theta)) * adj_speed;
			m_position.y += sin(glm::radians(m_theta)) * adj_speed;
		}
		else if (InputManager::is_key_pressed(SYN_KEY_A))
		{
			m_position.x -= cos(glm::radians(m_theta)) * adj_speed;
			m_position.y -= sin(glm::radians(m_theta)) * adj_speed;
		}
		
		if (InputManager::is_key_pressed(SYN_KEY_W))
		{
			m_position.x += -sin(glm::radians(m_theta)) * adj_speed;
			m_position.y +=  cos(glm::radians(m_theta)) * adj_speed;
		}
		else if (InputManager::is_key_pressed(SYN_KEY_S))
		{
			m_position.x -= -sin(glm::radians(m_theta)) * adj_speed;
			m_position.y -=  cos(glm::radians(m_theta)) * adj_speed;
		}

		// update rotation
		if (InputManager::is_key_pressed(SYN_KEY_Q))
			m_theta += m_cameraRotationSpeed * _dt;
		if (InputManager::is_key_pressed(SYN_KEY_E))
			m_theta -= m_cameraRotationSpeed * _dt;
		
		// rotation limits
		if (m_theta > 180.0f)
			m_theta -= 360.0f;
		else if (m_theta <= -180.0f)
			m_theta += 360.0f;

		// reset?
		if (InputManager::is_key_pressed(SYN_KEY_R))
			reset();


		// update the view matrix if position has changed
		if ((prevPos != m_position) || (prevTheta != m_theta))
			updateViewMatrix();

	}


	//-----------------------------------------------------------------------------------
	void OrthographicCamera::onEvent(Event* _e)
	{
		SYN_PROFILE_FUNCTION();

		switch (_e->getEventType())
		{
			case EventType::VIEWPORT_RESIZE:
			{
				SYN_CORE_TRACE("called");
				m_aspectRatio = (float)dynamic_cast<ViewportResizeEvent*>(_e)->getViewportX() / (float)dynamic_cast<ViewportResizeEvent*>(_e)->getViewportY();
				m_bounds = { -m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel };
				updateProjectionMatrix();
				break;
			}

			case EventType::WINDOW_TOGGLE_FROZEN_CURSOR:
			{
				// TODO: add code to prevent camera from moving when in editor mode
				//
				updateProjectionMatrix();
				break;
			}

			case EventType::INPUT_MOUSE_SCROLL:
			{
				// only update if in engine camera mode (i.e. not edit mode).			
				// -- not true for orthographic cameras
				//if (!m_updateCamera)
				//	break;

				//SYN_CORE_TRACE("scrolled: ", dynamic_cast<MouseScrolledEvent*>(_e)->getYOffset());

				if (m_zoomAmplifier > 1.0f || m_zoomAmplifier < 1.0f)
					m_zoomLevel *= pow(m_zoomAmplifier, dynamic_cast<MouseScrolledEvent*>(_e)->getYOffset());// * m_zoomSpeed);
				else
					m_zoomLevel -= dynamic_cast<MouseScrolledEvent*>(_e)->getYOffset() * m_zoomSpeed;

				m_zoomLevel = std::max(m_zoomLimit, m_zoomLevel);
				m_bounds = { -m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel };				
				updateProjectionMatrix();
				break;
			}

			default: 
				break;
		}
	}



}