SECTIONS
{	
	. = 0x80000;
	__sp_address = .;
	__image_start = .;
	.text : {
		*start.S.obj(.text)
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
		__bss_start = .;
		*(.bss .bss.*)
		*(COMMON)
		. = ALIGN(16);
		__bss_end = .;
	}

	.rela.dyn : {
		. = ALIGN(16);
		__rel_dyn_start = .;
		*(.rela*)
		__rel_dyn_end = .;
	}

	. = ALIGN(16);
	__image_end = .;
}

__bootloader_size = (__image_end - __image_start);
__rel_dyn_size = (__rel_dyn_end - __rel_dyn_start);