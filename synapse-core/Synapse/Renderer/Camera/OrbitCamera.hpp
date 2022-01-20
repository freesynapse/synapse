#pragma once


#include <glm/gtc/quaternion.hpp>

#include "Synapse/Renderer/Camera/PerspectiveCamera.hpp"
#include "Synapse/Core.hpp"
#include "Synapse/Event/Event.hpp"


namespace Syn {


	class OrbitCamera : public PerspectiveCamera
	{
	public:
		OrbitCamera(float _fov_deg, float _screen_w, float _screen_h, float _z_near, float _z_far);

		virtual void onUpdate(float _dt) override;
		virtual void onEvent(Event* _e) override;

		// accessors
		/* Rotation speed, default 10.0f */
		inline void setOrbitSpeed(float _s) { m_orbitSpeed = _s; }
		/* Distance from target, initial value 10.0f */
		inline void setRadius(float _r) { m_radius = _r; }
		inline const float getRadius() const { return m_radius; }

	private:
		virtual void handleInput(float _dt) override;


	private:
		bool m_reset = true;
		glm::vec2 m_prevMousePosition = { 0.0f, 0.0f };
		glm::vec2 m_delta 			  = { 0.0f, 0.0f };

		float m_radius		= 10.0f;	// distance from target
		//float m_zoomSpeed 	=  1.0f;
		float m_orbitSpeed 	=  0.1f;

		//float m_xAngle      = 0.0f;		// theta : x/z rotation angle (degrees)
		//float m_yAngle		= 90.0f;	// phi   :   y rotation angle (degrees)

		glm::vec3 m_target 	= { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_up 		= { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_right 	= { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_forward = { 0.0f, 0.0f, 0.0f };
		
		// the look-at vector is inherited from Camera
		//glm::vec3 m_lookAt = glm::vec3(0.0f);
	};


}
