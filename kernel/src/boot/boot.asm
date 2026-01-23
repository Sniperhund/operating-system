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

; For some reason multiboot2 boots fine when VMA is in higher half
section .start
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
    ; FIX: I don't know how it hasn't crashed from this small stack...
    ;      If it ain't broke don't fix it
    resb 1024 * 16 ; 16 KiB
stack_end:

user_stack_start:
    resb 1024 ; 1 KiB
user_stack_end:

; For some stupid reason this code is put at the very front of the kernel code, so it crashes before even hitting start
; For now it's gonna live here...
section .text
global jump_usermode
jump_usermode:
    cli         ; Disable interrupts

    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; SS is handled by iret

    mov eax, esp
    push 0x23   ; Usermode SS
    push eax
    pushf       ; Push low 16 bits of eflags
    pop eax
    and eax, ~(1 << 14)   ; clear NT
    or  eax, (1 << 9)    ; set IF
    push eax
    push 0x1b
    push test_user_function
    iret

test_user_function:
    mov eax, 23
    hlt