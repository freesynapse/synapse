
#include "pch.hpp"

#include "Synapse/Renderer/Shader/Shader.hpp"
#include "Synapse/Renderer/Renderer.hpp"
#include "Synapse/Core.hpp"


namespace Syn {	


	static GLenum shader_type_from_str(const std::string& _type)
	{
		if (_type == "VERTEX" || _type == "VERTEX_SHADER" || _type == "vertex")		
			return GL_VERTEX_SHADER;
		if (_type == "FRAGMENT" || _type == "FRAGMENT_SHADER" || _type == "fragment" || _type == "pixel")	
			return GL_FRAGMENT_SHADER;
		if (_type == "GEOMETRY" || _type == "GEOMETRY_SHADER" || _type == "geometry")
			return GL_GEOMETRY_SHADER;
		return GL_NONE;
	}

	//-----------------------------------------------------------------------------------
	static std::string shader_str_from_type(GLenum _type)
	{
		std::string ret;
		switch (_type)
		{
			case GL_VERTEX_SHADER: 		ret = "VERTEX_SHADER"; 		break;
			case GL_FRAGMENT_SHADER:	ret = "FRAGMENT_SHADER";	break;
			case GL_GEOMETRY_SHADER:	ret = "GEOMETRY_SHADER";	break;
			default:					ret = "GL_NONE";			break;
		};

		return ret;
	}

	//-----------------------------------------------------------------------------------
	static std::string g_str;
	inline const char* leading_blank_spaces(int _line_num, int _error_code=0)
	{
		/* error code 1 = error, 2 = warning */
		
		// obs: max 10000 lines of shader code permitted
		int n = 0;
		// get number of digits in _line_num
		int numChars = _line_num > 0 ? (int)log10((double)_line_num) + 1 : 1;
		switch (_error_code)
		{
			case 0: g_str = "   "; break;
			case 1: g_str = "  E"; break;
			case 2: g_str = "  W"; break;
		}
		
		for (int i = numChars; i < 5; i++)
			g_str.append(" ");

		return g_str.c_str();
	}

	//-----------------------------------------------------------------------------------
	/* 
	Function for parsing the error message for an erronous shader source (failed
	compilation). The source is outputted and the lines raising errors are marked.
	*/
	void annotateShaderErrorMsg(const std::string& _source, const std::string& _error_msg)
	{
		// error messages line number are given as 0:line_number(character_number). 
		// First, let's find that line number.
		// Example:
		// 0:19(2): error: value of type vec4 cannot be assigned to variable of type vec2
		
		// map of the line number and the error code (0 = error, 1 = warning)
		std::map<int, int> errorLinesMap;
		std::istringstream errorLineStream(_error_msg);
		std::string line;
		// split error message into lines and parse each line for 
		// erronous line and error code (warning or error).
		while (std::getline(errorLineStream, line, '\n'))
		{
			size_t lineNumStart = line.find('(');
			size_t lineNumEnd =  line.find(':') - 1;
			if (lineNumStart == std::string::npos || lineNumEnd == std::string::npos)
				continue;
			
			// extract the line number
			std::string s = line.substr(line.find(':')+1, lineNumEnd - lineNumStart);
			int errLineNumber = atoi(s.c_str());
			
			// find if warning or error
			int errCode;
			if (line.find("error") != std::string::npos)
				errCode = 1;
			else if (line.find("warning") != std::string::npos)
				errCode = 2;
			else
				errCode = 0;

			errorLinesMap.insert(std::pair<int, int>(errLineNumber, errCode));
		}

		// read the source line by line
		std::vector<std::string> srcLines;
		std::istringstream sstream(_source);
		int lineNumber = 1;
		
		while (std::getline(sstream, line, '\n'))
		{
			// add leading blanks and the line number to each line.
			// the erronous line will be marked with 'E' or 'W'.

			// scan the marked errors for the current line number
			int errCode = 0;
			for (auto& item : errorLinesMap)
			{
				if (item.first == lineNumber)
					errCode = item.second;
			}

			// create a line prefix with line number and optional 'E' or 'W'
			std::string linePrefix = std::string(leading_blank_spaces(lineNumber, errCode));
			linePrefix += std::to_string(lineNumber);
			linePrefix += ": ";

			line.insert(0, linePrefix);
			srcLines.push_back(line);
			lineNumber++;
			
		}

		// concatenate message for logging
		std::string logMsg = "";
		for (auto& line : srcLines)
			logMsg = logMsg + line + '\n';

		Log::log_msg(logMsg);

	}

	//-----------------------------------------------------------------------------------
	// Shader class functions
	//-----------------------------------------------------------------------------------
	Shader::Shader(const std::string& _shader_file_path) : 
		m_assetPath(_shader_file_path)
	{
		// extract name from path
		size_t lastSlash = _shader_file_path.find_last_of("/\\");
		lastSlash = (lastSlash == std::string::npos ? _shader_file_path.size() : lastSlash + 1);
		size_t lastDot = _shader_file_path.rfind(".");
		lastDot = (lastDot == std::string::npos ? _shader_file_path.size() : lastDot);
		m_shaderName = _shader_file_path.substr(lastSlash, (lastDot - lastSlash));

		reload();

	}

	//-----------------------------------------------------------------------------------
	Shader::Shader(const std::string& _shader_name, const std::string& _file_path) :
		m_assetPath(_file_path), m_shaderName(_shader_name)
	{
		reload();
	}

	//-----------------------------------------------------------------------------------
	Shader::~Shader()
	{
		if (m_shaderID)
		{
			SYN_RENDER_S0({
				glDeleteProgram(self->m_shaderID);
			});
		}
	}

	//-----------------------------------------------------------------------------------
	void Shader::reload()
	{
		// reset setup flag
		m_loaded = false;

		// read shader from file
		std::string src;
		int result = FileIOHandler::read_file_to_buffer(m_assetPath, src);
		if (result != RETURN_SUCCESS)
		{
			SYN_CORE_TRACE("shader not loaded, could not open file '", m_assetPath, "'.");
			return;
		}

		// preprocess, i.e. get vertex and fragment shaders separated
		m_shaderSrc = preprocess(src);

		// parse all uniforms for resolving below, threaded
		std::vector<std::string> uniforms = parseUniforms();

		SYN_RENDER_S1(uniforms, {
			// clean the slate
			if (self->m_shaderID)
				glDeleteProgram(self->m_shaderID);

			// compile the shader program
			int res = self->compileShader();
			if (res != RETURN_SUCCESS)
			{
				SYN_CORE_WARNING(self->m_shaderName, ": couldn't compile shader.");
				return;
			}

			// resolve uniforms
			self->resolveUniforms(uniforms);

			// update flag
			self->m_loaded = true;
		});

	}

	//-----------------------------------------------------------------------------------
	boost::unordered_map<GLenum, std::string> Shader::preprocess(const std::string& _source)
	{
		boost::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t lenToken = strlen(typeToken);
		// find first token
		size_t pos = _source.find(typeToken, 0);

		while (pos != std::string::npos)
		{
			size_t eol = _source.find_first_of("\r\n", pos);
			size_t begin = pos + lenToken + 1;
			GLenum type = shader_type_from_str(_source.substr(begin, eol - begin));
			if (type == GL_NONE)
			{
				SYN_CORE_WARNING("unknown shader type -- loading default backup shader.");
				m_loaded = false;
				break;
			}

			size_t nextLinePos = _source.find_first_not_of("\r\n", eol);
			pos = _source.find(typeToken, nextLinePos);
			shaderSources[type] = _source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? _source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	//-----------------------------------------------------------------------------------
	std::vector<std::string> Shader::parseUniforms()
	{
		/* Returns RETURN_FAILURE (-1), on failure, of ok returns number
		 * of uniforms parsed.
		 */

		const char* uniToken = "uniform";
		size_t lenToken = strlen(uniToken);
		int n_uniforms = 0;
		std::vector<std::string> uniforms;

		for (auto& kv : m_shaderSrc)
		{
			std::string& src = kv.second;
			size_t pos = src.find(uniToken, 0);	// first uniform

			while (pos != std::string::npos)
			{
				size_t eol = src.find_first_of("\r\n", pos);
				std::string str_type_and_name = src.substr(pos + lenToken, eol - pos - lenToken);

				// split on ' ' and extract last item
				std::vector<std::string> split_str;
				size_t prev = 0;
				size_t curr = str_type_and_name.find(' ');
				while (curr != std::string::npos)
				{
					split_str.push_back(str_type_and_name.substr(prev, curr - prev));
					prev = curr + 1;
					curr = str_type_and_name.find(' ', prev);
				}
				std::string last = str_type_and_name.substr(prev, curr - prev);
				// remove trailing ';'
				last.pop_back();
				split_str.push_back(last);

				if (split_str[2] != "")
				{
					uniforms.push_back(split_str[2]);
					n_uniforms++;
				}
				//
				size_t nextLinePos = src.find_first_not_of("\r\n", eol);
				pos = src.find(uniToken, nextLinePos);
			}
		}

		return uniforms;

	}

	//-----------------------------------------------------------------------------------
	int Shader::compileShader()
	{
		std::array<GLuint, 4> shaderIDs;
		int index = 0;
		GLuint program;

		// create the shader program server side
		program = glCreateProgram();
		#ifdef DEBUG_SHADER_SETUP
			SYN_CORE_TRACE("creating shader ", m_shaderName, " [", program, "].");
		#endif

		// step through each key-value pair in shader sources
		for (auto& kv : m_shaderSrc)
		{
			GLenum type = kv.first;
			std::string& src = kv.second;

			// create and compile shader of type from source
			GLuint shaderID = glCreateShader(type);
			const GLchar* srcCstr = (const GLchar*)src.c_str();
			glShaderSource(shaderID, 1, &srcCstr, 0);

			glCompileShader(shaderID);

			// compilation status
			GLint isCompiled;
			glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint len = 0;
				glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &len);
				std::vector<char> errorLog(len);
				glGetShaderInfoLog(shaderID, len, &len, errorLog.data());

				std::string msg = "";
				for (auto c : errorLog)
					msg += c;
				SYN_CORE_ERROR(msg);

				SYN_CORE_ERROR("Annotated source '", m_shaderName, "' (", shader_str_from_type(type) ,"): ");
				annotateShaderErrorMsg(src, msg);

				// prevent mem leak
				glDeleteShader(shaderID);

				return RETURN_FAILURE;
			}

			shaderIDs[index++] = shaderID;
			glAttachShader(program, shaderID);

		}

		// link the program
		#ifdef DEBUG_SHADER_SETUP
			SYN_CORE_TRACE("linking program [", program, "].");
		#endif

		glLinkProgram(program);

		// error checking
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint len;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
			std::vector<char> errorLog(len);

			glGetProgramInfoLog(program, len, &len, errorLog.data());

			std::string msg = "";
			for (auto c : errorLog)
				msg += c;
			SYN_CORE_ERROR(msg);

			// release program and shaders
			glDeleteProgram(program);

			for (auto id : shaderIDs)
				glDeleteShader(id);

			return RETURN_FAILURE;
		}

		// detach and delete shaders after linking
		for (auto id : shaderIDs)
			glDetachShader(program, id);


		m_shaderID = program;

		return RETURN_SUCCESS;

	}

	//-----------------------------------------------------------------------------------
	void Shader::resolveUniforms(const std::vector<std::string>& _uniforms)
	{
		for (auto uniform : _uniforms)
		{
			GLint i = glGetUniformLocation(m_shaderID, uniform.c_str());
			if (i == -1)
			{
				#ifdef DEBUG_UNIFORMS
					SYN_CORE_WARNING(m_shaderName, ": ", uniform, ": -1");
				#endif
			}
			else
			{
				#ifdef DEBUG_UNIFORMS
					SYN_CORE_TRACE(m_shaderName, ": ", uniform, ": ", i);
				#endif
			}

			m_uniforms[uniform] = i;
		}
	}

	//-----------------------------------------------------------------------------------
	GLint Shader::getUniformLocation(const std::string& _uniform_name)
	{
		boost::unordered_map<std::string, GLint>::const_iterator found = m_uniforms.find(_uniform_name);
		if (found == m_uniforms.end())
		{
			#ifdef DEBUG_UNIFORMS
				SYN_CORE_WARNING("uniform ", _uniform_name, " not found.");
			#endif

			return -1;
		}
		return found->second;
	}

	//-----------------------------------------------------------------------------------
	void Shader::enable()
	{
		SYN_RENDER_S0({
			//SYN_CORE_TRACE("enabling shader.");
			glUseProgram(self->m_shaderID);
		});
	}

	//-----------------------------------------------------------------------------------
	void Shader::disable()
	{
		SYN_RENDER_S0({
			glUseProgram(0);
		});
	}

	//-----------------------------------------------------------------------------------
	void Shader::printUniforms()
	{
		std::string str = "shader '"+ m_shaderName +"' uniforms:\n";
		for (auto& it : m_uniforms)
			str += '\t' + it.first + '\n';
		str = str.substr(0, str.size()-1);
		SYN_CORE_TRACE(str);
	}

	//-----------------------------------------------------------------------------------
	// Uniform accessors
	//-----------------------------------------------------------------------------------
	void Shader::setUniform1i(const GLint& _location, const int& _i) 
	{
		SYN_RENDER_S2(_location, _i, {
			glUniform1i(_location, _i);
		});
	}
	void Shader::setUniform1f(const GLint& _location, const float& _f) 
	{
		SYN_RENDER_S2(_location, _f, {
			glUniform1f(_location, _f);
		});
	}
	void Shader::setUniform2iv(const GLint& _location, const glm::ivec2& _v) 
	{
		SYN_RENDER_S2(_location, _v, {
			glUniform2iv(_location, 1, (GLint*)(&_v));
		});
	}
	void Shader::setUniform2fv(const GLint& _location, const glm::vec2& _v) 
	{
		SYN_RENDER_S2(_location, _v, {
			glUniform2fv(_location, 1, (GLfloat*)(&_v));
		});
	}
	void Shader::setUniform3fv(const GLint& _location, const glm::vec3& _v) 
	{
		SYN_RENDER_S2(_location, _v, {
			glUniform3fv(_location, 1, (GLfloat*)(&_v));
		});
	}
	void Shader::setUniform4fv(const GLint& _location, const glm::vec4& _v) 
	{
		SYN_RENDER_S2(_location, _v, {
			glUniform4fv(_location, 1, (GLfloat*)(&_v));
		});
	}
	void Shader::setMatrix3fv(const GLint& _location, const glm::mat3& _mat) 
	{
		SYN_RENDER_S2(_location, _mat, {
			glUniformMatrix3fv(_location, 1, GL_FALSE, (GLfloat*)& _mat);
		});
	}
	void Shader::setMatrix4fv(const GLint& _location, const glm::mat4& _mat) 
	{
		SYN_RENDER_S2(_location, _mat, {
			//SYN_CORE_TRACE("uploading matrix4.");
			glUniformMatrix4fv(_location, 1, GL_FALSE, (GLfloat*)& _mat);
		});
	}

}

