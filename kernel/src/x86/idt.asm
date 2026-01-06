%macro isr_non_err 1
isr_stub_%+%1:
    cli
    push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

%macro isr_err 1
isr_stub_%+%1:
    cli
    push byte %1
    jmp isr_common_stub
%endmacro

extern exceptionHandlerC

isr_common_stub:
    pushad

    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, esp
    push eax
    call exceptionHandlerC

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popad
    add esp, 8
    sti
    iret

isr_non_err     0
isr_non_err     1
isr_non_err     2
isr_non_err     3
isr_non_err     4
isr_non_err     5
isr_non_err     6
isr_non_err     7
isr_err         8
isr_non_err     9
isr_err         10
isr_err         11
isr_err         12
isr_err         13
isr_err         14
isr_non_err     15
isr_non_err     16
isr_err         17
isr_non_err     18
isr_non_err     19
isr_non_err     20
isr_non_err     21
isr_non_err     22
isr_non_err     23
isr_non_err     24
isr_non_err     25
isr_non_err     26
isr_non_err     27
isr_non_err     28
isr_non_err     29
isr_err         30
isr_non_err     31

global isr_stub_table
isr_stub_table:
%assign i 0
%rep    32
    dd isr_stub_%+i
%assign i i+1
%endrep