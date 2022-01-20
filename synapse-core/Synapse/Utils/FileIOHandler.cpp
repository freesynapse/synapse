
#include "pch.hpp"

#ifdef _WIN64
#include <fileapi.h>
#else
#include <fcntl.h>
#include <errno.h>
#endif

#include "Synapse/Utils/FileIOHandler.hpp"
#include "Synapse/Core.hpp"
#include "Synapse/Event/EventHandler.hpp"
#include "Synapse/Event/Event.hpp"
#include "Synapse/Debug/Error.hpp"


namespace Syn {	


	// static declarations
	std::vector<std::string> FileIOHandler::m_createdFiles;


	//-----------------------------------------------------------------------------------
	#ifdef _WIN64
	void FileIOHandler::shutdown()
	{
		for (auto file : m_createdFiles)
		{
			SYN_CORE_TRACE("deleting file '", file, "'.");
			if (!DeleteFileA(file.c_str()))
			{
				std::string error = "could not delete file '" + file + "': ";
				DWORD code = GetLastError();
				switch (code)
				{
					case ERROR_FILE_NOT_FOUND:
						SYN_CORE_WARNING(error, "ERROR_FILE_NOT_FOUND.");
						break;
					case ERROR_ACCESS_DENIED:
						SYN_CORE_WARNING(error, "ERROR_ACCESS_DENIED.");
						break;
					default:
						SYN_CORE_WARNING(error, "unknown error code (", code, ").");
				}
			}
		}
	}
	#else
	void FileIOHandler::shutdown()
	{
		for (auto file : m_createdFiles)
		{
			SYN_CORE_TRACE("deleting file '", file, "'.");
			if (remove(file.c_str()) != 0)
			{
				std::string error = "could not delete file '" + file + "': ";
				SYN_CORE_WARNING(error, " : ", errno, " -- ", strerror(errno));
			}
		}
	}
	#endif

	//-----------------------------------------------------------------------------------
	int FileIOHandler::tell_file_size(const std::string& _file_path)
	{
		std::ifstream file(_file_path, std::ios::binary);

		file.seekg(0, std::ios::end);
		int fileSize = (int)file.tellg();
		file.close();

		return fileSize;
	}

	
	//-----------------------------------------------------------------------------------
	int FileIOHandler::write_buffer_to_file(const std::string& _file_path, const std::string& _buffer, bool _keep_file)
	{
		std::ofstream file(_file_path, std::ios::binary | std::ios::trunc);

		if (file.fail() || !file.is_open())
		{
			SYN_CORE_ERROR("File '", _file_path, "' could not be opened/created.");
			return RETURN_FAILURE;
		}

		file << _buffer;
		file.flush();

		file.close();

		if (!_keep_file)
			m_createdFiles.push_back(_file_path);

		return RETURN_SUCCESS;

	}


	//-----------------------------------------------------------------------------------
	int FileIOHandler::read_file_to_buffer(const std::string &_file_path, std::vector<unsigned char> &_buffer)
	{
		std::ifstream file(_file_path, std::ios::binary);

		// error check
		if (file.fail())
		{
			//EventHandler::push_event(new FileIOErrorEvent(_file_path));
			SYN_CORE_ERROR("File '", _file_path, "' could not be opened.");
			return RETURN_FAILURE;
		}

		// seek to end of file
		file.seekg(0, std::ios::end);

		// get file size and return to the top
		unsigned int fileSize = (unsigned int)file.tellg();
		file.seekg(0, std::ios::beg);

		// reduce file size by headers etc.
		fileSize -= (unsigned int)file.tellg();

		// resize the buffer and read the data
		_buffer.resize(fileSize);
		file.read((char *)&(_buffer[0]), fileSize);
		file.close();

		return RETURN_SUCCESS;

	}


	//-----------------------------------------------------------------------------------
	int FileIOHandler::read_file_to_buffer(const std::string &_file_path, std::string &_buffer)
	{
		std::ifstream file(_file_path, std::ios::binary);

		// error check
		if (file.fail())
		{
			SYN_CORE_ERROR("File '", _file_path, "' could not be opened.");
			return RETURN_FAILURE;
		}

		// seek to end of file
		file.seekg(0, std::ios::end);

		// get file size and return to the top
		unsigned int fileSize = (unsigned int)file.tellg();
		file.seekg(0, std::ios::beg);

		// reduce file size by headers etc.
		fileSize -= (unsigned int)file.tellg();

		// resize the buffer and read the data
		_buffer.resize(fileSize);
		file.read((char *)&(_buffer[0]), fileSize);
		file.close();

		return RETURN_SUCCESS;

	}

}