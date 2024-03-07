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
