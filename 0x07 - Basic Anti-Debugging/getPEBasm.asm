.code

getPEB proc
    mov rax, gs:[60h] ; address of PEB
    ret
getPEB endp

CheckDebugger proc
    xor eax, eax ; clear eax register
    call getPEB
    movzx eax, byte ptr [rax+2h] ; load byte from rax+2, if true: PEB -> isDebugged
    ret
CheckDebugger endp
end