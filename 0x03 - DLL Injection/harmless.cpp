#ifndef UNICODE
#define UNICODE
#endif

#include "windows.h"
#include "stdio.h"

// Define macros to create status symbols
#define DEFINE_STATUS_SYMBOL(name, value) \
    const char* name = value;

// Use the macros to define status symbols
DEFINE_STATUS_SYMBOL(SUCC, "[+]")
DEFINE_STATUS_SYMBOL(ERR, "[-]")
DEFINE_STATUS_SYMBOL(INFO, "[*]")

int main() {
    HMODULE hMalicousDLL = NULL; // Default initalization
    wchar_t dllPath[MAX_PATH] = L"C:\\Users\\thoma\\Documents\\MalwareDev\\0x03 - DLL Injection\\malicious_dll.dll";

    printf("%s Attempting to get Malicious DLL's handle.\n", INFO);
    hMalicousDLL = LoadLibrary(dllPath);

    if (hMalicousDLL == NULL) {
        printf("%s Error getting a handle to Malicious DLL\n", ERR);
        return EXIT_FAILURE;
    }
    printf("%s Got a handle to Malicious DLL.\n", SUCC);
    printf("\t-Malicious DLL's Handle: 0x%p\n", hMalicousDLL);

    // Cleanup
    FreeLibrary(hMalicousDLL);
    printf("%sExiting...\n", SUCC);
    
    return EXIT_SUCCESS;
}