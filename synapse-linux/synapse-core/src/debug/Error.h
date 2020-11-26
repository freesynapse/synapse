#pragma once


#include "pch.hpp"

#include "src/Core.h"


namespace Syn { 

	class Error
	{
	public:

		template<typename T, typename ...Args>
		static void raise_fatal_error(std::function<void()> _callback, const char* _func, const T& _output_item, Args ...args)
		{
			std::function<void()> callback = nullptr;
			if (_callback != nullptr)
				callback = _callback;
			else
				// default to crash!
				callback = Error::error_exit;
			
			Log::log_no_func(_func, ">>> FATAL ERROR (callback 0x" + std::to_string(*(long*)(char*)& callback) + "): ", _output_item, args...);

			callback();

		}


		template<typename T, typename ...Args>
		static void raise_error(std::function<void()> _callback, const char* _func, const T& _output_item, Args ...args)
		{
			std::function<void()> callback = nullptr;
			if (_callback != nullptr)
				callback = _callback;
			else
				callback = nullptr;
			
			std::string errorMsg = ">>> ERROR";
			errorMsg += (callback == nullptr ? ": " : (" (callback 0x" + std::to_string(*(long*)(char*)& callback) + "): "));

			Log::log(_func, errorMsg, _output_item, args...);

		}


		template<typename T, typename ...Args>
		static void raise_warning(std::function<void()> _callback, const char* _func, const T& _output_item, Args ...args)
		{
			std::function<void()> callback = nullptr;
			if (_callback != nullptr)
				callback = _callback;
			else
				callback = nullptr;

			std::string errorMsg = ">>> WARNING";
			errorMsg += (callback == nullptr ? ": " : (" (callback 0x" + std::to_string(*(long*)(char*)& callback) + "): "));

			Log::log(_func, errorMsg, _output_item, args...);
		}


		static int glError(const char* _calling_func, const char* _gl_call="");
		static std::string &get_gl_error_string(GLenum _error_code);
		static std::string m_lastError;


	private:
		// default shutdown method
		static void error_exit() { exit(-1); }


	};



}


