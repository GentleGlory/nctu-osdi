SECTIONS
{	
	. = 0x80000;
	__sp_address = .;
	.text : {
		*start.o(.text)
		*(.text)
	}

	.bss : {
		. = ALIGN(16);
		__bss_start = .;
		*(.bss)
		. = ALIGN(16);
		__bss_end = .;
	}
}