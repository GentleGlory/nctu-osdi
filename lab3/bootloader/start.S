.section ".text"

.global _start
.global __jump_to_start_loading_kernel

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
	b 4b



//x0: kernel start addr, x1: kernel size
//x2: kernel checksum, x3: new bootloader start addr
__jump_to_start_loading_kernel:
	ldr x5, =__image_start
	sub x5, x3, x5 //offset
	mov sp, x3
	ldr x4, =start_loading_kernel
	add x4, x4, x5
	mov lr, x4
	ldr x3, =__image_start
	ldr x4, =__bootloader_size
	ret