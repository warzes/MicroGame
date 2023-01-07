#pragma once

namespace FileSystem
{
	std::optional<std::vector<uint8_t>> FileToMemory(const char* fileName, unsigned int* bytesRead = nullptr);

	bool FileExists(const char* fileName);     // Check if file exists

	// Get pointer to extension for a filename string (includes the dot: .png)
	const char* GetFileExtension(const char* fileName);
	// Get pointer to filename for a path string
	const char* GetFileName(const char* filePath);
}