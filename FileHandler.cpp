#include "md5.h"
#include "FileHandler.h"
#include "Net.h"




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

	return CalculateMD5Internal(data.data(), data.size());
}

void FileHandler::SendFileMetadata(const std::string& fileName, size_t fileSize, ReliableConnection& connection)
{
}

void FileHandler::SendFileContent(const std::vector<char>& fileContent, ReliableConnection& connection)
{
}

void FileHandler::ReceiveFileMetadata(std::string& fileName, size_t& fileSize, ReliableConnection& connection)
{
}

void FileHandler::ReceiveFileContentAndVerify(const std::string& fileName, size_t fileSize, ReliableConnection& connection)
{
}

uint32_t FileHandler::CalculateMD5Internal(const char* data, size_t size)
{
	MD5 md5; //creating instatnce of md5 method 
	md5.update(reinterpret_cast<const unsigned char*>(data), size);
	md5.finalize();

	//converted first 4 bytes of the hashi into uint32
	const unsigned char* digest = reinterpret_cast<const unsigned char*>(md5.hexdigest().c_str());
	uint32_t result = static_cast<uint32_t>(digest[0]) | (static_cast<uint32_t>(digest[1]) << 8) | (static_cast<uint32_t>(digest[2]) << 16) | (static_cast<uint32_t>(digest[3]) << 24);
	return result;  // this function will return the hash value
}
