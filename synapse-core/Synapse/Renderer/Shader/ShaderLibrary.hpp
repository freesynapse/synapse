#pragma once


#include "Synapse/Renderer/Shader/Shader.hpp"
#include "Synapse/Core.hpp"
#include "Synapse/Renderer/Renderer.hpp"


namespace Syn {


	class ShaderLibrary
	{
	public:
		static void initFromFileList(const std::string& _shader_file_list);
		static void add(const Ref<Shader>& _shader);
		static void add(const std::string& _name, const Ref<Shader>& _shader);
		static void load(const std::string& _file_path);
		static void load(const std::string& _name, const std::string& _file_path);
		static void reload(const std::string& _name);
		static void reload(const Ref<Shader>& _shader);

		// accessors
		static inline Ref<Shader> getDefaultInstance() 
		{
			static Ref<Shader> s_defaultInstance = createDefaultShader();
			return s_defaultInstance;
		}
		//
		static inline Ref<Shader> get(const std::string& _name)
		{
			if (!exists(_name) || !s_shaders[_name]->isLoaded())
			{
				SYN_CORE_WARNING("shader '", _name, "' not found, returning defualt static shader instance.");
				return getDefaultInstance();
			}
			return s_shaders[_name];
		}
		//
		static inline Ref<Shader> getShader(const std::string& _name)
		{
			if (!exists(_name) || !s_shaders[_name]->isLoaded())
				return nullptr;
			return s_shaders[_name];
		}
		//
		static const void debugListShaders();
		static std::unordered_map<std::string, Ref<Shader>> debugGetShaders() { return s_shaders; }

	private:
		static bool exists(const std::string& _name);
		static Ref<Shader> createDefaultShader();


	private:
		static std::unordered_map<std::string, Ref<Shader>> s_shaders;


	};


}
