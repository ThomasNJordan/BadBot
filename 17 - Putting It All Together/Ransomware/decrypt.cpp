#include "decrypt.h"

void HandleError(LPCTSTR psz, DWORD dwErrorNumber) {
    _ftprintf(stderr, TEXT("An error occurred in the program.\n"));
    _ftprintf(stderr, TEXT("%s\n"), psz);
    _ftprintf(stderr, TEXT("Error number: %x\n"), dwErrorNumber);
}

bool DecryptFileWithKey(LPCTSTR pszSourceFile, LPCTSTR pszKey) {
    HANDLE hInpFile = CreateFile(pszSourceFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hInpFile == INVALID_HANDLE_VALUE) {
        HandleError(TEXT("Cannot open input file!\n"), GetLastError());
        return false;
    }

    std::basic_string<_TCHAR> filePath(pszSourceFile);
    size_t pos = filePath.find(_T(".enc"));
    if (pos != std::basic_string<_TCHAR>::npos) {
        filePath = filePath.substr(0, pos);
    }

    HANDLE hOutFile = CreateFile(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOutFile == INVALID_HANDLE_VALUE) {
        HandleError(TEXT("Cannot open output file!\n"), GetLastError());
        CloseHandle(hInpFile);
        return false;
    }

    HCRYPTPROV hProv;
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        HandleError(TEXT("CryptAcquireContext failed!\n"), GetLastError());
        CloseHandle(hInpFile);
        CloseHandle(hOutFile);
        return false;
    }

    HCRYPTHASH hHash;
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        HandleError(TEXT("CryptCreateHash failed!\n"), GetLastError());
        CryptReleaseContext(hProv, 0);
        CloseHandle(hInpFile);
        CloseHandle(hOutFile);
        return false;
    }

    if (!CryptHashData(hHash, (BYTE*)pszKey, _tcslen(pszKey) * sizeof(TCHAR), 0)) {
        HandleError(TEXT("CryptHashData failed!\n"), GetLastError());
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        CloseHandle(hInpFile);
        CloseHandle(hOutFile);
        return false;
    }

    HCRYPTKEY hKey;
    if (!CryptDeriveKey(hProv, CALG_AES_128, hHash, 0, &hKey)) {
        HandleError(TEXT("CryptDeriveKey failed!\n"), GetLastError());
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        CloseHandle(hInpFile);
        CloseHandle(hOutFile);
        return false;
    }

    const size_t chunk_size = OUT_CHUNK_SIZE;
    BYTE* chunk = new BYTE[chunk_size];
    DWORD out_len = 0;

    BOOL isFinal = FALSE;
    DWORD readTotalSize = 0;
    DWORD inputSize = GetFileSize(hInpFile, NULL);

    while (ReadFile(hInpFile, chunk, IN_CHUNK_SIZE, &out_len, NULL)) {
        if (0 == out_len) {
            break;
        }

        readTotalSize += out_len;
        if (readTotalSize >= inputSize) {
            isFinal = TRUE;
        }

        if (!CryptDecrypt(hKey, NULL, isFinal, 0, chunk, &out_len)) {
            HandleError(TEXT("CryptDecrypt failed!\n"), GetLast Error());
            break;
        }

        DWORD written = 0;
        if (!WriteFile(hOutFile, chunk, out_len, &written, NULL)) {
            HandleError(TEXT("Writing failed!\n"), GetLast Error());
            break;
        }
    }

    delete[] chunk;
    CryptDestroyKey(hKey);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    CloseHandle(hInpFile);
    CloseHandle(hOutFile);

    return true;
}
