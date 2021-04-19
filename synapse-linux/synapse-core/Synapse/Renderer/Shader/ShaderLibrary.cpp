
#include "pch.hpp"

#include "Synapse/Renderer/Shader/ShaderLibrary.hpp"
#include "Synapse/Renderer/Renderer.hpp"


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
	void ShaderLibrary::add(const Ref<Shader>& _shader)
	{
		std::string name = _shader->getName();
		add(name, _shader);
	}


	//-----------------------------------------------------------------------------------
	void ShaderLibrary::add(const std::string& _name, const Ref<Shader>& _shader)
	{
		if (exists(_name))
		{
			SYN_CORE_WARNING("shader '", _name, "' already exists.");
			return;
		}
		
		s_shaders[_name] = _shader;
		
		Renderer::get().executeRenderCommands();

		SYN_CORE_TRACE("shader '", _name, "' added to library", (_shader->isLoaded() ? "." : " (compilation failed)."));
	}


	//-----------------------------------------------------------------------------------
	void ShaderLibrary::load(const std::string& _file_path)
	{
		auto shader = MakeRef<Shader>(_file_path);
		add(shader);
	}


	//-----------------------------------------------------------------------------------
	void ShaderLibrary::load(const std::string& _name, const std::string& _file_path)
	{
		auto shader = MakeRef<Shader>(_file_path);
		add(_name, shader);
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

		FileIOHandler::write_buffer_to_file("./static_default_shader.glsl", src);
		Ref<Shader> shader = MakeRef<Shader>("./static_default_shader.glsl");

		return shader;

	}


	//-----------------------------------------------------------------------------------
	const void ShaderLibrary::debugListShaders()
	{
		SYN_CORE_TRACE("shaders in library: ");
		for (auto& it : s_shaders)
		{
			SYN_CORE_TRACE("name: ", it.first, " -- ", it.second);
		}
	}


}