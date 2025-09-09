.section ".text"
_start:
	ldr x0, =__sp_address
	mov sp, x0

	mrs x0, mpidr_el1
	and x0, x0, #0xff
	cmp x0, 0
	b.eq 2f

1:	wfe
	b 1b

2:	ldr x0, =__bss_start
	ldr x1, =__bss_end
	mov x2, #0
	mov x3, #0
3:	cmp x0, x1
	b.eq 4f
	stp x2, x3, [x0], #16
	b 3b
4:	b main