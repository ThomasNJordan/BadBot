#include <windows.h>

BOOL __stdcall DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved) {
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            MessageBoxW(NULL, L"My Window", L"Custom Window", MB_OK);
            break;
    }

    return TRUE;
}