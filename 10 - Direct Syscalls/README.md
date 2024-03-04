# 10 - Direct Syscalls
Using syscalls means we can avoid the hooking of NTAPI function calls - something that every AV/EDR worth its salt will check for and monitor.

This is the lowest layer of abstraction we can reach before we start writing entirely in assembly. However, it is very, very similar to the NTAPI code.

The attack chain is the same as process injection:
- Get a target process' handle
- Allocate virtual memory for our payload
- Write our payload into memory
- Start a thread at our payload's memory location

This was written and edited in the Intel Compiler and all offsets were found here: https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html

Code gathered and used from:
- https://redops.at/en/blog/direct-syscalls-vs-indirect-syscalls
- https://www.crow.rip/crows-nest/mal/dev/inject/syscalls/direct-syscalls#the-normal-portion