#pragma once


#include "Camera.hpp"


namespace Syn {	

	struct OrthographicCameraBounds
	{
		float left, right;
		float bottom, top;

		float getWidth()  { return right - left; }
		float getHeight() { return top - bottom; }
	};


	class OrthographicCamera : public Camera
	{
	public:
		// default constructor -- with zoom
		OrthographicCamera(float _aspect_ratio, float _zoom_level=10.0f);
		~OrthographicCamera() {}
		// don't really know when I'd use this
		//OrthographicCamera(float _left, float _right, float _bottom, float _top);

		// Required in every inheritance of the base Camera class:
		// ImGui calls camera_ptr->updateProjectionMatrix() on viewport resize.
		virtual void updateProjectionMatrix() override;

		// specific for orthographic camera
		virtual void updateViewMatrix() override;

		// usual overrides
		virtual void onUpdate(float _dt) override;
		virtual void reset() override;
		virtual void onEvent(Event* _e) override;

		// accessors
		inline const void setProjectionMatrix(float _left, float _right, float _bottom, float _top) override;
		inline void setPosition(const glm::vec3& _vpos) override { m_position = _vpos; updateViewMatrix(); }
		inline void setRotation(const float& _theta) { m_theta = _theta; updateViewMatrix(); }
		inline float getTheta() const { return m_theta; }
		inline void setZoomLevel(float _zoom_level) { m_zoomLevel = _zoom_level; }
		inline float getZoomLevel() { return m_zoomLevel; }
		inline void setZoomLimit(float _zoom_limit) { m_zoomLimit = _zoom_limit; }
		inline float getZoomLimit() { return m_zoomLimit; }
		inline void setZoomAmplifier(float _zoom_amp) { m_zoomAmplifier = _zoom_amp; }
		inline float getZoomAmplifier() { return m_zoomAmplifier; }

		inline const OrthographicCameraBounds& getBounds() const { return m_bounds; }


	private:

		float m_theta = 0.0f;
		float m_zoomLevel = 1.0f;
		float m_zoomLimit = 0.05f;
		float m_zoomAmplifier = 1.0f;

		OrthographicCameraBounds m_bounds;

		const float m_cameraRotationSpeed = 180.0f;

	};


}
