#pragma once


#include "pch.hpp"

#include "Synapse/Core.hpp"


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
			
			Log::log_no_func(">>> FATAL ERROR (callback 0x" + std::to_string(*(long*)(char*)& callback) + ")", _func, ": ", _output_item, args...);
			Log::increaseErrorCount();

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
			std::string errorMsg = (callback == nullptr ? "" : (" (callback 0x" + std::to_string(*(long*)(char*)& callback) + ")"));
			//std::cout << "errorMsg: " << errorMsg << '\n' << "_func: " << _func << '\n' << "_output_item: " << _output_item << '\n';
			Log::log_no_func(">>> ERROR", errorMsg, _func, ": ", _output_item, args...);
			Log::increaseErrorCount();

		}


		//
		template<typename T, typename ...Args>
		static void raise_warning(std::function<void()> _callback, const char* _func, const T& _output_item, Args ...args)
		{
			std::function<void()> callback = nullptr;
			if (_callback != nullptr)
				callback = _callback;
			else
				callback = nullptr;

			std::string errorMsg = (callback == nullptr ? "" : (" (callback 0x" + std::to_string(*(long*)(char*)& callback) + ")"));
			Log::log_no_func(">>> WARNING", errorMsg, _func, ": ", _output_item, args...);
			Log::increaseWarningCount();
		}


		static int glError(const char* _calling_func, const char* _gl_call="");
		static std::string &get_gl_error_string(GLenum _error_code);
		static std::string m_lastError;


	private:
		// default shutdown method
		static inline void error_exit() { exit(-1); }

	};



}


