.DATA 
EXTERN g_NtOpenPROCessSSN:DWORD
EXTERN g_NtAllocateVirtualMemorySSN:DWORD
EXTERN g_NtWriteVirtualMemorySSN:DWORD
EXTERN g_NtCreateThreadExSSN:DWORD
EXTERN g_NtWaitForSingleObjectSSN:DWORD
EXTERN g_NtCloseSSN:DWORD

.CODE
NtOpenPROCess PROC 
		mov r10, rcx
		mov eax, g_NtOpenPROCessSSN       
		syscall                         
		ret                             
NtOpenPROCess ENDP

NtAllocateVirtualMemory PROC    
		mov r10, rcx
		mov eax, g_NtAllocateVirtualMemorySSN      
		syscall                        
		ret                             
NtAllocateVirtualMemory ENDP

NtWriteVirtualMemory PROC 
		mov r10, rcx
		mov eax, g_NtWriteVirtualMemorySSN      
		syscall                        
		ret                             
NtWriteVirtualMemory ENDP 

NtCreateThreadEx PROC 
		mov r10, rcx
		mov eax, g_NtCreateThreadExSSN      
		syscall                        
		ret                             
NtCreateThreadEx ENDP 

NtWaitForSingleObject PROC 
		mov r10, rcx
		mov eax, g_NtWaitForSingleObjectSSN      
		syscall                        
		ret                             
NtWaitForSingleObject ENDP 

NtClose PROC 
		mov r10, rcx
		mov eax, g_NtCloseSSN      
		syscall                        
		ret                             
NtClose ENDP 
END