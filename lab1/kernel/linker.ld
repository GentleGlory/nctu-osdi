SECTIONS
{	
	. = 0x80000;
	__sp_address = .;
	.text : {
		*start.S.obj(.text)
		*(.text)
	}

	.data : {
		*(.data .data.*)
	}

	.rodata : { 
		*(.rodata .rodata.*)
	}

	.bss : {
		. = ALIGN(16);
		__bss_start = .;
		*(.bss .bss.*)
		*(COMMON)
		. = ALIGN(16);
		__bss_end = .;
	}
}