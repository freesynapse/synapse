#pragma once


#include <glm/gtc/quaternion.hpp>

#include "Camera.hpp"
#include "../../Core.hpp"
#include "../../Event/Event.hpp"


namespace Syn {


	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera() = default;
		PerspectiveCamera(const PerspectiveCamera&) = default;

		PerspectiveCamera(float _fov_deg, float _screen_w, float _screen_h, float _z_near, float _z_far);

		virtual void onUpdate(float _dt) override;
		virtual void onEvent(Event* _e) override;

		virtual inline void resetMousePosition() { m_reset = true; }

		inline float getXAngle() const  	 { return m_xAngle; 	 }
		inline void setXAngle(float _x) 	 { m_xAngle = _x;   	 }
		inline float getYAngle() const  	 { return m_yAngle; 	 }
		inline void setYAngle(float _y) 	 { m_yAngle = _y;   	 }
		/* default = 0.05f */
		inline float getLookAtSpeed() const  { return m_lookatSpeed; }
		/* default = 0.05f */
		inline void setLookAtSpeed(float _s) { m_lookatSpeed = _s; 	 }
		virtual inline const glm::vec2& getMouseDelta()    const  { return m_delta;   }
		virtual inline const glm::vec3& getLookAtVector()  const override { return m_lookAt;  }
		virtual inline const glm::vec3& getForwardVector() const  { return m_forward; }
		virtual inline const glm::vec3& getRightVector()   const  { return m_right;   }
		virtual inline const glm::vec3& getUpVector() 	   const  { return m_up; 	  }

	private:
		virtual void handleInput(float _dt);
		virtual void registerEvents();


	protected:
		bool m_reset = true;
		glm::vec2 m_prevMousePosition = { 0.0f, 0.0f };
		glm::vec2 m_delta 			  = { 0.0f, 0.0f };

		float m_lookatSpeed = 0.05f;

		float m_xAngle = 0.0f;
		float m_yAngle = 0.0f;

		glm::vec3 m_up 		= { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_right 	= { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_forward = { 0.0f, 0.0f, 0.0f };
		
		// the look-at vector is inherited from Camera
		//glm::vec3 m_lookAt = glm::vec3(0.0f);
	};


}
