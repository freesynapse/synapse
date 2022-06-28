
#include "pch.hpp"

#include "Synapse/Utils/Timer/TimeStep.hpp"
#include "Synapse/Debug/Profiler.hpp"


namespace Syn {


	// static declarations
	float TimeStep::s_currentTime = 0.0f;
	float TimeStep::s_deltaTimeMs = 0.0f;
	float TimeStep::s_lastFrameTime = 0.0f;
	float TimeStep::s_cumulativeTime = 0.0f;
	float TimeStep::s_idleTime = 0.0f;
	uint32_t TimeStep::s_nframes = 0;
	long long TimeStep::s_framesTotal = 0;
	float TimeStep::s_fps = 0.0f;


	//-----------------------------------------------------------------------------------
	void TimeStep::update()
	{
		SYN_PROFILE_FUNCTION();

		s_currentTime = (float)glfwGetTime();
		s_deltaTimeMs = (s_currentTime - s_lastFrameTime);// * 1000.0f;

		s_lastFrameTime = s_currentTime;

		s_cumulativeTime += s_deltaTimeMs;
		s_nframes++;

		// update FPS every second
		if (s_cumulativeTime >= 1.0f)//1000.0f)
		{
			s_fps = (float)s_nframes / (s_cumulativeTime);// / 1000.0f);
			s_cumulativeTime = 0.0f;
			s_nframes = 0;
		}

		// increase total frame count
		s_framesTotal++;
	}


}
