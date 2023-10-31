#pragma once


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../Core.hpp"
#include "../../Memory.hpp"
#include "../../Event/Event.hpp"
#include "../../Utils/MathUtils.hpp"


namespace Syn {


	enum class CameraInputAction
	{
		NO_ACTION,
		MOVE_UP, MOVE_DOWN, MOVE_FORWARD, MOVE_BACKWARD, MOVE_LEFT, MOVE_RIGHT,
		STRAFE_LEFT, STRAFE_RIGHT,
		INCREASE_THETA, DECREASE_THETA,
		INCREASE_PHI, DECREASE_PHI,
		INCREASE_RADIUS, DECREASE_RADIUS,
	};


	class Camera
	{
	public:
		virtual ~Camera() = default;

		virtual void updateViewMatrix() {};
		virtual void reset() {};
		
		virtual void onUpdate(float _dt) {}
		virtual void updateProjectionMatrix() { m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_zNear, m_zFar); }
		virtual void onInput(CameraInputAction _action, float _dt) {}
		virtual void onInput(float _dx, float _dy, float _dt) {}
		virtual void onInput(float _dt) {}
		virtual void onEvent(Event* _e) = 0;

		// accessors
		virtual inline void setUpdateMode(bool _update) { m_updateCamera = _update; }

		// virtual inline void setProjectionMatrix(float _fov, float _width, float _height, float _z_near, float _z_far)
		// { m_projectionMatrix = glm::perspective(glm::radians(_fov), _width / _height, _z_near, _z_far); }
		virtual inline const void setProjectionMatrix(const glm::mat4& _mat) { m_projectionMatrix = _mat; }
		virtual inline const void setProjectionMatrix(float _left, float _right, float _bottom, float _top) {}
		/* FOV (in degrees) */
		virtual inline const void setFOV(float _fov) { m_fov = _fov; updateProjectionMatrix(); }
		virtual inline const void setViewDistance(float _znear, float _zfar) { m_zNear = _znear; m_zFar = _zfar; updateProjectionMatrix(); }

		virtual inline const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }
		virtual inline const glm::mat4& getViewMatrix() const { return m_viewMatrix; }
		virtual inline glm::mat4 getInvertedViewMatrix() { return glm::inverse(m_viewMatrix); }
		virtual inline const glm::mat4& getViewProjectionMatrix() const { return m_viewProjectionMatrix; }
		virtual inline const glm::vec3& getPosition() const { return m_position; }
		virtual inline void setPosition(const glm::vec3& _v) { m_position = _v; }
		/* FOV (in degrees) */
		virtual inline float getFOV() const { return m_fov; }
		virtual inline float getZNear() const { return m_zNear; }
		virtual inline float getZFar() const { return m_zFar; }
		virtual inline float getAspectRatio() const { return m_aspectRatio; }
		virtual inline void setAspectRatio(float _aspect_ratio) { m_aspectRatio = _aspect_ratio; }
		virtual inline const glm::vec3& getLookAtVector() const { return m_lookAt; }
		/* Speed variables */
		inline float getZoomSpeed() const 	 { return m_zoomSpeed; 	 }
		inline void setZoomSpeed(float _s) 	 { m_zoomSpeed = _s; 	 }
		inline float getMoveSpeed() const 	 { return m_moveSpeed;   }
		inline void setMoveSpeed(float _s) 	 { m_moveSpeed = _s; 	 }

	protected:
		glm::mat4 m_projectionMatrix = glm::mat4(1.0f);
		glm::mat4 m_viewMatrix = glm::mat4(1.0f);
		glm::mat4 m_viewProjectionMatrix = glm::mat4(1.0f);
		glm::vec3 m_position = glm::vec3(0.0f);

		float m_fov = 65.0f;
		float m_aspectRatio = 16.0f / 9.0f;
		float m_zNear = 0.1f;
		float m_zFar = 1000.0f;

		float m_moveSpeed   = 15.0f;	// PerspectiveCamera defaults
		float m_zoomSpeed   = 5.0f;

		glm::vec3 m_lookAt = glm::vec3(0.0f);

		bool m_updateCamera = false;
	};



}

