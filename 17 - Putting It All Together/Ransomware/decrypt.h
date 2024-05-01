// decrypt.h
#ifndef DECRYPT_H
#define DECRYPT_H

#include "encrypt.h"
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>

#include <windows.h>
#include <wincrypt.h>

void HandleError(LPCTSTR psz, DWORD dwErrorNumber);

bool DecryptFileWithKey(LPCTSTR pszSourceFile, LPCTSTR pszKey);

#endif // DECRYPT_H
