#pragma once


#include <GLFW/glfw3.h>
#include <vector>


namespace Syn {


	class TimeStep
	{
	public:
		static void update();

		static inline const float getDeltaTime() { return s_deltaTimeMs; }
		static inline const float getFPS() { return s_fps; }
		static inline const long long getFrameCount() { return s_framesTotal; }
		static inline void setIdleTime(float _idle) { s_idleTime = _idle; }
		static inline const float getIdleTime() { return s_idleTime; }

	private:
		static float s_currentTime;
		static float s_deltaTimeMs;
		static float s_lastFrameTime;
		static float s_cumulativeTime;
		static float s_idleTime;
		static uint32_t s_nframes;
		static long long s_framesTotal;
		static float s_fps;



	};

}
