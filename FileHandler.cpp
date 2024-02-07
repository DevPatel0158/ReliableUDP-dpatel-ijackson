#include "FileHandler.h"
#include <openssl/md5.h>


FileHandler::FileHandler()
{
}

bool FileHandler::GetFileInfo(const std::string& filePath, std::string& fileName, size_t& fileSize)
{
	return false;
}

bool FileHandler::ReadFileContent(const std::string& filePath, std::vector<char>& fileContent)
{
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
