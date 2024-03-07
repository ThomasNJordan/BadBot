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

    // Key used for encryption
    LPCTSTR key = TEXT("3igcZhRdWq96m3GUmTAiv9");

    // Wait for user input before exiting
    getchar();
    
    return EXIT_SUCCESS;
}
