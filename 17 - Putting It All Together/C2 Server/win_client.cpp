// Standard C++ library headers
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <sstream> 
#include <iomanip> // for std::setfill and std::setw

// Windows specific headers
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <psapi.h>
#include <intrin.h> // For CPU information using __cpuid
#include <bcrypt.h>

// OpenSSL headers for cryptographic functionality
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/err.h>

// Pragma comments to link necessary libraries
#pragma comment(lib, "ws2_32.lib")   // Winsock library for network communication
#pragma comment(lib, "iphlpapi.lib") // IP Helper API for network interface information
#pragma comment(lib, "psapi.lib")    // Process Status API for system information
#pragma comment(lib, "bcrypt.lib")   // Microsoft's Cryptography API: Next Generation
#pragma comment(lib, "libssl.lib")   // OpenSSL's SSL library
#pragma comment(lib, "libcrypto.lib")// OpenSSL's cryptographic library

#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)
#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)

/* Initalize socket using Win32 API */
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
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

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

std::string getSystemInfo() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    std::ostringstream info;
    info << "Number of Processors: " << si.dwNumberOfProcessors << "\n"
         << "Processor Type: " << si.dwProcessorType << "\n"
         << "Processor Architecture: ";

    switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: info << "AMD64 (x64)"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: info << "Intel x86"; break;
        case PROCESSOR_ARCHITECTURE_UNKNOWN: info << "Unknown"; break;
        default: info << "Other"; break;
    }
    info << "\n";

    info << getOSInfo();
    info << getProcessorInfo();
    info << getMemoryInfo();

    return info.str();
}


/* Helper function, replace sendall() in python */
void sendData(SOCKET sock, const char* buffer, int len) {
    send(sock, buffer, len, 0);
}

// Function to receive a public key in PEM format
std::vector<char> receivePublicKey(SOCKET sock) {
    std::vector<char> buffer(4096);
    int bytesRead = recv(sock, buffer.data(), buffer.size(), 0);
    if (bytesRead == SOCKET_ERROR) {
        std::cerr << "Failed to receive public key" << std::endl;
        buffer.clear();
        return buffer;
    }
    buffer.resize(bytesRead);
    return buffer;
}

// Function to convert PEM to DER using OpenSSL
std::vector<unsigned char> convertPemToDer(const std::vector<char>& pemKey) {
    BIO* bio = BIO_new_mem_buf(pemKey.data(), static_cast<int>(pemKey.size()));
    EVP_PKEY* pubKey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);

    int len = 0;
    unsigned char* p = NULL;
    len = i2d_PublicKey(pubKey, &p);
    std::vector<unsigned char> derKey(p, p + len); // Correctly forming the vector from raw pointer.

    // Freeing resources
    OPENSSL_free(p);
    EVP_PKEY_free(pubKey);
    BIO_free(bio);
    return derKey;
}

/* Encrypt system data */
std::vector<unsigned char> encryptData(const std::vector<unsigned char>& plaintext, EVP_PKEY* pubKey) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pubKey, NULL);
    if (!ctx) {
        std::cerr << "Failed to create EVP_PKEY_CTX" << std::endl;
        return {};
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        std::cerr << "Failed to initialize encryption context or set padding" << std::endl;
        return {};
    }

    size_t ciphertext_len = 0;
    // Determine buffer length
    if (EVP_PKEY_encrypt(ctx, NULL, &ciphertext_len, plaintext.data(), plaintext.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        std::cerr << "Failed to determine ciphertext length" << std::endl;
        return {};
    }

    std::vector<unsigned char> ciphertext(ciphertext_len);
    if (EVP_PKEY_encrypt(ctx, ciphertext.data(), &ciphertext_len, plaintext.data(), plaintext.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        std::cerr << "Encryption failed" << std::endl;
        return {};
    }

    ciphertext.resize(ciphertext_len);
    EVP_PKEY_CTX_free(ctx);
    return ciphertext;
}

/* Function to send encrypted data to the server */
void sendEncryptedData(const SOCKET& sock, EVP_PKEY* pubKey, const std::string& systemInfo) {
    std::vector<unsigned char> infoBytes(systemInfo.begin(), systemInfo.end());

    const size_t maxChunkSize = 190; // Adjust based on the public key size and padding
    size_t offset = 0;
    std::cout << "Attempting to encrypt data in chunks" << std::endl;

    while (offset < infoBytes.size()) {
        size_t chunkSize = std::min(maxChunkSize, infoBytes.size() - offset);
        std::vector<unsigned char> chunk(infoBytes.begin() + offset, infoBytes.begin() + offset + chunkSize);
        std::vector<unsigned char> encryptedChunk = encryptData(chunk, pubKey);

        if (encryptedChunk.empty()) {
            std::cerr << "Encryption failed for chunk. Aborting send." << std::endl;
            return;
        }

        // Print the encrypted chunk
        std::cout << "Encrypted Chunk (hex): ";
        for (auto byte : encryptedChunk) {
            std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)byte;
        }
        std::cout << std::dec << "\n";

        if (send(sock, reinterpret_cast<char*>(encryptedChunk.data()), encryptedChunk.size(), 0) == SOCKET_ERROR) {
            std::cerr << "Failed to send data: " << WSAGetLastError() << std::endl;
            return;
        }

        offset += chunkSize;
    }

    std::cout << "Successfully encrypted and sent data in chunks." << std::endl;
    const char eofSignal[] = "EOF";
    if (send(sock, eofSignal, sizeof(eofSignal), 0) == SOCKET_ERROR) {
        std::cerr << "Failed to send EOF signal: " << WSAGetLastError() << std::endl;
    }
}

int main() {
    /* Connect to server */
    initWinsock();
    SOCKET sock = connectSocket("localhost", 8888);

    /* Receive public key */
    std::vector<char> pemKeyVec = receivePublicKey(sock);
    BIO* bio = BIO_new_mem_buf(pemKeyVec.data(), pemKeyVec.size());
    EVP_PKEY* pubKey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);
    if (!pubKey) {
        std::cerr << "Failed to parse public key" << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    /* Send system info */
    std::string systemInfo = getSystemInfo();  
    std::cout << systemInfo;
    auto encryptedData = encryptData(std::vector<unsigned char>(systemInfo.begin(), systemInfo.end()), pubKey);
    sendData(sock, reinterpret_cast<const char*>(encryptedData.data()), encryptedData.size());

    EVP_PKEY_free(pubKey);
    closesocket(sock);
    WSACleanup();
    return 0;
}
