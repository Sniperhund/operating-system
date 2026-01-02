bits 32

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
start:
    mov esp, stack_end

    extern kernel_main
    call kernel_main

    hlt

section .bss
stack_start:
    resb 1024 * 16 ; 16 KiB
stack_end: