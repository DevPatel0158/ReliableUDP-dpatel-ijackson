#pragma once
#include <string>
#include <fstream>
#include <vector>

class FileHandler {
public:
	FileHandler();

	//defined a method to read the file information
	bool GetFileInfo(const std::string& filePath, std::string& fileName, size_t& fileSize);

	//defined a method to read the file content
	bool ReadFileContent(const std::string& filePath, std::vector<char>& fileContent);

	//defined a mathod to calculate crc32 checksome to check file integrity
	uint32_t CalculateCRC32(const std::vector<char>& data);

private:
	//internal mehtod to support checksum method
	uint32_t CalculateCRC32Internal(const char* data, size_t size);
};