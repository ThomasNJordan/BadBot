#ifndef ENCRYPT_H
#define ENCRYPT_H

#include "enumerate_files.h"

// Encrypts a folder using AES encryption
bool encryptFolder(const std::wstring& folderPath, const BYTE* decryptionKey, DWORD keySize);

#endif