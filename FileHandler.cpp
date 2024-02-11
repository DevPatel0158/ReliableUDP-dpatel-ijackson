/*
* Issac Jackson & Dev Patel
* Student 8866936 & 8765267
* February 10 2024
* Assignment 01: This program ensures reliable transmission of user-selected files and implements a file integrity verification method, and calculates transmission time and speed.
*/
#include "Net.h"
#include "FileHandler.h"
#include "md5.h"
#include "ReliableUDP.cpp"

constexpr int AckPacketSize = 128;
constexpr int MetadataPacketSize =64 ;


/**
 * @brief Default constructor for FileHandler.
 */
FileHandler::FileHandler()
{
}


/**
 * @brief Retrieves file information such as name and size.
 *
 * @param filePath Path to the file.
 * @param fileName Reference to store the extracted file name.
 * @param fileSize Reference to store the extracted file size.
 * @return True if the file information is successfully retrieved, false otherwise.
 */
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


/**
 * @brief Reads the content of a file into a vector of characters.
 *
 * @param filePath Path to the file.
 * @param fileContent Reference to a vector to store the file content.
 * @return True if the file content is successfully read, false otherwise.
 */
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


/**
 * @brief Calculates the MD5 hash of the given data.
 *
 * @param data Vector of characters representing the data.
 * @return MD5 hash as a 32-bit unsigned integer.
 */
uint32_t FileHandler::CalculateMD5(const std::vector<char>& data)
{

	return CalculateMD5Internal(data.data(), data.size());
}


/**
 * @brief Sends file metadata over a reliable connection.
 *
 * @param fileName Name of the file.
 * @param fileSize Size of the file.
 * @param connection ReliableConnection object for sending metadata.
 */
void FileHandler::SendFileMetadata(const std::string& fileName, size_t fileSize, ReliableConnection& connection)
{
	std::vector<char> metadataPacket(MetadataPacketSize, 0);  // created a metadata packet
	snprintf(metadataPacket.data(), MetadataPacketSize, "META|%s|%lu", fileName.c_str(), static_cast<unsigned long>(fileSize));

	connection.SendPacket(reinterpret_cast<const unsigned char*>(metadataPacket.data()), static_cast<int>(metadataPacket.size()));
	// sending the metadata packets
}


/**
 * @brief Sends file content and its MD5 hash over a reliable connection.
 *
 * @param fileContent Vector of characters representing the file content.
 * @param connection ReliableConnection object for sending content and acknowledgment.
 */
void FileHandler::SendFileContent(const std::vector<char>& fileContent, ReliableConnection& connection)
{
	uint32_t md5Hash = CalculateMD5(fileContent);   // calculating md5 hash
	connection.SendPacket(reinterpret_cast<const unsigned char*>(fileContent.data()), static_cast<int>(fileContent.size())); //Sending the file content

	std::vector<char> ackPacket(AckPacketSize, 0);
	snprintf(ackPacket.data(), ackPacket.size(), "ACK|%u", md5Hash);
	connection.SendPacket(reinterpret_cast<const unsigned char*>(fileContent.data()), static_cast<int>(fileContent.size()));


}


/**
 * @brief Receives file metadata over a reliable connection.
 *
 * @param fileName Reference to store the received file name.
 * @param fileSize Reference to store the received file size.
 * @param connection ReliableConnection object for receiving metadata.
 */
void FileHandler::ReceiveFileMetadata(std::string& fileName, size_t& fileSize, ReliableConnection& connection)
{
	std::vector<char> metadataPacket(MetadataPacketSize, 0);
	connection.SendPacket(reinterpret_cast<const unsigned char*>(metadataPacket.data()), static_cast<int>(metadataPacket.size()));

	char fileNameBuffer[256]; 
	sscanf_s(metadataPacket.data(), "META|%[^|]|%llu", fileNameBuffer, static_cast<unsigned>(_TRUNCATE), &fileSize);
	fileName = fileNameBuffer;



	// parsing the metadata
}


/**
 * @brief Receives file content, calculates MD5 hash, and verifies integrity over a reliable connection.
 *
 * @param fileName Name of the file for verification.
 * @param fileSize Size of the file for verification.
 * @param connection ReliableConnection object for receiving content and acknowledgment.
 */
void FileHandler::ReceiveFileContentAndVerify(const std::string& fileName, size_t fileSize, ReliableConnection& connection)
{
	std::vector<char> fileContent;

	std::vector<char> ackPacket(AckPacketSize, 0);
	connection.ReceivePacket(reinterpret_cast<unsigned char*>(ackPacket.data()), static_cast<int>(ackPacket.size())); // received file content

	uint32_t receivedMd5Hash = CalculateMD5(fileContent); /// calculated the md5 hash for the received file



	connection.ReceivePacket(reinterpret_cast<unsigned char*>(ackPacket.data()), static_cast<int>(ackPacket.size()));
	//received acked packet 

	uint32_t sentMd5Hash;
	sscanf_s(ackPacket.data(), "ACK|%u", &sentMd5Hash);
	// parsed received md5 hash 

	if (receivedMd5Hash == sentMd5Hash)     // checking and verifying integrity with comparing md5 hashes
	{
		printf("File transfer successful. Integrity verified.\n");
	}
	else
	{
		
		printf("File transfer failed. Integrity verification failed.\n");
	}
}


/**
 * @brief Calculates the MD5 hash of the given data.
 *
 * @param data Pointer to the data.
 * @param size Size of the data.
 * @return MD5 hash as a 32-bit unsigned integer.
 */
uint32_t FileHandler::CalculateMD5Internal(const char* data, size_t size)
{
	MD5 md5; //creating instatnce of md5 method 
	md5.update(reinterpret_cast<const unsigned char*>(data), static_cast<MD5::size_type>(size));
	md5.finalize();

	//converted first 4 bytes of the hashi into uint32
	const unsigned char* digest = reinterpret_cast<const unsigned char*>(md5.hexdigest().c_str());
	uint32_t result = static_cast<uint32_t>(digest[0]) | (static_cast<uint32_t>(digest[1]) << 8) | (static_cast<uint32_t>(digest[2]) << 16) | (static_cast<uint32_t>(digest[3]) << 24);
	return result;  // this function will return the hash value
}
