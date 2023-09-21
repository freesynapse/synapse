#pragma once


#include "../../../pch.hpp"


namespace Syn {	

	class Time
	{
	public:
		static std::string current_time();
		static std::string current_date();
	private:
		static std::tm now();

	private:
		static std::tm m_tmNow;

	};


}

