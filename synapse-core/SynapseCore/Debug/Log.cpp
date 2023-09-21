
#include "../../pch.hpp"

#include "../Debug/Log.hpp"
#include "../Utils/Timer/Time.hpp"


namespace Syn 
{	

	// static declarations
	std::fstream* Log::m_logFile;
	std::string Log::m_filename;
	bool Log::m_bNewline = true;
	bool Log::m_bInitialized = false;
	bool Log::m_bUseStdOut = false;

	uint32_t Log::s_errorCount = 0;
	uint32_t Log::s_warningCount = 0;

	#ifdef DEBUG_IMGUI_LOG
		uint32_t Log::m_imgui_log_last_size = 0;
		ImGuiTextBuffer Log::m_imgui_textbuffer;
		ImVector<int> Log::m_imgui_line_offset;
	#endif

	#ifdef _WIN64
		HANDLE Log::m_consoleHandle;
		WORD Log::m_colorCore = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
		WORD Log::m_colorError = FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
		WORD Log::m_colorWarning = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		WORD Log::m_colorClient = FOREGROUND_RED  | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		WORD Log::m_colorOk = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		WORD Log::m_colorNormal = 0;
	#endif


	//-----------------------------------------------------------------------------------
	void Log::open(const std::string &_file_name)
	{
		m_filename = _file_name;
		m_logFile = new std::fstream;
		m_logFile->open(_file_name.c_str(), std::ofstream::out);

		*m_logFile << ">>>> " << Time::current_date() << " : SYNAPSE-CORE <<<<" << std::endl;
		m_bInitialized = true;

		// initialize colors
		#ifdef _WIN64
			m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO info;
			GetConsoleScreenBufferInfo(m_consoleHandle, &info);
			m_colorNormal = info.wAttributes;
		#endif

	}


	//-----------------------------------------------------------------------------------
	void Log::close()
	{
		*m_logFile << ">>>> Application terminated : " 
				   << s_errorCount << " error(s); " << s_warningCount << " warning(s). <<<<" 
				   << std::endl;
		m_logFile->flush();
		m_logFile->close();
	}


	//-----------------------------------------------------------------------------------
	void Log::print()
	{
		std::cout << m_logFile->tellp();
		m_logFile->seekp(0);

		std::string line;
		while (std::getline(*m_logFile, line))
			std::cout << line << std::endl;
	}


	//-----------------------------------------------------------------------------------
	void Log::debug_matrix(const char* _func, const char* _mat_name, const glm::mat4& _m4)
	{
		char buffer[1024];
		memset(buffer, 0, 1024);
		int c = 0;
		c += std::sprintf(buffer, "%s\n", _mat_name);
		c += std::sprintf(buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  %s%.2f  |\n", ff(_m4[0][0]), fabs(_m4[0][0]), ff(_m4[0][1]), fabs(_m4[0][1]), ff(_m4[0][2]), fabs(_m4[0][2]), ff(_m4[0][3]), fabs(_m4[0][3]));
		c += std::sprintf(buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  %s%.2f  |\n", ff(_m4[1][0]), fabs(_m4[1][0]), ff(_m4[1][1]), fabs(_m4[1][1]), ff(_m4[1][2]), fabs(_m4[1][2]), ff(_m4[1][3]), fabs(_m4[1][3]));
		c += std::sprintf(buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  %s%.2f  |\n", ff(_m4[2][0]), fabs(_m4[2][0]), ff(_m4[2][1]), fabs(_m4[2][1]), ff(_m4[2][2]), fabs(_m4[2][2]), ff(_m4[2][3]), fabs(_m4[2][3]));
		std::sprintf(buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  %s%.2f  |", ff(_m4[3][0]), fabs(_m4[3][0]), ff(_m4[3][1]), fabs(_m4[3][1]), ff(_m4[3][2]), fabs(_m4[3][2]), ff(_m4[3][3]), fabs(_m4[3][3]));
		Log::log_no_func(_func, buffer);
	}


	//-----------------------------------------------------------------------------------
	void Log::debug_matrix(const char* _func, const char* _mat_name, const glm::mat3& _m3)
	{
		char buffer[1024];
		memset(buffer, 0, 1024);
		int c = 0;

		c += std::sprintf(buffer, "%s\n", _mat_name);
		c += std::sprintf(buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  |\n", ff(_m3[0][0]), fabs(_m3[0][0]), ff(_m3[0][1]), fabs(_m3[0][1]), ff(_m3[0][2]), fabs(_m3[0][2]));
		c += std::sprintf(buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  |\n", ff(_m3[1][0]), fabs(_m3[1][0]), ff(_m3[1][1]), fabs(_m3[1][1]), ff(_m3[1][2]), fabs(_m3[1][2]));
		std::sprintf(buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  |", ff(_m3[2][0]), fabs(_m3[2][0]), ff(_m3[2][1]), fabs(_m3[2][1]), ff(_m3[2][2]), fabs(_m3[2][2]));
		Log::log_no_func(_func, buffer);
	}


	//-----------------------------------------------------------------------------------
	void Log::debug_vector(const char* _func, const char* _vec_name, const glm::vec4& _v4)
	{
		char buffer[128];
		memset(buffer, 0, 128);
		int c = 0;

		c += std::sprintf(buffer, "%s: %s  ", _func, _vec_name);
		std::sprintf(buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  %s%.2f  |", ff(_v4.x), fabs(_v4.x), ff(_v4.y), fabs(_v4.y), ff(_v4.z), fabs(_v4.z), ff(_v4.w), fabs(_v4.w));
		Log::log_no_func(_func, buffer);
	}


	//-----------------------------------------------------------------------------------
	void Log::debug_vector(const char* _func, const char* _vec_name, const glm::vec3& _v3)
	{
		char buffer[128];
		memset(buffer, 0, 128);
		int c = 0;

		c += std::sprintf(buffer, "%s: %s  ", _func, _vec_name);
		std::sprintf(buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  |", ff(_v3.x), fabs(_v3.x), ff(_v3.y), fabs(_v3.y), ff(_v3.z), fabs(_v3.z));
		Log::log_no_func(_func, buffer);
	}


	//-----------------------------------------------------------------------------------
	void Log::debug_vector(const char* _func, const char* _vec_name, const glm::vec2& _v2)
	{
		char buffer[128];
		memset(buffer, 0, 128);
		int c = 0;

		c += std::sprintf(buffer, "%s: %s  ", _func, _vec_name);
		std::sprintf(buffer + c, "|  %s%.2f  %s%.2f  |", ff(_v2.x), fabs(_v2.x), ff(_v2.y), fabs(_v2.y));
		Log::log_no_func(_func, buffer);
	}

	//-----------------------------------------------------------------------------------
	void Log::debug_vector(const char* _func, const char* _vec_name, const glm::ivec4& _iv4)
	{
		char buffer[128];
		memset(buffer, 0, 128);
		int c = 0;

		c += std::sprintf(buffer, "%s: %s  ", _func, _vec_name);
		std::sprintf(buffer + c, "|  %s%d  %s%d  %s%d  %s%d  |", fi(_iv4.x), abs(_iv4.x), fi(_iv4.y), abs(_iv4.y), fi(_iv4.z), abs(_iv4.z), fi(_iv4.w), abs(_iv4.w));
		Log::log_no_func(_func, buffer);
	}


	//-----------------------------------------------------------------------------------
	void Log::debug_vector(const char* _func, const char* _vec_name, const glm::ivec3& _iv3)
	{
		char buffer[128];
		memset(buffer, 0, 128);
		int c = 0;

		c += std::sprintf(buffer, "%s: %s  ", _func, _vec_name);
		std::sprintf(buffer + c, "|  %s%d  %s%d  %s%d  |", fi(_iv3.x), abs(_iv3.x), fi(_iv3.y), abs(_iv3.y), fi(_iv3.z), abs(_iv3.z));
		Log::log_no_func(_func, buffer);
	}


	//-----------------------------------------------------------------------------------
	void Log::debug_vector(const char* _func, const char* _vec_name, const glm::ivec2& _iv2)
	{
		char buffer[128];
		memset(buffer, 0, 128);
		int c = 0;

		c += std::sprintf(buffer, "%s: %s  ", _func, _vec_name);
		std::sprintf(buffer + c, "|  %s%d  %s%d  |", fi(_iv2.x), abs(_iv2.x), fi(_iv2.y), abs(_iv2.y));
		Log::log_no_func(_func, buffer);
	}


	//-----------------------------------------------------------------------------------
	/*
	void Log::debug_AABB(const char* _func, const char* _aabb_name, const AABB& _aabb)
	{
		char buffer[256];
		memset(buffer, 0, 256);
		int c = 0;

		c += std::sprintf(buffer, "%s: %s  ", _func, _aabb_name);
		std::sprintf(buffer + c, "min |  %s%d  %s%d  %s%d  |   max |  %s%d  %s%d  %s%d  |", 
			fi(_aabb.min.x), fabs(_aabb.min.x), fi(_aabb.min.y), fabs(_aabb.min.y), fi(_aabb.min.z), fabs(_aabb.min.z),
			fi(_aabb.max.x), fabs(_aabb.max.x), fi(_aabb.max.y), fabs(_aabb.max.y), fi(_aabb.max.z), fabs(_aabb.max.z)
		);
		Log::log_no_func(_func, buffer);
	}
	*/

}

