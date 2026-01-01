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
    mov dword [0xb8000], 0x07690748
    hlt