#pragma once


#include "Synapse/Core.hpp"


namespace Syn {


	// 
	struct PerspectiveCameraConfig
	{
		glm::vec3 position;
		float xAngle;
		float yAngle;

		float fovDegrees;
		float viewportWidth;
		float viewportHeight;
		float zNear;
		float zFar;

	};

	//
	struct MouseConfig
	{
		float zoomSpeed;
		float panSpeed;
		float rotationSpeed;
	};


	// forward declarations
	class PerspectiveCamera;


	// static class
	class Config
	{

		static void load(const std::string& _load_path="");
		static void save(const std::string& _save_path="");

		static void loadShaders();

		// accessors
		static inline Config& get() { return *s_instance; }


	private:
		PerspectiveCameraConfig s_perspectiveCameraConfig;


		static Config* s_instance;

	};


}