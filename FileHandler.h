#pragma once

#include <string>
#include <fstream>
#include <vector>
#include "Net.h"
#include "ReliableUDP.cpp"




class FileHandler {
public:
	FileHandler();

	//defined a method to read the file information
	bool GetFileInfo(const std::string& filePath, std::string& fileName, size_t& fileSize);

	//defined a method to read the file content
	bool ReadFileContent(const std::string& filePath, std::vector<char>& fileContent);

	//defined a mathod to calculate MD5 checksome to check file integrity
	static uint32_t CalculateMD5(const std::vector<char>& data);

	//defined a method to send file metadata
	static void SendFileMetadata(const std::string& fileName, size_t fileSize, ReliableConnection& connection);

	//defined a function to send filecontent
	static void SendFileContent(const std::vector<char>& fileContent, ReliableConnection& connection);

	//defined a method to receive metadata
	static void ReceiveFileMetadata(std::string& fileName, size_t& fileSize, ReliableConnection& connection);

	//defined a function that receives file content and validates verify the hash received from md5
	static void ReceiveFileContentAndVerify(const std::string& fileName, size_t fileSize, ReliableConnection& connection);


private:
	//internal mehtod to support checksum method
	static uint32_t CalculateMD5Internal(const char* data, size_t size);
};