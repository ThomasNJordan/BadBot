#include "encrypt.h"

// Encrypts a folder using AES encryption
bool encryptFolder(const std::wstring& folderPath, const BYTE* decryptionKey, DWORD keySize) {
    // Open the directory
    HANDLE hFind;
    WIN32_FIND_DATAW findData;
    std::wstring searchPath = folderPath + L"\\*";
    hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening directory." << std::endl;
        return false;
    }

    // Initialize AES encryption
    HCRYPTPROV hProv;
    if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        std::cerr << "Error acquiring cryptographic context." << std::endl;
        FindClose(hFind);
        return false;
    }

    // Create an AES key from the decryption key
    HCRYPTKEY hKey;
    if (!CryptImportKey(hProv, decryptionKey, keySize, 0, 0, &hKey)) {
        std::cerr << "Error importing AES key." << std::endl;
        CryptReleaseContext(hProv, 0);
        FindClose(hFind);
        return false;
    }

    // Encrypt each file in the directory
    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::wstring filePath = folderPath + L"\\" + findData.cFileName;
            HCRYPTKEY hSessionKey;
            if (!CryptGenKey(hProv, CALG_AES_256, CRYPT_EXPORTABLE, &hSessionKey)) {
                std::cerr << "Error generating session key." << std::endl;
                CryptDestroyKey(hKey);
                CryptReleaseContext(hProv, 0);
                FindClose(hFind);
                return false;
            }

            // Encrypt the file
            if (!CryptEncrypt(hKey, hSessionKey, TRUE, 0, nullptr, nullptr, nullptr)) {
                std::cerr << "Error encrypting file." << std::endl;
                CryptDestroyKey(hSessionKey);
                CryptDestroyKey(hKey);
                CryptReleaseContext(hProv, 0);
                FindClose(hFind);
                return false;
            }

            CryptDestroyKey(hSessionKey);
        }
    } while (FindNextFileW(hFind, &findData) != 0);

    // Clean up resources
    CryptDestroyKey(hKey);
    CryptReleaseContext(hProv, 0);
    FindClose(hFind);

    return true;
}

