#ifndef FOLDER_UTILS_H
#define FOLDER_UTILS_H

#include <vector>
#include <string>
#include <windows.h> // Include for Windows-specific types used in the function declarations
#include <iostream>

// Function to get all folder names within a directory.
// This function scans the specified directory and returns a list of subdirectory names, excluding "." and "..".
std::vector<std::wstring> getFolderNames(const std::wstring& directoryPath);

// Function to find all files in a specified directory (non-recursive).
// This function returns a list of file paths for all files located directly within the specified directory.
std::vector<std::wstring> EnumerateFiles(const std::wstring& directory);

// Function to recursively enumerate all files in a directory and its subdirectories.
// This function populates the fileList vector with the paths of all files found within the specified directory and its subdirectories.
void EnumerateFilesRecursive(const std::wstring& directory, std::vector<std::wstring>& fileList);

#endif // FOLDER_UTILS_H
