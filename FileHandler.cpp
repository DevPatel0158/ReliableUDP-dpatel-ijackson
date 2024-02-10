
#include "FileHandler.h"
#include "md5.h"




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
	std::vector<char> metadataPacket(MetadataPacketSize, 0);  // created a metadata packet
	snprintf(metadataPacket.data(), MetadataPacketSize, "META|%s|%zu", fileName.c_str(), fileSize);
	connection.SendPacket(metadataPacket.data(), metadataPacket.size()); // sending the metadata packets
}

void FileHandler::SendFileContent(const std::vector<char>& fileContent, ReliableConnection& connection)
{
	uint32_t md5Hash = CalculateMD5(fileContent);   // calculating md5 hash
	connection.SendPacket(fileContent.data(), fileContent.size()); //Sending the file content

	std::vector<char> ackPacket(AckPacketSize, 0);
	snprintf(ackPacket.data(), ackPacket.size(), "ACK|%u", md5Hash);
	connection.SendPacket(fileContent.data(), fileContent.size());   // sending acknowledgement
}

void FileHandler::ReceiveFileMetadata(std::string& fileName, size_t& fileSize, ReliableConnection& connection)
{
	std::vector<char> metadataPacket(MetadataPacketSize, 0);
	connection.SendPacket(metadataPacket.data(), metadataPacket.size());

	sscanf(metadataPacket.data(), "META|%[^|]|%zu", fileName.data(), &fileSize);  // parsing the metadata
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
