
#include "pch.hpp"

#include "Synapse/Utils/Timer/Time.hpp"


namespace Syn {	

	// static class variable declarations
	std::tm Time::m_tmNow;


	//-----------------------------------------------------------------------------------
	std::tm Time::now()
	{
		std::chrono::time_point<std::chrono::system_clock> chronoTime = std::chrono::system_clock::now();
		std::time_t now_t = std::chrono::system_clock::to_time_t(chronoTime);
#pragma warning(push)
#pragma warning(disable: 4996)
		m_tmNow = *std::localtime(&now_t);
#pragma warning(pop)

		return m_tmNow;
	}

	//-----------------------------------------------------------------------------------
	std::string Time::current_time()
	{
		std::tm tm_now = now();

		// extract the time in HH:MM:SS and return.
		std::string time = "";
		std::stringstream ss;
		ss << std::put_time(&tm_now, "%H:%M:%S");

		return ss.str();
	}


	//-----------------------------------------------------------------------------------
	std::string Time::current_date()
	{
		std::tm tm_now = now();
		// extract the time in YYYY:MM:DD and return.
		std::string time = "";
		std::stringstream ss;
		ss << std::put_time(&tm_now, "%Y-%m-%d");

		return ss.str();

	}


}