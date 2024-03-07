#include "enumerate_files.h"

// Function to get all folder names within a directory
std::vector<std::wstring> getFolderNames(const std::wstring& directoryPath) {
    std::vector<std::wstring> folderNames;
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW((directoryPath + L"\\*").c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                wcscmp(findData.cFileName, L".") != 0 && wcscmp(findData.cFileName, L"..") != 0) {
                folderNames.push_back(findData.cFileName);
            }
        } while (FindNextFileW(hFind, &findData) != 0);
        FindClose(hFind);
    }
    return folderNames;
}

// Get all files
std::vector<std::wstring> EnumerateFiles(const std::wstring& directory) {
    std::vector<std::wstring> fileList;
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW((directory + L"\\*").c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                fileList.push_back(directory + L"\\" + findData.cFileName);
            }
        } while (FindNextFileW(hFind, &findData) != 0);
        FindClose(hFind);
    }
    return fileList;
}

// Function to recursively enumerate all files in a directory
void EnumerateFilesRecursive(const std::wstring& directory, std::vector<std::wstring>& fileList) {
    std::vector<std::wstring> subFolders = getFolderNames(directory);
    for (const auto& folder : subFolders) {
        if (folder != L"." && folder != L"..") {
            std::wstring subFolderPath = directory + L"\\" + folder;
            EnumerateFilesRecursive(subFolderPath, fileList);
        }
    }

    std::vector<std::wstring> files = EnumerateFiles(directory); // Assuming EnumerateFiles is correctly implemented
    fileList.insert(fileList.end(), files.begin(), files.end());
}

