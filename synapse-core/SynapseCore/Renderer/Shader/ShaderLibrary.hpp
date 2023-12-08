#pragma once


#include "Shader.hpp"
#include "../../Core.hpp"
#include "../Renderer.hpp"


namespace Syn
{
	struct FileName
	{
		std::string file_name;
		FileName() : file_name("") {}
		FileName(const std::string &_file_name) : file_name(_file_name) {}
	};

	//
	class ShaderLibrary
	{
	public:
		static void initFromFileList(const std::string& _shader_file_list);
		static std::string add(const Ref<Shader>& _shader);
		static std::string add(const std::string& _name, const Ref<Shader>& _shader);
		
		/* Loads and compiles shader from file path. If no name is provided it will be
		 * deduced from the filename.
		 */
		static const Ref<Shader>& load(const std::string& _file_path);
		static const Ref<Shader>& load(const std::string& _name, const std::string& _file_path);
		
		/* Loads from separate files, and the vertex and fragment sources will be deduced
		 * and compiled appropriately.
		 */
		static const Ref<Shader>& load(const FileName &_fn0, const FileName &_fn1);
		static const Ref<Shader>& load(const std::string& _name, const FileName &_fn0, const FileName &_fn1);
		
		/* Compiles shaders from source. Shader id (_name) is mandatory.
		 */
		static const Ref<Shader>& loadFromSrc(const std::string &_name, const std::string &_src);
		static const Ref<Shader>& loadFromSrc(const std::string &_name, const std::string &_vert_src, const std::string &_frag_src);
		
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
		static const void __debug_ListShaders();
		static std::unordered_map<std::string, Ref<Shader>> __debug_GetShaders() { return s_shaders; }

	private:
		static bool exists(const std::string& _name);
		static std::string extractNameFromFilePath(const std::string _fp);
		static Ref<Shader> createDefaultShader();

	private:
		static std::unordered_map<std::string, Ref<Shader>> s_shaders;


	};


}
