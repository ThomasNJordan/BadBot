.CODE

getPEB PROC
    mov rax, gs:[60h] ; address of PEB
    ret
getPEB ENDP

CheckDebugger PROC
    xor eax, eax ; clear eax register
    call getPEB
    movzx eax, byte ptr [rax+2h] ; load byte from rax+2, if true: PEB -> isDebugged
    ret
CheckDebugger ENDP

END