#include "windows.h"
#include "winternl.h"
#include "stdio.h"

/**
 * Call external assembly:
 * PPEB -> pointer to PEB
 * CheckDebugger returns ifDebugger byte
*/
extern PPEB getPEB(void);
extern BYTE CheckDebugger(void);

int main(void) {
    /* Get PEB from asm */
    printf("Fetching the PEB\n");
    PPEB pPEB = getPEB();
    printf("\t-PEB:0x%p\n", pPEB);

    /* Exit if being debugged */
    if (CheckDebugger() != 0) {
        printf("Debugger detected, exiting...\n");
        return EXIT_FAILURE;
    }

    /* "Payload" */
    printf("No debugger found, continuing\n");
    MessageBoxW(NULL, L"Payload", L"Payload", MB_ICONEXCLAMATION);
    return EXIT_SUCCESS;
}
