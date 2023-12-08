
#include "../../../pch.hpp"

#include "ShaderLibrary.hpp"
#include "../Renderer.hpp"

#include "../../Utils/FileIOHandler.hpp"


namespace Syn {

	// static declarations
	std::unordered_map<std::string, Ref<Shader>> ShaderLibrary::s_shaders;
	
	//-----------------------------------------------------------------------------------
	// TODO: implement when need arises
	void ShaderLibrary::initFromFileList(const std::string& _shader_file_list)
	{
		if (_shader_file_list == "")
		{
			SYN_CORE_WARNING("no file provided.");
			return;
		}
	}

	//-----------------------------------------------------------------------------------
	std::string ShaderLibrary::add(const Ref<Shader>& _shader)
	{
		std::string name = _shader->getName();
		return add(name, _shader);

	}

	//-----------------------------------------------------------------------------------
	std::string ShaderLibrary::add(const std::string& _name, const Ref<Shader>& _shader)
	{
		if (exists(_name))
		{
			SYN_CORE_TRACE("shader '", _name, "' already exists.");
			return "";

		}
		
		s_shaders[_name] = _shader;
		
		Renderer::get().executeRenderCommands();

		SYN_CORE_TRACE("shader '", _name, "' added to library", (_shader->isLoaded() ? "." : " (compilation failed)."));
		return _name;

	}

	//-----------------------------------------------------------------------------------
	const Ref<Shader>& ShaderLibrary::load(const std::string& _file_path)
	{
		auto shader = MakeRef<Shader>(_file_path);
		auto name = add(shader);
		return s_shaders[name];

	}

	//-----------------------------------------------------------------------------------
	const Ref<Shader>& ShaderLibrary::load(const std::string& _name, const std::string& _file_path)
	{
		auto shader = MakeRef<Shader>(_file_path);
		add(_name, shader);
		return s_shaders[_name];

	}

	//-----------------------------------------------------------------------------------
	const Ref<Shader>& ShaderLibrary::load(const FileName &_fn0, const FileName &_fn1)
	{
		return load("", _fn0, _fn1);

	}

	//-----------------------------------------------------------------------------------
	const Ref<Shader>& ShaderLibrary::load(const std::string& _name, 
										   const FileName &_fn0, 
										   const FileName &_fn1)
	{
		std::string vert, vert_file, frag, frag_file;
		vert_file = _fn0.file_name;
		frag_file = _fn1.file_name;
		
		FileIOHandler::read_file_to_buffer(vert_file, vert);
		FileIOHandler::read_file_to_buffer(frag_file, frag);

		// deduce which is the vertex shader
		if (vert.find("#VERTEX") == std::string::npos && 
			vert.find("#vertex") == std::string::npos)
		{
			std::swap(vert, frag);
			std::swap(vert_file, frag_file);
		}

		// if no name is provided, shader name will be comprised of the combination of 
		// the vertex and fragment filenames
		std::string name = _name;
		if (strcmp(name.c_str(), "") == 0)
			name = extractNameFromFilePath(vert_file) + '_' + \
				   extractNameFromFilePath(frag_file) + "_shader";

		return loadFromSrc(name, vert + frag);

	}

	//-----------------------------------------------------------------------------------
	const Ref<Shader>& ShaderLibrary::loadFromSrc(const std::string &_name, const std::string &_src)
	{
		auto shader = MakeRef<Shader>();
		shader->loadFromSource(_name, _src);
		add(_name, shader);
		return s_shaders[_name];

	}

	//-----------------------------------------------------------------------------------
	const Ref<Shader>& ShaderLibrary::loadFromSrc(const std::string &_name, 
								   				  const std::string &_vert_src, 
								   				  const std::string &_frag_src)
	{
		auto shader = MakeRef<Shader>();
		shader->loadFromSource(_name, _vert_src+_frag_src);
		add(_name, shader);
		return s_shaders[_name];

	}

	//-----------------------------------------------------------------------------------
	void ShaderLibrary::reload(const std::string& _name)
	{
		SYN_CORE_TRACE("reloading shader '", _name, "'.");

		auto& shader = s_shaders[_name];
		shader->reload();
		Renderer::get().executeRenderCommands();
		
	}

	//-----------------------------------------------------------------------------------
	void ShaderLibrary::reload(const Ref<Shader>& _shader)
	{
		SYN_CORE_TRACE("reloading shader '", _shader->getName(), "'.");

		auto& shader = s_shaders[_shader->getName()];
		shader->reload();
		Renderer::get().executeRenderCommands();

	}

	//-----------------------------------------------------------------------------------
	bool ShaderLibrary::exists(const std::string& _name)
	{
		return s_shaders.find(_name) != s_shaders.end();

	}

	//-----------------------------------------------------------------------------------
	std::string ShaderLibrary::extractNameFromFilePath(const std::string _fp)
	{
		size_t lastSlash = _fp.find_last_of("/\\");
		lastSlash = (lastSlash == std::string::npos ? _fp.size() : lastSlash + 1);
		size_t lastDot = _fp.rfind(".");
		lastDot = (lastDot == std::string::npos ? _fp.size() : lastDot);
		return _fp.substr(lastSlash, (lastDot - lastSlash));

	}

	//-----------------------------------------------------------------------------------
	Ref<Shader> ShaderLibrary::createDefaultShader()
	{
		SYN_CORE_TRACE("creating static default shader.");

		std::string src = R"(
			#type VERTEX_SHADER
			#version 330 core

			layout(location = 0) in vec4 a_position;

			uniform mat4 u_view_projection_matrix = mat4(1.0f);
			uniform mat4 u_model_matrix = mat4(1.0f);

			void main() {
				gl_Position = u_view_projection_matrix * u_model_matrix * a_position;
			}

			#type FRAGMENT_SHADER
			#version 330 core

			layout(location = 0) out vec4 out_color;

			void main() {
				out_color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
			}
		)";

		// FileIOHandler::write_buffer_to_file("./static_default_shader.glsl", src);
		// Ref<Shader> shader = MakeRef<Shader>("./static_default_shader.glsl");
		auto shader = loadFromSrc("static_default_shader", src);

		return shader;

	}

	//-----------------------------------------------------------------------------------
	const void ShaderLibrary::__debug_ListShaders()
	{
		SYN_CORE_TRACE("shaders in library: ");
		for (auto& it : s_shaders)
		{
			SYN_CORE_TRACE("name: ", it.first, " -- ", it.second);
		}
	}

}