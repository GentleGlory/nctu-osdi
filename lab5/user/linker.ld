
ENTRY(_start)

SECTIONS
{
	. = 0x80000;
	
	.text : {
		*entry.S.obj(.text)
		*(.text)
	}

	.data : {
		. = ALIGN(16);
		*(.data .data.*)
	}

	.rodata : {
		. = ALIGN(16);
		*(.rodata .rodata.*)
	}

	.bss : {
		. = ALIGN(16);
		*(.bss .bss.*)
		*(COMMON)
		. = ALIGN(16);
	}
}
