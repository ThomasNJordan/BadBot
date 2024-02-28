#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <stdio.h>

// Create a process that opens calc.exe
// C:\Windows\System32\calc.exe
int main(void) {
    /*
    * BOOL CreateProcessW(
    *   [in, optional]      LPCWSTR               lpApplicationName,
    *   [in, out, optional] LPWSTR                lpCommandLine,
    *   [in, optional]      LPSECURITY_ATTRIBUTES lpProcessAttributes,
    *   [in, optional]      LPSECURITY_ATTRIBUTES lpThreadAttributes,
    *   [in]                BOOL                  bInheritHandles,
    *   [in]                DWORD                 dwCreationFlags,
    *   [in, optional]      LPVOID                lpEnvironment,
    *   [in, optional]      LPCWSTR               lpCurrentDirectory,
    *   [in]                LPSTARTUPINFOW        lpStartupInfo,
    *   [out]               LPPROCESS_INFORMATION lpProcessInformation
    * );
    */

    /* Create pointers for startup info and process info (Windows created them already)*/
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    printf("Creating process...\n");
    if (!CreateProcessW(
        L"C:\\Windows\\System32\\notepad.exe",
        NULL,
        NULL,
        NULL,
        FALSE,
        BELOW_NORMAL_PRIORITY_CLASS,
        NULL,
        NULL,
        &si,
        &pi)) {
            printf("(-) Failed to create process, error code: %ld", GetLastError());
            return EXIT_FAILURE;
        }
    printf("(+) Process Created Successfully\n");
    printf("\t(+) PID:  %ld\tHandle:  %ld\n", pi.dwProcessId, pi.hProcess);
    printf("\t(+) TID:  %ld\tHandle:  %ld\n", pi.dwThreadId, pi.hThread);

    /* Wait for handle to finish, then close */
    /*
    * DWORD WaitForSingleObject(
    *   [in] HANDLE hHandle,
    *   [in] DWORD  dwMilliseconds
    * );
    */
    if (!WaitForSingleObject( // assume thread finished when process is finished
        pi.hProcess,
        INFINITE
    )) {
        printf("(+) Process Finished Executing\n");
        // Close Process
        if (!CloseHandle(pi.hProcess)) {
            printf("(-) Error closing process\n");
            return EXIT_FAILURE;
        } /* fi */
    } /* fi */
    return EXIT_SUCCESS;
}
