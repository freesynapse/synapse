#pragma once


#include <vector>
#include <string>

#include "../../Core.hpp"


namespace Syn {	

	//
	class Shader
	{
	public:
		friend class ShaderLibrary;

	public:
		Shader() {}
		Shader(const std::string& _shader_file_path);
		Shader(const std::string& _shader_name, const std::string& _file_path);
		~Shader();

		void reload();

		void loadFromFile();
		void loadFromSource(const std::string &_name, const std::string &_src);

	protected:
		std::unordered_map<GLenum, std::string> preprocess(const std::string& _source);
		std::vector<std::string> parseUniforms();
		int compileShader();
		void resolveUniforms(const std::vector<std::string>& _uniforms);

	public:
		void enable();
		void disable();
		//void dispose();
		//void addUniform(const std::string& _uniform_name);
		void printUniforms();

		// accessors -- more below
		GLint getUniformLocation(const std::string& _uniform_name);
		const GLuint getShaderID() { return m_shaderID; }
		const std::string& getName() { return m_shaderName; }
		const bool isLoaded() { /*SYN_CORE_TRACE(m_shaderName, " - m_loaded = ", m_loaded);*/ return m_loaded; }


	protected:
		std::string m_shaderName = "";
		std::string m_assetPath = "";
		std::string m_rawSrc = "";
		std::unordered_map<GLenum, std::string> m_shaderSrc;
		std::unordered_map<std::string, GLint> m_uniforms;
		bool m_loaded = false;

		GLuint m_shaderID = 0;


	public:
		// accessors -- continued
		void setUniform1i(const GLint& _location, const int& _i);
		void setUniform1f(const GLint& _location, const float& _f);
		void setUniform2iv(const GLint& _location, const glm::ivec2& _v);
		void setUniform2fv(const GLint& _location, const glm::vec2& _v);
		void setUniform3fv(const GLint& _location, const glm::vec3& _v);
		void setUniform4fv(const GLint& _location, const glm::vec4& _v);
		void setMatrix2fv(const GLint& _location, const glm::mat2& _mat);
		void setMatrix3fv(const GLint& _location, const glm::mat3& _mat);
		void setMatrix4fv(const GLint& _location, const glm::mat4& _mat);

		void setUniform1i(const std::string& _name, const int& _i) { setUniform1i(getUniformLocation(_name), _i); }
		void setUniform1f(const std::string& _name, const float& _f) { setUniform1f(getUniformLocation(_name), _f); }
		void setUniform2iv(const std::string& _name, const glm::ivec2& _v) { setUniform2iv(getUniformLocation(_name), _v); }
		void setUniform2fv(const std::string& _name, const glm::vec2& _v) { setUniform2fv(getUniformLocation(_name), _v); }
		void setUniform3fv(const std::string& _name, const glm::vec3& _v) { setUniform3fv(getUniformLocation(_name), _v); }
		void setUniform4fv(const std::string& _name, const glm::vec4& _v) { setUniform4fv(getUniformLocation(_name), _v); }
		void setMatrix2fv(const std::string& _name, const glm::mat2& _mat) { setMatrix2fv(getUniformLocation(_name), _mat); }
		void setMatrix3fv(const std::string& _name, const glm::mat3& _mat) { setMatrix3fv(getUniformLocation(_name), _mat); }
		void setMatrix4fv(const std::string& _name, const glm::mat4& _mat) { setMatrix4fv(getUniformLocation(_name), _mat); }

	};


}


