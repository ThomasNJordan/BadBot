#ifndef UNICODE
#define UNICODE
#endif

#include "windows.h"
#include "iostream"

int main() {
    MessageBox(NULL, L"My Window", L"Custom Window", MB_OK);
    return EXIT_SUCCESS;
}