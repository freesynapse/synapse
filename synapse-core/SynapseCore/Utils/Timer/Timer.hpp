#pragma once


#include "../../../pch.hpp"

#include "../../Core.hpp"
#include "../../Debug/Log.hpp"


namespace Syn {


	// scoped-based timer class
	class Timer
	{
	public:
		Timer(const char* _func="", bool _print_result=false) : // RAII - resource acquisition is initialization.
			m_printResult(_print_result)
		{
			m_printResult = _print_result;

			if (m_printResult)
			{
				memset(m_caller, '\0', 128);
				std::sprintf(m_caller, "%s", _func);
			}

			m_startTimepoint = std::chrono::high_resolution_clock::now();
		}

		~Timer()
		{
			stop();
		}

		void stop()
		{
			auto endTimepoint = std::chrono::high_resolution_clock::now();
			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimepoint).time_since_epoch().count();
			auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

			auto duration = end - start;
			double ms = duration * 0.001;

			if (m_printResult && Log::initialized())
				Log::log_no_func(m_caller, ": ", ms, " ms.");
			else if (m_printResult && !Log::initialized())
				printf("%s: %.3f ms.\n", m_caller, ms);
		}

		/* Elapsed time in microseconds. */
		long long getDeltaTime()
		{
			auto endTimepoint = std::chrono::high_resolution_clock::now();
			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimepoint).time_since_epoch().count();
			auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

			auto duration = end - start;

			return duration;
		}

		float getDeltaTimeMs() { return getDeltaTime() * 0.001f; }

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimepoint;
		char m_caller[128] = "";
		bool m_printResult = false;

	};



}


