# 0x03 - DLL Injection
Today I spent my time learning how to create a DLL, how to call one, and how to call a DLL from another process. In this example I used kernel32's LoadLibrary() function to load the DLL. 

List of files:
- harmless.cpp - An example of how to load a DLL
- load_malicious_dll.cpp - my script to load a malicious DLL from Kernel32 and inject it into a running process
- malicious_dll.cpp - a sample DLL that spawns a window as POC