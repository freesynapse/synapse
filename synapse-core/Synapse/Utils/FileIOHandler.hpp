#pragma once


#include <string>
#include <vector>


namespace Syn { 


	// static class
	class FileIOHandler
	{
	public:
		static void shutdown();

		static int tell_file_size(const std::string& _file_path);
		static int write_buffer_to_file(const std::string& _file_path, const std::string& _buffer, bool _keep_file=false);
		static int read_file_to_buffer(const std::string& _file_path, std::vector<unsigned char> &_buffer);
		static int read_file_to_buffer(const std::string& _file_path, std::string &_buffer);


	private:
		static std::vector<std::string> m_createdFiles;

	};


}

