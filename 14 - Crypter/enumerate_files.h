#ifndef FOLDER_UTILS_H
#define FOLDER_UTILS_H

#include <string>
#include <vector>
#include <iostream>
#include <windows.h>

// Function to get all folder names within a directory
std::vector<std::wstring> getFolderNames(const std::wstring& directoryPath);

// Find all files in folder
std::vector<std::wstring> EnumerateFiles(const std::wstring& directory);

// Function to recursively enumerate all files in a directory
void EnumerateFilesRecursive(const std::wstring& directory, std::vector<std::wstring>& fileList);

#endif // FOLDER_UTILS_H
