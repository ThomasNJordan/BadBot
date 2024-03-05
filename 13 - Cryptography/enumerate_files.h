#ifndef FOLDER_UTILS_H
#define FOLDER_UTILS_H

#include <string>
#include <vector>
#include <iostream>
#include <windows.h>
#include <wincrypt.h>

// Function to get all folder names within a directory
std::vector<std::wstring> getFolderNames(const std::wstring& directoryPath);

#endif // FOLDER_UTILS_H
