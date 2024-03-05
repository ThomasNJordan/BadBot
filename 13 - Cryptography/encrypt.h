#ifndef ENCRYPT_H
#define ENCRYPT_H

#include "enumerate_files.h"

// Define AES block size
#define AES_BLOCK_SIZE 16

void encryptFile(const char* inputFile, const BYTE* key, const BYTE* iv);
void handleErrors(DWORD errorCode);

// Encrypts a folder using AES encryption
// bool encryptFolder(const std::wstring& folderPath, const BYTE* decryptionKey, DWORD keySize);

#endif
