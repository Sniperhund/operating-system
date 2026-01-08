bits 32

VM_BASE     equ 0xC0000000
PDE_INDEX   equ (VM_BASE >> 22)
PSE_BIT     equ 0x00000010
PG_BIT      equ 0x80000000

section .multiboot2
header_start:
    dd 0xe85250d6
    dd 0
    dd header_end - header_start

    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    dw 0
    dw 0
    dd 8
header_end:

global start

section .text
low_start equ (start - VM_BASE)
start:
    mov ecx, (TEMP_PD - VM_BASE)
    mov cr3, ecx

    mov ecx, cr4
    or ecx, PSE_BIT
    mov cr4, ecx

    mov ecx, cr0
    or ecx, PG_BIT
    mov cr0, ecx

    lea ecx, [higher_half]
    jmp ecx

higher_half:
    ;mov dword[TEMP_PD], 0 <-- Crashes it?
    ;invlpg[0]

    mov esp, stack_end

    extern kernel_main
    call kernel_main

.end:
    hlt
    jmp .end

section .data
align 4096
global TEMP_PD
TEMP_PD:
    dd 0x00000083
    times (PDE_INDEX - 1) dd 0
    dd 0x00000083
    times (1024 - PDE_INDEX - 1) dd 0

section .bss
align 4
stack_start:
    resb 1024 * 16 ; 16 KiB
stack_end: