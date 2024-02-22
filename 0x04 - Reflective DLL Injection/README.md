# 0x04 - Reflective DLL Injection
These days it's incredibly rare for malware to write anything to disk. Modern AV's have gotten really good at determining if files are malicious or not, so any malware worth its salt operates almost entirely from memory. Saying this, reflective DLL injection is a technique where a DLL is loaded from memory (not disk).

As described in https://www.ired.team/offensive-security/code-injection-process-injection/reflective-dll-injection:
- Execution is passed, either via CreateRemoteThread() or a tiny bootstrap shellcode, to the library's ReflectiveLoader function which is an exported function found in the library's export table.
- As the library's image will currently exists in an arbitrary location in memory the ReflectiveLoader will first calculate its own image's current location in memory so as to be able to parse its own headers for use later on.
- The ReflectiveLoader will then parse the host processes kernel32.dll export table in order to calculate the addresses of three functions required by the loader, namely LoadLibraryA, GetProcAddress and VirtualAlloc.
- The ReflectiveLoader will now allocate a continuous region of memory into which it will proceed to load its own image. The location is not important as the loader will correctly relocate the image later on.
- The library's headers and sections are loaded into their new locations in memory.
- The ReflectiveLoader will then process the newly loaded copy of its image's import table, loading any additional library's and resolving their respective imported function addresses.
- The ReflectiveLoader will then process the newly loaded copy of its image's relocation table.
- The ReflectiveLoader will then call its newly loaded image's entry point function, DllMain with DLL_PROCESS_ATTACH. The library has now been successfully loaded into memory.
- Finally the ReflectiveLoader will return execution to the initial bootstrap shellcode which called it, or if it was called via CreateRemoteThread, the thread will terminate.

I ended up using Red Team notes' code since it is much easier to understand and learn from than re-writing it myself.