#pragma once


#include <glm/gtc/quaternion.hpp>

#include "Synapse/Renderer/Camera/Camera.hpp"
#include "Synapse/Core.hpp"
#include "Synapse/Event/Event.hpp"


namespace Syn {


	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera(const glm::mat4& _projection_matrix);

		virtual void onUpdate(float _dt) override;
		virtual void onEvent(Event* _e) override;

		// accessors
		inline float getXAngle() const { return m_xAngle; }
		inline void setXAngle(float _x) { m_xAngle = _x; }
		inline float getYAngle() const { return m_yAngle; }
		inline void setYAngle(float _y) { m_yAngle = _y; }
		inline void resetMousePosition() { m_reset = true; }
		inline const glm::vec2& getMouseDelta() const { return m_delta; }
		inline const glm::vec3& getLookAtVector() const { return m_lookAt; }
		inline const glm::vec3& getRightVector() const { return m_right; }
		inline const glm::vec3& getUpVector() const { return m_up; }

	private:
		void handleInput(float _dt);


	private:
		bool m_reset = true;
		glm::vec2 m_prevMousePosition = glm::vec2(0.0f);
		glm::vec2 m_delta = glm::vec2(0.0f);

		float m_xAngle = 0.0f;
		float m_yAngle = 0.0f;

		glm::vec3 m_up = glm::vec3(0.0f);
		glm::vec3 m_right = glm::vec3(0.0f);
		glm::vec3 m_forward = glm::vec3(0.0f);
		
		// the look-at vector is inherited from Camera
		//glm::vec3 m_lookAt = glm::vec3(0.0f);
	};


}
