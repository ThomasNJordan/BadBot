# 0x06 - Keylogger
For this project I'm building off of what I've covered already by making a simple keylogger.   
This code:
- Contains a DLL that is a keylogger
- The DLL is injected into `winlogon.exe`, a priveleged process (using kernel32 function calls)
- The kelogger logs all files locally to a file

There's also this method where you can create a process that can get keyboard input from secure desktop: https://0xpat.github.io/Malware_development_part_7/
I experimented with both methods and I've included the code to inject into a DLL