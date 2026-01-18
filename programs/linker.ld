ENTRY(_start)
SECTIONS
{
    . = 0x400000; /* load user program at 4MB, for example */
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}