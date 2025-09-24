.section ".text"
_start:
	//Setting sp 
	ldr x0, =__sp_address
	mov sp, x0
	
	//Let only one core proceed, and let others enter a busy loop.
	mrs x0, mpidr_el1
	and x0, x0, #0xff
	cmp x0, 0
	b.eq 2f

1:	wfe
	b 1b

2:	//Setting exception table
	ldr x0, =exception_table
	msr vbar_el2, x0

	//Route IRQ to EL2
	mrs x0, HCR_EL2
	orr x0, x0, #(1 << 4)
	msr HCR_EL2, x0

	//Clear BSS
	ldr x0, =__bss_start
	ldr x1, =__bss_end
	mov x2, #0
	mov x3, #0	
3:	cmp x0, x1
	b.eq 4f
	stp x2, x3, [x0], #16
	b 3b

	//Run main function
4:	b main
	b 4b