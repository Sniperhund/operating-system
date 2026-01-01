ENTRY (start)

SECTIONS {
	. = 0x100000;

	kernel_start = . ;
	
	.text : ALIGN(4K) {
        KEEP(*(.multiboot2))
        *(.text)
    }

	.bss : ALIGN(4K) {
        *(.bss)
    }

	.data : ALIGN(4K) {
        *(.data)
    }

	.rodata	: ALIGN(4K) {
        *(.rodata)
    }
	
	kernel_end = . ;
	
}