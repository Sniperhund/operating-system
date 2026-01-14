section .text

global usermode
usermode:
    mov esi, [esp + 4]
    mov eax, [esi + 0] ; ctx->eax
    mov ebx, [esi + 4]
    mov ecx, [esi + 8]
    mov edx, [esi + 12]
    mov edi, [esi + 20]
    mov ebp, [esi + 24]
    push 0x23       ; User data segment
    push [esi + 40]     ; ctx->useresp
    push [esi + 36]         ; ctx->eflags
    push 0x1b       ; Kernel code segment
    push [esi + 32]         ; ctx->eip

    mov esi, [esp + 16]
    iret