ENTRY (start)

SECTIONS {
	. = 0xC0100000;

	kernel_start = . ;
	
	.text ALIGN(4K) : AT(ADDR(.text)-0xC0000000) {
        KEEP(*(.multiboot2))
        *(.start)
        *(.text)
    }

	.bss ALIGN(4K) : AT(ADDR(.bss)-0xC0000000) {
        *(.bss)
    }

	.data ALIGN(4K) : AT(ADDR(.data)-0xC0000000) {
        *(.data)
    }

	.rodata	ALIGN(4K) : AT(ADDR(.rodata)-0xC0000000) {
        *(.rodata)
    }
	
	kernel_end = . ;
	
}