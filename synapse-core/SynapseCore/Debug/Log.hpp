
#pragma once


#include "../../pch.hpp"

#include "../Utils/Timer/Time.hpp"
#include "../Utils/FileIOHandler.hpp"
#include "../../External/imgui/imgui.h"

#define SYN_DEBUG_VECTOR(id, vec) 	Syn::Log::debug_vector(__func__, id, vec)
#define SYN_DEBUG_MATRIX(id, mat) 	Syn::Log::debug_matrix(__func__, id, mat)
#define SYN_DEBUG_PTR(ptr_id, ptr)	Syn::Log::setCoreColor();\
									{\
									char s[64]; memset(s, 0, 64); sprintf(s, "%s : 0x%" PRIXPTR ".", ptr_id, (uintptr_t)ptr);\
									Syn::Log::log(__func__, s);\
									}\
									Syn::Log::resetColor();



namespace Syn { 

	// static class
	class Log
	{
	public:
		
		// Opens the log for writing.
		static void open(const std::string &_file_name);
		
		// Closes the log; NB: important since a pointer std::ofstream is used!
		static void close();

		// Prints the whole log.
		static void print();

		// Debug matrices and vectors
		static void debug_matrix(const char* _func, const char *_mat_name, const glm::mat4& _m4);
		static void debug_matrix(const char* _func, const char* _mat_name, const glm::mat3& _m3);
		static void debug_vector(const char* _func, const char* _vec_name, const glm::vec4& _v4);
		static void debug_vector(const char* _func, const char* _vec_name, const glm::vec3& _v3);
		static void debug_vector(const char* _func, const char* _vec_name, const glm::vec2& _v2);
		static void debug_vector(const char* _func, const char* _vec_name, const glm::ivec4& _iv4);
		static void debug_vector(const char* _func, const char* _vec_name, const glm::ivec3& _iv3);
		static void debug_vector(const char* _func, const char* _vec_name, const glm::ivec2& _iv2);
		//static void debug_AABB(const char* _func, const char* _aabb_name, const AABB& _aabb);

		// Flags
		static void output_new_line(const bool &_b = true) { m_bNewline = _b; }
		static void use_stdout(const bool &_b = false) { m_bUseStdOut = _b; }


		// ImGui log functions
		// #define DEBUG_IMGUI_LOG	// messy, circular includes with Log.h and Core.h otherwise
		#ifdef DEBUG_IMGUI_LOG
			inline static const ImGuiTextBuffer& getImGuiBuffer() { return m_imgui_textbuffer; }
			inline static const ImVector<int>& getImGuiLineOffset() { return m_imgui_line_offset; }

			static void imgui_log_clear()
			{
				m_imgui_textbuffer.clear();
				m_imgui_line_offset.clear();
				m_imgui_line_offset.push_back(0);
			}

			// add new lines in output file to log
			static void imgui_log_update()
			{				
				std::ifstream file(m_filename, std::ios::in | std::ios::binary);
				int old_buffer_size = m_imgui_textbuffer.size();

				// find new bytes in file since last time
				file.seekg(0, std::ios::end);
				int new_bytes = (int)file.tellg() - m_imgui_log_last_size;

				if (new_bytes > 0)
				{
					// goto offset m_imgui_log_last_size from beginning and read new bytes
					file.seekg(m_imgui_log_last_size, std::ios::beg);

					char* buffer = new char[new_bytes + 1];

					// read the new content
					file.read(buffer, new_bytes);
					// add terminating 0 to tell ImGui that this entry ends.
					buffer[new_bytes] = '\0';
				
					// add to output
					m_imgui_textbuffer.append(buffer);
				
					// step through new entry and update the line count buffer used by ImGui
					for (int new_buffer_size = m_imgui_textbuffer.size(); old_buffer_size < new_buffer_size; old_buffer_size++)
					{
						if (m_imgui_textbuffer[old_buffer_size] == '\n')
							m_imgui_line_offset.push_back(old_buffer_size + 1);
					}

					// release memory
					delete[] buffer;

					// update log size
					m_imgui_log_last_size += new_bytes;
				}

				file.close();
			}

		#endif

		// Logs a message to specified file. Utilizes the __FUNCDNAME__, __FUNSIG__
		// MSVC preproessor directives to acquire the callers function name as
		// a const char *. As defined by the macro above, the address of the caller
		// function may also be included.
		template<typename T, typename ...Args>
		static void log(const char *_func, const T &_output_item, Args ...args)
		{
			std::string fnc(_func);

			#ifndef _WIN_32
			//
			// format func for Linux output, examples:
			// "static void FluidFramebuffer::resize(uint32_t, uint32_t)::RenderCommand89::execute(void *): Framebuffer 'front_velocity'..."
			// "static void Syn::ShaderLibrary::add(const std::string &, const Ref<Syn::Shader> &): shader 'fluidJacobi' added to library."
			// "int Syn::Window::init(bool): OpenGL renderer: Mesa Intel(R) UHD Graphics 620 (KBL GT2)"
			// "Syn::Font::Font(const char *, const int &, const Ref<Syn::Shader> &): using static shaders."
			//
			size_t firstParanthesis = fnc.find('(');
			// reverse find 1) the start of the fnc, or 2) the space before the fnc name
			size_t lastSpace = fnc.substr(0, firstParanthesis).rfind(' ');
			// use the substring leading up to first paranthesis (since this has shifted, adjustment by lastSpace).
			std::string fncName = lastSpace == std::string::npos ? fnc.substr(0, firstParanthesis) : fnc.substr(lastSpace+1, firstParanthesis-lastSpace-1);
			// remove argument types, replace with '()'.
			fnc = fncName.substr(0, firstParanthesis) + "()";
			#endif

			std::string out = "[" + Time::current_time() + "] " + fnc;
			out.append(": ");

			*m_logFile << out;

			if (m_bUseStdOut)
				std::cout << out;

			log_msg(_output_item, args...);

			if (m_bNewline)
			{
				*m_logFile << std::endl;
				if (m_bUseStdOut)
					std::cout << std::endl;
			}

			m_logFile->flush();
		}


		// Similar as above, but without __FUNCDNAME___.
		template<typename T, typename ...Args>
		static void log_no_func(const T &_output_item, Args ...args)
		{
			std::string out = "[" + Time::current_time() + "] " + _output_item;
			out.append(": ");

			*m_logFile << out;

			if (m_bUseStdOut)
				std::cout << out;

			log_msg(args...);

			if (m_bNewline)
			{
				*m_logFile << std::endl;
				if (m_bUseStdOut)
					std::cout << std::endl;
			}
			m_logFile->flush();
			
		}


		// Recursive parameter unpacking of variadic arguments.
		template<typename T, typename ...Args>
		static void log_msg(const T &_output_item, Args ...args)
		{
			*m_logFile << _output_item;

			if (m_bUseStdOut)
				std::cout << _output_item;

			log_msg(args...);
		}

		// Final parameter
		template<typename T>
		static void log_msg(const T &_output_item)
		{
			*m_logFile << _output_item;

			if (m_bUseStdOut)
				std::cout << _output_item;
		}


		// accessors
		static uint32_t getErrorCount() { return s_errorCount; }
		static uint32_t getWarningCount() { return s_warningCount; }
		static void increaseErrorCount() { s_errorCount++; }
		static void increaseWarningCount() { s_warningCount++; }
		static bool initialized() { return m_bInitialized; }


		#ifdef _WIN64
			inline static const void setErrorColor() 	{ SetConsoleTextAttribute(m_consoleHandle, m_colorError); }
			inline static const void setWarningColor() 	{ SetConsoleTextAttribute(m_consoleHandle, m_colorWarning); }
			inline static const void setCoreColor() 	{ SetConsoleTextAttribute(m_consoleHandle, m_colorCore); }
			inline static const void setClientColor() 	{ SetConsoleTextAttribute(m_consoleHandle, m_colorClient); }
			inline static const void resetColor() 		{ SetConsoleTextAttribute(m_consoleHandle, m_colorNormal); }
			inline static const void setOkColor() 		{ SetConsoleTextAttribute(m_consoleHandle, m_colorOk); }
		#else
			inline static const void setErrorColor() 	{ std::cout << "\x1b[1;37;41m"; }	// bold white text, red background
			inline static const void setWarningColor() 	{ std::cout << "\x1b[0;30;42m"; }	// regular, black text, cyan background
			inline static const void setCoreColor() 	{ std::cout << "\x1b[0m"; }			// regular, unformatted color
			// inline static const void setCoreColor() 	{ std::cout << "\x1b[1m"; }			// bold, unformatted color
			// inline static const void setClientColor() 	{ std::cout << "\x1b[0m"; }			// regular, unformatted color
			inline static const void setClientColor() 	{ std::cout << "\x1b[0;33m"; }		// regular, yellow text
			inline static const void resetColor() 		{ std::cout << "\x1b[0m"; }			// regular, unformatted color
			inline static const void setOkColor() 		{ std::cout << "\x1b[0;32m"; }		// regular, green text
		#endif


	private:
		static std::fstream *m_logFile;
		static std::string m_filename;
		static bool m_bInitialized;
		// flags
		static bool m_bNewline;
		static bool m_bUseStdOut;

		#ifdef DEBUG_IMGUI_LOG
			static uint32_t m_imgui_log_last_size;
			static ImGuiTextBuffer m_imgui_textbuffer;
			static ImVector<int> m_imgui_line_offset;
		#endif

		// console colors
		#ifdef _WIN64
			static HANDLE m_consoleHandle;
			static WORD m_colorNormal;
			static WORD m_colorCore;
			static WORD m_colorError;
			static WORD m_colorWarning;
			static WORD m_colorClient;
			static WORD m_colorOk;
		#else // linux
			static const char* m_terminalStartToken;
			static const char* m_terminalEndToken;	// reset color
		#endif

		// warning / error counters
		static uint32_t s_errorCount;
		static uint32_t s_warningCount;

		// debug buffers
		static char s_debug_buffer[1024];
		static char s_debug_func_buffer[512];

		//
		static __attribute__((always_inline)) void fmt_debug_func(const char *_func)
		{
			memset(s_debug_func_buffer, 0, 512);
			std::sprintf(s_debug_func_buffer, "%s: %s", "DEBUG", _func);
		}

		// helper function for matrix and vector debug
		static inline const char* ff(float _f) { return (_f < 0.0f ? "-" : " "); }
		static inline const char* fi(int _i) { return (_i < 0 ? "-" : " "); }


	};


}

