#include "FileHandler.h"
#include <openssl/md5.h>


FileHandler::FileHandler()
{
}

bool FileHandler::GetFileInfo(const std::string& filePath, std::string& fileName, size_t& fileSize)
{
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		fileSize = static_cast<size_t>(file.tellg());
		fileName = filePath.substr(filePath.find_last_of('/') + 1);  // extracting file name from the path it is coming 
		return true;
	}
	return false;
}

bool FileHandler::ReadFileContent(const std::string& filePath, std::vector<char>& fileContent)
{
	std::ifstream file(filePath, std::ios::binary);
	if (file.is_open())
	{
		fileContent.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		return true;
	}
	return false;
}

uint32_t FileHandler::CalculateMD5(const std::vector<char>& data)
{
	return 0;
}

uint32_t FileHandler::CalculateMD5Internal(const char* data, size_t size)
{
	return 0;
}
