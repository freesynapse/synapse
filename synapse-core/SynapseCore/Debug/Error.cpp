
#include "../../pch.hpp"

#include "../Debug/Error.hpp"
#include "../Debug/Log.hpp"
#include "../Core.hpp"

#ifdef _WIN64
#include <GL/GLU.h>
#endif


namespace Syn {

	std::string Error::m_lastError = "GL_NO_ERROR";


	//-----------------------------------------------------------------------------------
	std::string &Error::get_gl_error_string(GLenum _error_code)
	{

		switch (_error_code)
		{
			case GL_INVALID_ENUM: 					m_lastError = "GL_INVALID_ENUM"; 					break;
			case GL_INVALID_VALUE: 					m_lastError = "GL_INVALID_VALUE"; 					break;
			case GL_INVALID_OPERATION: 				m_lastError = "GL_INVALID_OPERATION"; 				break;
			case GL_STACK_OVERFLOW: 				m_lastError = "GL_STACK_OVERFLOW"; 					break;
			case GL_STACK_UNDERFLOW: 				m_lastError = "GL_STACK_UNDERFLOW"; 				break;
			case GL_OUT_OF_MEMORY: 					m_lastError = "GL_OUT_OF_MEMORY"; 					break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: 	m_lastError = "GL_INVALID_FRAMEBUFFER_OPERATION"; 	break;
			default: break;
		}

		return m_lastError;
	}


	//-----------------------------------------------------------------------------------
	int Error::glError(const char* _calling_func, const char* _gl_call)
	{
		GLenum error = GL_NO_ERROR;
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			Log::setErrorColor();
			raise_error(nullptr, _calling_func, _gl_call, ": ", get_gl_error_string(error).c_str());
			Log::resetColor();
			return RETURN_FAILURE;
		}
		Log::setOkColor();
		Log::log(_calling_func, _gl_call, ": GL_NO_ERROR");
		Log::resetColor();

		return RETURN_SUCCESS;
	}

}

