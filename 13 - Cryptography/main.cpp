#include "encrypt.h"

int main() {
    // Get the path to the user's profile folder
    TCHAR userProfileDir[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, userProfileDir))) {
        // Print the user's profile folder path
        printf("User's profile folder: %s\n", userProfileDir);
    } else {
        // Failed to get the user's profile folder path
        fprintf(stderr, "Failed to get user's profile folder.\n");
        return EXIT_FAILURE;
    }

    // Construct the full path to the input file
    TCHAR inputFile[MAX_PATH];
    _sntprintf_s(inputFile, MAX_PATH, _T("%s\\Desktop\\test.txt"), userProfileDir);

    // Print the input file path
    printf("Input file path: %s\n", inputFile);

    // Key used for encryption
    LPCTSTR key = TEXT("3igcZhRdWq96m3GUmTAiv9");

    // Perform file encryption
    if (EncryptFileWithKey(inputFile, key)) {
        printf("Encryption successful.\n");
    } else {
        printf("Encryption failed.\n");
    }   

    // Wait for user input before exiting
    getchar();
    
    return EXIT_SUCCESS;
}
