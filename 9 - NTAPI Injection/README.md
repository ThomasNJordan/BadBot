# 9 - NTAPI Injection
This code was recreated from these two blog posts on NTAPI Injection. This was an exercise in familiarizing myself with the technique so very little of the code is my own.
- https://www.crow.rip/crows-nest/mal/dev/inject/ntapi-injection
- https://blog.omroot.io/process-code-injection-through-undocumented-ntapis/

NTAPI Injection is a layer of abstraction below using the Win32 API but above directly calling syscalls. It has the advantage of abstracting away from some of the undocumented system calls which are subject to frequent changes by Microsoft, however with that abstraction - it makes it easier for AV's to detect the injection. There is also a technique of API hooking used by AV's to detect this kind of attack by hooking themselves into function calls which also makes it easier to detect.
