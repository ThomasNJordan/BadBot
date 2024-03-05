#include "encrypt.h"

void handleErrors(DWORD errorCode) {
    fprintf(stderr, "Error: %lu\n", errorCode);
    exit(1);
}

// Encrypt a file using AES encryption
void encryptFile(const char* inputFile, const BYTE* key, const BYTE* iv) {
    /* Variable declaration */
    HCRYPTPROV hCryptProv = 0;
    HCRYPTKEY hKey = 0;
    HCRYPTHASH hHash = 0;
    HANDLE hInFile = INVALID_HANDLE_VALUE;
    HANDLE hOutFile = INVALID_HANDLE_VALUE;
    DWORD bytesRead = 0;
    DWORD bytesWritten = 0;
    BYTE buffer[4096];
    BOOL result;

    // Open input file
    hInFile = CreateFile(inputFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hInFile == INVALID_HANDLE_VALUE)
        handleErrors(GetLastError());

    // Generate output file name by adding ".enc" extension
    char encryptedFileName[MAX_PATH];
    snprintf(encryptedFileName, MAX_PATH, "%s.enc", inputFile);

    // Open output file
    hOutFile = CreateFile(encryptedFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOutFile == INVALID_HANDLE_VALUE)
        handleErrors(GetLastError());

    // Acquire a cryptographic provider context
    result = CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
    if (!result)
        handleErrors(GetLastError());

    // Create an AES hash
    result = CryptCreateHash(hCryptProv, CALG_AES_128, 0, 0, &hHash);
    if (!result)
        handleErrors(GetLastError());

    result = CryptHashData(hHash, key, AES_BLOCK_SIZE, 0);
    if (!result)
        handleErrors(GetLastError());

    result = CryptDeriveKey(hCryptProv, CALG_AES_128, hHash, 0, &hKey);
    if (!result)
        handleErrors(GetLastError());

    // Encrypt data from input file and write to output file
    while (ReadFile(hInFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        result = CryptEncrypt(hKey, (HCRYPTHASH)NULL, FALSE, 0, buffer, &bytesRead, sizeof(buffer));
        if (!result)
            handleErrors(GetLastError());
        WriteFile(hOutFile, buffer, bytesRead, &bytesWritten, NULL);
    }

    // Cleanup
    if (hKey)
        CryptDestroyKey(hKey);
    if (hHash)
        CryptDestroyHash(hHash);
    if (hCryptProv)
        CryptReleaseContext(hCryptProv, 0);
    if (hInFile != INVALID_HANDLE_VALUE)
        CloseHandle(hInFile);
    if (hOutFile != INVALID_HANDLE_VALUE)
        CloseHandle(hOutFile);
}


// Encrypts a folder using encryptFile();
/*
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
    HCRYPTKEY hKey = 0;
    if (!CryptImportKey(hProv, decryptionKey, keySize, 0, 0, &hKey)) {
        DWORD dwError = GetLastError();
        std::cerr << "Error importing AES key. Error code: " << dwError << std::endl;
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
            DWORD dwDataLen = 32; // 256 bit key
            if (!CryptEncrypt(hKey, hSessionKey, TRUE, 0, nullptr, nullptr, dwDataLen)) {
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
*/